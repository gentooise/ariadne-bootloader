/*
 * EthernetReset.cpp - Ariadne Bootloader Reset Server library
 * Copyright (c) 2012 Stylianos Tsampas.  All right reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "EthernetReset.h"

/******************************************************************************
 * Definitions
 ******************************************************************************/

void EthernetReset::stdResponse(char* msg, int refresh)
{
	_client.println("HTTP/1.1 200 OK");
	_client.println("Content-Type: text/html");
	_client.println("Connnection: close");
	_client.println();
	_client.println("<!DOCTYPE HTML>");
	_client.println("<html>");
	if (refresh) {
		_client.print("<head><meta http-equiv=\"refresh\" content=\"");
		_client.print(refresh);
		_client.println("\" ></head>");
	}
	_client.println("<body>");
	for (char* c = msg; *c != '\0'; c++) {
		_client.print(*c);
		if (*c == '\n') {
			_client.print("<br>\n");
		}
	}
	_client.println();
	_client.println("</body>");
	_client.println("</html>");
}

void EthernetReset::watchdogReset()
{
	delay(10);
	_client.stop();
	wdt_disable();
	wdt_enable(WDTO_2S);
	while(1);
}

/******************************************************************************
 * Constructors / Destructors
 ******************************************************************************/
EthernetReset::EthernetReset(int port) :
	_server(port),
	_refresh(0)
{
	String path = NetEEPROM.readPass();
	path.toCharArray(_path, 20);
	_status[0] = 0;
}

/******************************************************************************
 * User API
 ******************************************************************************/

void EthernetReset::begin()
{
	if(NetEEPROM.netSigIsSet()) {
		Ethernet.begin(NetEEPROM.readMAC(), NetEEPROM.readIP(), NetEEPROM.readGW(),
					   NetEEPROM.readGW(), NetEEPROM.readSN());

		_server.begin();
		DBG(
			Serial.print("Server is at ");
			Serial.println(Ethernet.localIP());
			Serial.print("Gw at ");
			Serial.println(NetEEPROM.readGW());
			Serial.print("Password: ");
			Serial.println(_path);)
	}
}

void EthernetReset::check()
{
	/* 25 is the the maximum command lenth plus
	 * the standart GET and HTTP prefix and postfix */
	char http_req[strlen(_path) + 25];
	_client = _server.available();
	if(_client) {
		DBG(Serial.println("new client connected");)
		while(_client.connected()) {
			if(_client.available()) {
				char c;
				char* url = http_req;
				while(( c = _client.read()) != '\n'){
					*url = c;
					url++;
				}
				DBG(*url = '\0';)
				url = http_req + 5;
				_client.flush();
				DBG(Serial.println(url);)
				if(!strncmp(url, _path,strlen(_path))) {
					url += (strlen(_path) + 1);
					if(!strncmp(url, "reset", 5)) {
						stdResponse("Arduino will be doing a normal reset in 2 seconds");
						watchdogReset();
					} else if(!strncmp(url, "reprogram", 9)) {
						stdResponse("Arduino will reset for reprogramming in 2 seconds");
						NetEEPROM.writeImgBad();
						watchdogReset();
					} else if(!strncmp(url, "status", 6)) {
						stdResponse(_status, _refresh);
					} else stdResponse("Wrong command");
				} else stdResponse("Wrong path");
				break;
			}
		}
		delay(10);
		_client.stop();
		DBG(Serial.println("client disconnected");)
	}
}

void EthernetReset::status(char* msg) {
	size_t len = strlen(msg);
	size_t maxlen = sizeof(_status) - 1;
	if (len > maxlen) len = maxlen;
	memcpy(_status, msg, len);
	_status[len] = 0;
}

void EthernetReset::refresh(int r) {
	_refresh = r;
}
