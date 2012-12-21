/****************************************************************************
 * Copyright (C) 2010 by Dimok
 *           (C) 2012 by FIX94
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
 ***************************************************************************/
#ifndef WIFI_GECKO_HPP_
#define WIFI_GECKO_HPP_

#include <network.h>

#define WIFIGECKO_SIZE	1024

class WifiGecko
{
public:
	WifiGecko();
	void Init(const char *ip, u16 port);
	int Send(const char *data, int datasize);
	void SetBuffer(bool buf);
	void Close();
private:
	int Connect();

	bool inited;
	bool buffer;
	const char *dest_ip;
	u16 dest_port;
	volatile int connection;
	sockaddr_in connect_addr;
	char wifigeckobuffer[WIFIGECKO_SIZE];
};
extern WifiGecko WiFiDebugger;

#endif
