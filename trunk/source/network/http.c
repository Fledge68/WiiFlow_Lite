
#include <stdlib.h>
#include <fcntl.h>
#include <ogc/lwp_watchdog.h>
#include <time.h>

#include "http.h"
#include "gecko/gecko.hpp"

/**
 * Emptyblock is a statically defined variable for functions to return if they are unable
 * to complete a request
 */
const struct block emptyblock = {0, NULL};
#define NET_BUFFER_SIZE 1024 //The maximum amount of bytes to send per net_write() call
#define TCP_TIMEOUT 	4000 // 4 secs to receive

// Write our message to the server
static s32 send_message(s32 server, char *msg)
{
	s32 bytes_transferred = 0, remaining = strlen(msg);
	s64 t = gettime();
	while (remaining)
	{
		if((bytes_transferred = net_write(server, msg, remaining > NET_BUFFER_SIZE ? NET_BUFFER_SIZE : remaining)) > 0)
		{
			remaining -= bytes_transferred;
			usleep (20 * 1000);
			t = gettime();
		}
		else if(bytes_transferred < 0) return bytes_transferred;

		if(ticks_to_millisecs(diff_ticks(t, gettime())) > TCP_TIMEOUT)
			break;
	}
	return 0;
}

/**
 * Connect to a remote server via TCP on a specified port
 *
 * @param u32 ip address of the server to connect to
 * @param u32 the port to connect to on the server
 * @return s32 The connection to the server (negative number if connection could not be established)
 */
static s32 server_connect(u32 ipaddress, u32 socket_port)
{
	//Initialize socket
	s32 connection = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if(connection < 0) return connection;

	//Set the connection parameters for the socket
 	struct sockaddr_in connect_addr;
	memset(&connect_addr, 0, sizeof(connect_addr));
	connect_addr.sin_family = AF_INET;
	connect_addr.sin_port = socket_port;
	connect_addr.sin_addr.s_addr= ipaddress;

	//Attemt to open a connection on the socket
	if(net_connect(connection, (struct sockaddr*)&connect_addr, sizeof(connect_addr)) < 0)
		net_close(connection);

	return connection;
}

//The amount of memory in bytes reserved initially to store the HTTP response in
//Be careful in increasing this number, reading from a socket on the Wii 
#define HTTP_BUFFER_GROWTH 1024 * 5

/**
 * This function reads all the data from a connection into a buffer which it returns.
 * It will return an empty buffer if something doesn't go as planned
 *
 * @param s32 connection The connection identifier to suck the response out of
 * @return block A 'block' struct (see http.h) in which the buffer is located
 */
static struct block read_message(s32 connection, struct block buffer, bool (*f)(void *, int, int), void *ud)
{
	static char tmpHdr[512];
	bool hdr = false, fail = true;
	u32 fileSize = 0, step = 0, offset = 0;
	s64 t = gettime();

	//The offset variable always points to the first byte of memory that is free in the buffer
	while (true)
	{
 		if(ticks_to_millisecs(diff_ticks(t, gettime())) > TCP_TIMEOUT || buffer.size <= offset)
			break;

		//Fill the buffer with a new batch of bytes from the connection,
		//starting from where we left of in the buffer till the end of the buffer
		u32 len = buffer.size - offset;
		s32 bytes_read = net_read(connection, buffer.data + offset, len > HTTP_BUFFER_GROWTH ? HTTP_BUFFER_GROWTH : len);
		//Anything below 0 is an error in the connection
		if(bytes_read > 0)
		{
			t = gettime ();

			offset += bytes_read;
			// Not enough memory
			if(buffer.size <= offset) return emptyblock;
			if(!hdr && offset >= sizeof tmpHdr)
			{
				hdr = true;
				memcpy(tmpHdr, buffer.data, sizeof tmpHdr - 1);
				tmpHdr[sizeof tmpHdr - 1] = 0;
				const char *p = strstr(tmpHdr, "Content-Length:");
				if(p != 0)
				{
					p += sizeof "Content-Length:";
					fileSize = strtol(p, 0, 10);
				}
			}
			if(step * HTTP_BUFFER_GROWTH < offset)
			{
				++step;
				if(f != 0)
				{
					if((fileSize != 0 && !f(ud, fileSize, offset <= fileSize ? offset : fileSize)) ||
						(fileSize == 0 && !f(ud, buffer.size, offset)))
						return emptyblock;
				}
			}
			fail = false;
		}
		else
		{
			if(bytes_read < 0) fail = true;
			break; // Need to translate the error messages here instead of just breaking.
		}
	}
	if(fail) return emptyblock;
	//At the end of above loop offset should be precisely the amount of bytes that were read from the connection
	buffer.size = offset;
	return buffer;
}

/* Downloads the contents of a URL to memory
 * This method is not threadsafe (because networking is not threadsafe on the Wii) */
struct block downloadfile(u8 *buffer, u32 bufferSize, const char *url, bool (*f)(void *, int, int), void *ud)
{
	//Check if the url starts with "http://", if not it is not considered a valid url
	if(strncmp(url, "http://", strlen("http://")) != 0) return emptyblock;
	
	//Locate the path part of the url by searching for '/' past "http://"
	char *path = strchr(url + strlen("http://"), '/');
	if(path == NULL) return emptyblock;
	
	//Extract the domain part out of the url
	int domainlength = path - url - strlen("http://");
	if(domainlength == 0) return emptyblock;
	
	char domain[domainlength + 1];
	strncpy(domain, url + strlen("http://"), domainlength);
	domain[domainlength] = '\0';
	
	//Parsing of the URL is done, start making an actual connection
	u32 ipaddress = getipbynamecached(domain);
	if(ipaddress == 0) return emptyblock;

	s32 connection = server_connect(ipaddress, 80);
	if(connection < 0) return emptyblock;
	
	//Form a nice request header to send to the webserver
	char* headerformat = "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: WiiFlow 2.1\r\n\r\n";;
	char header[strlen(headerformat) + strlen(domain) + strlen(path)];
	sprintf(header, headerformat, path, domain);

	//Do the request and get the response
	send_message(connection, header);
	if (bufferSize == 0) return emptyblock;
	struct block buf;
	buf.data = buffer;
	buf.size = bufferSize;
	struct block response = read_message(connection, buf, f, ud);
	net_close(connection);

	//Search for the 4-character sequence \r\n\r\n in the response which signals the start of the http payload (file)
	unsigned char *filestart = NULL;
	u32 filesize = 0;
	u32 i;
	for(i = 3; i < response.size; i++)
	{
		if(response.data[i] == '\n' &&
			response.data[i-1] == '\r' &&
			response.data[i-2] == '\n' &&
			response.data[i-3] == '\r')
		{
			filestart = response.data + i + 1;
			filesize = response.size - i - 1;
			break;
		}
	}
	
	if(response.size == 0 || response.data == NULL) return emptyblock;
	
	// Check for the headers
	char httpCode[3];
	memcpy(httpCode, &response.data[9], 3);
	int retCode = atoi(httpCode);

	switch(retCode)
	{
		case 301:
		case 302:
		case 307: // Moved
/*
			{
			char redirectedTo[255];
			if(findHeader((char *) response.data, (filestart - response.data), "Location", redirectedTo, 255) == 0) {
				return downloadfile(buffer, bufferSize, (char *) redirectedTo, f, ud);
			}
			return emptyblock;
			break;
		}
*/		
		case 404: // Error, file not found!
			return emptyblock;
	}
	
	if(filestart == NULL) return emptyblock;
	
	//Copy the file part of the response into a new memoryblock to return
	struct block file;
	file.data = filestart;
	file.size = filesize;
	return file;
}
