// Copyright 2010 Joseph Jordan <joe.ftpii@psychlaw.com.au>
// This code is licensed to you under the terms of the GNU GPL, version 2;
// see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
#include <errno.h>
#include <malloc.h>
#include <network.h>
#include <ogc/lwp_watchdog.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include "gui/fmt.h"
#include "gecko/gecko.hpp"
#include "defines.h"
#include "svnrev.h"
#include "FTP_Dir.hpp"
#include "ftp.h"
#include "net.h"

#define FTP_BUFFER_SIZE 1024
#define MAX_CLIENTS 2

bool ftp_allow_active = false;
u16 ftp_server_port = 21;

static const u16 SRC_PORT = 20;
static const s32 EQUIT = 696969;
static const char *CRLF = "\r\n";
static const u32 CRLF_LENGTH = 2;

static u8 num_clients = 0;
static u16 passive_port = 1024;
static char *password = NULL;

typedef s32 (*data_connection_callback)(s32 data_socket, void *arg);

struct client_struct {
    s32 socket;
    char representation_type;
    s32 passive_socket;
    s32 data_socket;
    char cwd[MAXPATHLEN];
    char pending_rename[MAXPATHLEN];
    off_t restart_marker;
    struct sockaddr_in address;
    bool authenticated;
    char buf[FTP_BUFFER_SIZE];
    s32 offset;
    bool data_connection_connected;
    data_connection_callback data_callback;
    void *data_connection_callback_arg;
    void (*data_connection_cleanup)(void *arg);
    u64 data_connection_timer;
};

typedef struct client_struct client_t;

static client_t *clients[MAX_CLIENTS] = { NULL };

void set_ftp_password(const char *new_password) {
    if (password != NULL) free(password);
    if (new_password != NULL && new_password[0] != '\0') {
        password = malloc(strlen(new_password) + 1);
        if (password == NULL) return;//die("Unable to allocate memory for password", errno);
        strcpy((char *)password, new_password);
    } else {
        password = NULL;
    }
}

static bool compare_ftp_password(char *password_attempt) {
    return !password || !strcmp((char *)password, password_attempt);
}

/*
    TODO: support multi-line reply
*/
static s32 write_reply(client_t *client, u16 code, char *msg) {
    u32 msglen = 4 + strlen(msg) + CRLF_LENGTH;
    char msgbuf[msglen + 1];
    if (msgbuf == NULL) return -ENOMEM;
    strncpy(msgbuf, fmt("%u %s\r\n", code, msg), msglen + 1);
    ftp_dbg_print(fmt("Wrote reply: %s", msgbuf));
    return send_exact(client->socket, msgbuf, msglen);
}

static void close_passive_socket(client_t *client) {
    if (client->passive_socket >= 0) {
        net_close_blocking(client->passive_socket);
        client->passive_socket = -1;
    }
}

/*
    result must be able to hold up to maxsplit+1 null-terminated strings of length strlen(s)
    returns the number of strings stored in the result array (up to maxsplit+1)
*/
static u32 split(char *s, char sep, u32 maxsplit, char *result[]) {
    u32 num_results = 0;
    u32 result_pos = 0;
    u32 trim_pos = 0;
    bool in_word = false;
    for (; *s; s++) {
        if (*s == sep) {
            if (num_results <= maxsplit) {
                in_word = false;
                continue;
            } else if (!trim_pos) {
                trim_pos = result_pos;
            }
        } else if (trim_pos) {
            trim_pos = 0;
        }
        if (!in_word) {
            in_word = true;
            if (num_results <= maxsplit) {
                num_results++;
                result_pos = 0;
            }
        }
        result[num_results - 1][result_pos++] = *s;
        result[num_results - 1][result_pos] = '\0';
    }
    if (trim_pos) {
        result[num_results - 1][trim_pos] = '\0';
    }
    u32 i = num_results;
    for (i = num_results; i <= maxsplit; i++) {
        result[i][0] = '\0';
    }
    return num_results;
}

static s32 ftp_USER(client_t *client, char *username __attribute__((unused))) {
    return write_reply(client, 331, "User name okay, need password.");
}

static s32 ftp_PASS(client_t *client, char *password_attempt) {
    if (compare_ftp_password(password_attempt)) {
        client->authenticated = true;
        return write_reply(client, 230, "User logged in, proceed.");
    } else {
        return write_reply(client, 530, "Login incorrect.");
    }
}

static s32 ftp_REIN(client_t *client, char *rest __attribute__((unused))) {
    close_passive_socket(client);
    strcpy(client->cwd, "/");
    client->representation_type = 'A';
    client->authenticated = false;
    return write_reply(client, 220, "Service ready for new user.");
}

static s32 ftp_QUIT(client_t *client, char *rest __attribute__((unused))) {
    // TODO: dont quit if xfer in progress
    s32 result = write_reply(client, 221, "Service closing control connection.");
	/* reset paths for some strange clients */
	ftp_init();
    return result < 0 ? result : -EQUIT;
}

static s32 ftp_SYST(client_t *client, char *rest __attribute__((unused))) {
    return write_reply(client, 215, "UNIX Type: L8 Version: wiiflow-ftpii");
}

static s32 ftp_TYPE(client_t *client, char *rest) {
    char representation_type[FTP_BUFFER_SIZE], param[FTP_BUFFER_SIZE];
    char *args[] = { representation_type, param };
    u32 num_args = split(rest, ' ', 1, args);
    if (num_args == 0) {
        return write_reply(client, 501, "Syntax error in parameters.");
    } else if ((!strcasecmp("A", representation_type) && (!*param || !strcasecmp("N", param))) ||
               (!strcasecmp("I", representation_type) && num_args == 1)) {
        client->representation_type = *representation_type;
    } else {
        return write_reply(client, 501, "Syntax error in parameters.");
    }
    char msg[15];
    strncpy(msg, fmt("Type set to %s.", representation_type), 15);
    return write_reply(client, 200, msg);
}

static s32 ftp_MODE(client_t *client, char *rest) {
    if (!strcasecmp("S", rest)) {
        return write_reply(client, 200, "Mode S ok.");
    } else {
        return write_reply(client, 501, "Syntax error in parameters.");
    }
}

static s32 ftp_PWD(client_t *client, char *rest __attribute__((unused))) {
    char msg[MAXPATHLEN + 24];
    // TODO: escape double-quotes
    strncpy(msg, fmt("\"%s\" is current directory.", ftp_getpath()), MAXPATHLEN + 24);
    return write_reply(client, 257, msg);
}

static s32 ftp_CWD(client_t *client, char *path) {
    s32 result;
    if (ftp_changedir(path) == 0) {
        result = write_reply(client, 250, "CWD command successful.");
    } else  {
        result = write_reply(client, 550, strerror(errno));
    }
    return result;
}

static s32 ftp_CDUP(client_t *client, char *rest __attribute__((unused))) {
    s32 result;
    if (ftp_changedir("..") == 0) {
        result = write_reply(client, 250, "CDUP command successful.");
    } else  {
        result = write_reply(client, 550, strerror(errno));
    }
    return result;
}

static s32 ftp_DELE(client_t *client, char *path) {
    if (ftp_delete(path) == 0) {
        return write_reply(client, 250, "File or directory removed.");
    } else {
        return write_reply(client, 550, strerror(errno));
    }
}

static s32 ftp_MKD(client_t *client, char *path) {
    if (!*path) {
        return write_reply(client, 501, "Syntax error in parameters.");
    }
    if (ftp_makedir(path) == 0) {
        char msg[MAXPATHLEN + 21];
        // TODO: escape double-quotes
        strncpy(msg, fmt("\"%s\" directory created.", path), MAXPATHLEN + 21);
        return write_reply(client, 257, msg);
    } else {
        return write_reply(client, 550, strerror(errno));
    }
}

static s32 ftp_RNFR(client_t *client, char *path) {
    strcpy(client->pending_rename, path);
    return write_reply(client, 350, "Ready for RNTO.");
}

static s32 ftp_RNTO(client_t *client, char *path) {
    if (!*client->pending_rename) {
        return write_reply(client, 503, "RNFR required first.");
    }
    s32 result;
    if (ftp_rename(client->pending_rename, path) == 0) {
        result = write_reply(client, 250, "Rename successful.");
    } else {
        result = write_reply(client, 550, strerror(errno));
    }
    *client->pending_rename = '\0';
    return result;
}

static s32 ftp_SIZE(client_t *client, char *path) {
    struct stat st;
    if (ftp_stat(path, &st) == 0) {
        char size_buf[12];
        strncpy(size_buf, fmt("%llu", st.st_size), 12);
        return write_reply(client, 213, size_buf);
    } else {
        return write_reply(client, 550, strerror(errno));
    }
}

static s32 ftp_PASV(client_t *client, char *rest __attribute__((unused))) {
    close_passive_socket(client);
    client->passive_socket = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (client->passive_socket < 0) {
        return write_reply(client, 520, "Unable to create listening socket.");
    }
    set_blocking(client->passive_socket, false);
    struct sockaddr_in bindAddress;
    memset(&bindAddress, 0, sizeof(bindAddress));
    bindAddress.sin_family = AF_INET;
    if (passive_port < 1024) passive_port = 1024;
    bindAddress.sin_port = htons(passive_port++);
    bindAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    s32 result;
    if ((result = net_bind(client->passive_socket, (struct sockaddr *)&bindAddress, sizeof(bindAddress))) < 0) {
        close_passive_socket(client);
        return write_reply(client, 520, "Unable to bind listening socket.");
    }
    if ((result = net_listen(client->passive_socket, 1)) < 0) {
        close_passive_socket(client);
        return write_reply(client, 520, "Unable to listen on socket.");
    }
    char reply[49];
    u16 port = bindAddress.sin_port;
    u32 ip = net_gethostip();
    struct in_addr addr;
    addr.s_addr = ip;
    ftp_dbg_print(fmt("Listening for data connections at %s:%u...\n", inet_ntoa(addr), port));
    strncpy(reply, fmt("Entering Passive Mode (%u,%u,%u,%u,%u,%u).", (ip >> 24) & 0xff, (ip >> 16) & 0xff, (ip >> 8) & 0xff, ip & 0xff, (port >> 8) & 0xff, port & 0xff), 49);
    return write_reply(client, 227, reply);
}

static s32 ftp_PORT(client_t *client, char *portspec) {
	if(ftp_allow_active == false) /* port is only used for active clients */
		return write_reply(client, 502, "Command not implemented.");

    u32 h1, h2, h3, h4, p1, p2;
    if (sscanf(portspec, "%3u,%3u,%3u,%3u,%3u,%3u", &h1, &h2, &h3, &h4, &p1, &p2) < 6) {
        return write_reply(client, 501, "Syntax error in parameters.");
    }
    char addr_str[44];
    strncpy(addr_str, fmt("%u.%u.%u.%u", h1, h2, h3, h4), 44);
    struct in_addr sin_addr;
    if (!inet_aton(addr_str, &sin_addr)) {
        return write_reply(client, 501, "Syntax error in parameters.");
    }
    close_passive_socket(client);
    u16 port = ((p1 &0xff) << 8) | (p2 & 0xff);
    client->address.sin_addr = sin_addr;
    client->address.sin_port = htons(port);
    ftp_dbg_print(fmt("Set client address to %s:%u\n", addr_str, port));
    return write_reply(client, 200, "PORT command successful.");
}

typedef s32 (*data_connection_handler)(client_t *client);

static s32 prepare_data_connection_active(client_t *client) {
    s32 data_socket = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (data_socket < 0) return data_socket;
    set_blocking(data_socket, false);
    struct sockaddr_in bindAddress;
    memset(&bindAddress, 0, sizeof(bindAddress));
    bindAddress.sin_family = AF_INET;
    bindAddress.sin_port = htons(SRC_PORT);
    bindAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    s32 result;
    if ((result = net_bind(data_socket, (struct sockaddr *)&bindAddress, sizeof(bindAddress))) < 0) {
        net_close(data_socket);
        return result;
    }
    
    client->data_socket = data_socket;
    ftp_dbg_print(fmt("Attempting to connect to client at %s:%u\n", 
		inet_ntoa(client->address.sin_addr), ntohs(client->address.sin_port)));
    return 0;
}

static s32 prepare_data_connection_passive(client_t *client) {
    client->data_socket = client->passive_socket;
    ftp_dbg_print("Waiting for data connections...\n");
    return 0;
}

static s32 prepare_data_connection(client_t *client, void *callback, void *arg, void *cleanup) {
    s32 result = write_reply(client, 150, "Transferring data.");
    if (result >= 0) {
		result = -1;
		if(client->passive_socket >= 0)
			result = prepare_data_connection_passive(client);
		else if(ftp_allow_active == true)
			result = prepare_data_connection_active(client);
        if (result < 0) {
            result = write_reply(client, 520, "Closing data connection, error occurred during transfer.");
        } else {
            client->data_connection_connected = false;
            client->data_callback = callback;
            client->data_connection_callback_arg = arg;
            client->data_connection_cleanup = cleanup;
            client->data_connection_timer = gettime() + secs_to_ticks(30);
        }
    }
    return result;
}

static s32 send_nlst(s32 data_socket, DIR *dir) {
    s32 result = 0;
    char filename[MAXPATHLEN + 2];
    while (ftp_dirnext(dir, filename) == 0) {
        size_t end_index = strlen(filename);
        filename[end_index] = CRLF[0];
        filename[end_index + 1] = CRLF[1];
        filename[end_index + 2] = '\0';
        if ((result = send_exact(data_socket, filename, strlen(filename))) < 0) {
            break;
        }
    }
    return result < 0 ? result : 0;
}

static s32 send_list(s32 data_socket, DIR *dir) {
    s32 result = 0;
    char filename[MAXPATHLEN];
    struct stat st;
    char line[MAXPATHLEN + 56 + CRLF_LENGTH + 1];
    while (ftp_dirnext(dir, filename) == 0) {
        char timestamp[13];
		ftp_stat(filename, &st);
        strftime(timestamp, sizeof(timestamp), "%b %d  %Y", localtime(&st.st_mtime));
        strncpy(line, fmt("%crwxr-xr-x    1 0        0     %10llu %s %s\r\n", (st.st_mode & S_IFDIR) ? 'd' : '-', st.st_size, timestamp, filename), MAXPATHLEN + 56 + CRLF_LENGTH + 1);
        if ((result = send_exact(data_socket, line, strlen(line))) < 0) {
            break;
        }
    }
    return result < 0 ? result : 0;
}

static s32 ftp_NLST(client_t *client, char *path) {
    if (!*path) {
        path = ".";
    }

    DIR *dir = ftp_diropen();
    if (dir == NULL) {
        return write_reply(client, 550, strerror(errno));
    }

    s32 result = prepare_data_connection(client, send_nlst, dir, ftp_dirclose);
    if (result < 0) ftp_dirclose(dir);
    return result;
}

static s32 ftp_LIST(client_t *client, char *path) {
    if (*path == '-') {
        // handle buggy clients that use "LIST -aL" or similar, at the expense of breaking paths that begin with '-'
        char flags[FTP_BUFFER_SIZE];
        char rest[FTP_BUFFER_SIZE];
        char *args[] = { flags, rest };
        split(path, ' ', 1, args);
        path = rest;
    }
    if (!*path) {
        path = ".";
    }
	//s32 result = -1;
    DIR *dir = ftp_diropen();
    /*if (dir == NULL) {
        return write_reply(client, 550, strerror(errno));
    }*/
    s32 result = prepare_data_connection(client, send_list, dir, ftp_dirclose);
    if (result < 0) ftp_dirclose(dir);
    return result;
}

static s32 ftp_RETR(client_t *client, char *path) {
    FILE *f = ftp_fopen(path, "rb");
    if (!f) {
        return write_reply(client, 550, strerror(errno));
    }

    int fd = fileno(f);
    if (client->restart_marker && lseek(fd, client->restart_marker, SEEK_SET) != client->restart_marker) {
        s32 lseek_error = errno;
        ftp_fclose(f);
        client->restart_marker = 0;
        return write_reply(client, 550, strerror(lseek_error));
    }
    client->restart_marker = 0;

    s32 result = prepare_data_connection(client, send_from_file, f, ftp_fclose);
    if (result < 0) ftp_fclose(f);
    return result;
}

static s32 stor_or_append(client_t *client, FILE *f) {
    if (!f) {
        return write_reply(client, 550, strerror(errno));
    }
    s32 result = prepare_data_connection(client, recv_to_file, f, ftp_fclose);
    if (result < 0) ftp_fclose(f);
    return result;
}

static s32 ftp_STOR(client_t *client, char *path __attribute__((unused))) {
    FILE *f = ftp_fopen(path, "wb");
    int fd;
    if (f) fd = fileno(f);
    if (f && client->restart_marker && lseek(fd, client->restart_marker, SEEK_SET) != client->restart_marker) {
        s32 lseek_error = errno;
        ftp_fclose(f);
        client->restart_marker = 0;
        return write_reply(client, 550, strerror(lseek_error));
    }
    client->restart_marker = 0;

    return stor_or_append(client, f);
}

static s32 ftp_APPE(client_t *client, char *path __attribute__((unused))) {
    return stor_or_append(client, ftp_fopen(path, "ab"));
}

static s32 ftp_REST(client_t *client, char *offset_str) {
    off_t offset;
    if (sscanf(offset_str, "%lli", &offset) < 1 || offset < 0) {
        return write_reply(client, 501, "Syntax error in parameters.");
    }
    client->restart_marker = offset;
    char msg[FTP_BUFFER_SIZE];
    strncpy(msg, fmt("Restart position accepted (%lli).", offset), FTP_BUFFER_SIZE);
    return write_reply(client, 350, msg);
}

typedef s32 (*ftp_command_handler)(client_t *client, char *args);

static s32 dispatch_to_handler(client_t *client, char *cmd_line, const char **commands, const ftp_command_handler *handlers) {
    char cmd[FTP_BUFFER_SIZE], rest[FTP_BUFFER_SIZE];
    char *args[] = { cmd, rest };
    split(cmd_line, ' ', 1, args); 
    s32 i;
    for (i = 0; commands[i]; i++) {
        if (!strcasecmp(commands[i], cmd)) break;
    }
    return handlers[i](client, rest);
}

static s32 ftp_NOOP(client_t *client, char *rest __attribute__((unused))) {
    return write_reply(client, 200, "NOOP command successful.");
}

static s32 ftp_SUPERFLUOUS(client_t *client, char *rest __attribute__((unused))) {
    return write_reply(client, 202, "Command not implemented, superfluous at this site.");
}

static s32 ftp_NEEDAUTH(client_t *client, char *rest __attribute__((unused))) {
    return write_reply(client, 530, "Please login with USER and PASS.");
}

static s32 ftp_UNKNOWN(client_t *client, char *rest __attribute__((unused))) {
    return write_reply(client, 502, "Command not implemented.");
}

static const char *unauthenticated_commands[] = { "USER", "PASS", "QUIT", "REIN", "NOOP", NULL };
static const ftp_command_handler unauthenticated_handlers[] = { ftp_USER, ftp_PASS, ftp_QUIT, ftp_REIN, ftp_NOOP, ftp_NEEDAUTH };

static const char *authenticated_commands[] = {
    "USER", "PASS", "LIST", "PWD", "CWD", "CDUP",
    "SIZE", "PASV", "PORT", "TYPE", "SYST", "MODE",
    "RETR", "STOR", "APPE", "REST", "DELE", "MKD",
    "RMD", "RNFR", "RNTO", "NLST", "QUIT", "REIN",
    "NOOP", "ALLO", NULL
};
static const ftp_command_handler authenticated_handlers[] = {
    ftp_USER, ftp_PASS, ftp_LIST, ftp_PWD, ftp_CWD, ftp_CDUP,
    ftp_SIZE, ftp_PASV, ftp_PORT, ftp_TYPE, ftp_SYST, ftp_MODE,
    ftp_RETR, ftp_STOR, ftp_APPE, ftp_REST, ftp_DELE, ftp_MKD,
    ftp_DELE, ftp_RNFR, ftp_RNTO, ftp_NLST, ftp_QUIT, ftp_REIN,
    ftp_NOOP, ftp_SUPERFLUOUS, ftp_UNKNOWN
};

/*
    returns negative to signal an error that requires closing the connection
*/
static s32 process_command(client_t *client, char *cmd_line) {
    if (strlen(cmd_line) == 0) {
        return 0;
    }

    ftp_dbg_print(fmt("Got command: %s\n", cmd_line));

    const char **commands = unauthenticated_commands;
    const ftp_command_handler *handlers = unauthenticated_handlers;

    if (client->authenticated) {
        commands = authenticated_commands;
        handlers = authenticated_handlers;
    }

    return dispatch_to_handler(client, cmd_line, commands, handlers);
}

static void cleanup_data_resources(client_t *client) {
    if (client->data_socket >= 0 && client->data_socket != client->passive_socket) {
        net_close_blocking(client->data_socket);
    }
    client->data_socket = -1;
    client->data_connection_connected = false;
    client->data_callback = NULL;
    if (client->data_connection_cleanup) {
        client->data_connection_cleanup(client->data_connection_callback_arg);
    }
    client->data_connection_callback_arg = NULL;
    client->data_connection_cleanup = NULL;
    client->data_connection_timer = 0;
}

static void cleanup_client(client_t *client) {
    net_close_blocking(client->socket);
    cleanup_data_resources(client);
    close_passive_socket(client);
    int client_index;
    for (client_index = 0; client_index < MAX_CLIENTS; client_index++) {
        if (clients[client_index] == client) {
            clients[client_index] = NULL;
            break;
        }
    }
    free(client);
    num_clients--;
	if(num_clients == 0)
		ftp_init(); /* reinit for new clients */
    ftp_dbg_print("Client disconnected.\n");
}

void cleanup_ftp() {
    int client_index;
    for (client_index = 0; client_index < MAX_CLIENTS; client_index++) {
        client_t *client = clients[client_index];
        if (client) {
            write_reply(client, 421, "Service not available, closing control connection.");
            cleanup_client(client);
        }
    }
}

static bool process_accept_events(s32 server) {
    s32 peer;
    struct sockaddr_in client_address;
    socklen_t addrlen = sizeof(client_address);
    while ((peer = net_accept(server, (struct sockaddr *)&client_address, &addrlen)) != -EAGAIN) {
        if (peer < 0) {
            ftp_dbg_print(fmt("Error accepting connection: [%i] %s\n", -peer, strerror(-peer)));
            return false;
        }

        ftp_dbg_print(fmt("Accepted connection from %s!\n", inet_ntoa(client_address.sin_addr)));

        if (num_clients == MAX_CLIENTS) {
            ftp_dbg_print(fmt("Maximum of %u clients reached, not accepting client.\n", MAX_CLIENTS));
            net_close(peer);
            return true;
        }

        client_t *client = malloc(sizeof(client_t));
        if (!client) {
            ftp_dbg_print("Could not allocate memory for client state, not accepting client.\n");
            net_close(peer);
            return true;
        }
        client->socket = peer;
        client->representation_type = 'A';
        client->passive_socket = -1;
        client->data_socket = -1;
        strcpy(client->cwd, "/");
        *client->pending_rename = '\0';
        client->restart_marker = 0;
        client->authenticated = false;
        client->offset = 0;
        client->data_connection_connected = false;
        client->data_callback = NULL;
        client->data_connection_callback_arg = NULL;
        client->data_connection_cleanup = NULL;
        client->data_connection_timer = 0;
        memcpy(&client->address, &client_address, sizeof(client_address));
        int client_index;
		char *welcome = fmt("Welcome to %s (%s-r%s)! This is the ftpii server core.", APP_NAME, APP_VERSION, SVN_REV);
        if (write_reply(client, 220, welcome) < 0) {
            ftp_dbg_print("Error writing greeting.\n");
            net_close_blocking(peer);
            free(client);
        } else {
            for (client_index = 0; client_index < MAX_CLIENTS; client_index++) {
                if (!clients[client_index]) {
                    clients[client_index] = client;
                    break;
                }
            }
            num_clients++;
        }
    }
    return true;
}

static void process_data_events(client_t *client) {
    s32 result = -1;
    if (!client->data_connection_connected) {
        if (client->passive_socket >= 0) {
            struct sockaddr_in data_peer_address;
            socklen_t addrlen = sizeof(data_peer_address);
            result = net_accept(client->passive_socket, (struct sockaddr *)&data_peer_address ,&addrlen);
            if (result >= 0) {
                client->data_socket = result;
                client->data_connection_connected = true;
            }
        } else if(ftp_allow_active == true) {
            if ((result = net_connect(client->data_socket, (struct sockaddr *)&client->address, sizeof(client->address))) < 0) {
                if(result == -EINPROGRESS || result == -EALREADY)
					result = -EAGAIN;
                if(result != -EAGAIN && result != -EISCONN)
					ftp_dbg_print(fmt("Unable to connect to client: [%i] %s\n", -result, strerror(-result)));
            }
             if (result >= 0 || result == -EISCONN) {
                client->data_connection_connected = true;
            }
        }
        if (client->data_connection_connected) {
            result = 1;
            ftp_dbg_print("Connected to client!  Transferring data...\n");
        } else if (gettime() > client->data_connection_timer) {
            result = -1;
            ftp_dbg_print("Timed out waiting for data connection.\n");
        }
    } else {
        result = client->data_callback(client->data_socket, client->data_connection_callback_arg);
    }

    if (result <= 0 && result != -EAGAIN) {
        cleanup_data_resources(client);
        if (result < 0) {
            result = write_reply(client, 520, "Closing data connection, error occurred during transfer.");
        } else {
            result = write_reply(client, 226, "Closing data connection, transfer successful.");
        }
        if (result < 0) {
            cleanup_client(client);
        }
    }
}

static void process_control_events(client_t *client) {
    s32 bytes_read;
    while (client->offset < (FTP_BUFFER_SIZE - 1)) {
        if (client->data_callback) {
            return;
        }
        char *offset_buf = client->buf + client->offset;
        if ((bytes_read = net_read(client->socket, offset_buf, FTP_BUFFER_SIZE - 1 - client->offset)) < 0) {
            if (bytes_read != -EAGAIN) {
                ftp_dbg_print(fmt("Read error %i occurred, closing client.\n", bytes_read));
                goto recv_loop_end;
            }
            return;
        } else if (bytes_read == 0) {
            goto recv_loop_end; // EOF from client
        }
        client->offset += bytes_read;
        client->buf[client->offset] = '\0';
    
        if (strchr(offset_buf, '\0') != (client->buf + client->offset)) {
            ftp_dbg_print("Received a null byte from client, closing connection ;-)\n"); // i have decided this isn't allowed =P
            goto recv_loop_end;
        }

        char *next;
        char *end;
        for (next = client->buf; (end = strstr(next, CRLF)) && !client->data_callback; next = end + CRLF_LENGTH) {
            *end = '\0';
            if (strchr(next, '\n')) {
                ftp_dbg_print("Received a line-feed from client without preceding carriage return, closing connection ;-)\n"); // i have decided this isn't allowed =P
                goto recv_loop_end;
            }
        
            if (*next) {
                s32 result;
                if ((result = process_command(client, next)) < 0) {
                    if (result != -EQUIT) {
                        ftp_dbg_print(fmt("Closing connection due to error while processing command: %s\n", next));
                    }
                    goto recv_loop_end;
                }
            }
        
        }
    
        if (next != client->buf) { // some lines were processed
            client->offset = strlen(next);
            char tmp_buf[client->offset];
            memcpy(tmp_buf, next, client->offset);
            memcpy(client->buf, tmp_buf, client->offset);
        }
    }
    ftp_dbg_print(fmt("Received line longer than %u bytes, closing client.\n", FTP_BUFFER_SIZE - 1));

    recv_loop_end:
    cleanup_client(client);
}

bool process_ftp_events(s32 server) {
    bool network_down = !process_accept_events(server);
    int client_index;
    for (client_index = 0; client_index < MAX_CLIENTS; client_index++) {
        client_t *client = clients[client_index];
        if (client) {
            if (client->data_callback) {
                process_data_events(client);
            } else {
                process_control_events(client);
            }
        }
    }
    return network_down;
}
