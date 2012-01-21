/****************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <network.h>
#include "wifi_gecko.h"

static int connection = -1;
static int init = 0;

const char *dest_ip = NULL;
u16 dest_port = 0;

void WifiGecko_Init(const char *ip, const u16 port)
{
	dest_ip = ip;
	dest_port = port;
	init = 1;
}

void WifiGecko_Close()
{
	if (!init) return;

    if(connection >= 0)
        net_close(connection);

    connection = -1;
}

int WifiGecko_Connect()
{
	if (!init) return -2;

    if(connection >= 0)
        return connection;

	if (dest_ip == NULL || dest_port == 0) return connection;

    connection = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (connection < 0)
        return connection;

	struct sockaddr_in connect_addr;
	memset(&connect_addr, 0, sizeof(connect_addr));
	connect_addr.sin_family = AF_INET;
	connect_addr.sin_port = htons(dest_port);
	inet_aton(dest_ip, &connect_addr.sin_addr);

    if(net_connect(connection, (struct sockaddr*)&connect_addr, sizeof(connect_addr)) < 0)
	{
        WifiGecko_Close();
		return -1;
	}

	// First time connect, send hello message
	char *msg = "Wiiflow WiFi Gecko output console connected\n";
	net_send(connection, msg, strlen(msg), 0);

	return connection;
}

int WifiGecko_Send(const char * data, int datasize)
{
	if (!init) return -2;

    if(WifiGecko_Connect() < 0)
        return connection;

    int ret = 0, done = 0, blocksize = 1024;

    while (done < datasize)
    {
        if(blocksize > datasize-done)
            blocksize = datasize-done;

        ret = net_send(connection, data + done, blocksize, 0);
        if (ret < 0)
        {
            WifiGecko_Close();
            return ret;
        }
        else if(ret == 0)
        {
            break;
        }

        done += ret;
        usleep (1000);
    }

    return ret;
}

void wifi_printf(const char * format, ...)
{
	if (!init) return;

	char * tmp = NULL;
	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va) >= 0) && tmp)
	{
		WifiGecko_Send(tmp, strlen(tmp));
	}
	va_end(va);

	SAFE_FREE(tmp);
}