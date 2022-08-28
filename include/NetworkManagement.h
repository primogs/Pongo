/////////////////////////////////////////////////////////////////////////////////////////
//    This file is part of Pongo.
//
//    Copyright (C) 2020 Matthias Hund
//    
//    This program is free software; you can redistribute it and/or
//    modify it under the terms of the GNU General Public License
//    as published by the Free Software Foundation; either version 2
//    of the License, or (at your option) any later version.
//    
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//    
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
/////////////////////////////////////////////////////////////////////////////////////////

#ifndef NetworkManagement_H
#define NetworkManagement_H

#include <errno.h>
#include <iostream>
#include <thread>
#include <list>
#include <tuple>
#include <signal.h>
#include <arpa/inet.h>
#include "NetworkManagerBase.h"

class NetworkManagement
{
public:
	NetworkManagement();
	virtual ~NetworkManagement();
	
	static void StartWebServer(int port);
	static void CloseServerSocket();
private:
	static bool Init(int port);
	static void RunServerLoop();
	
	static int BindToSocket(int port);
	static int ListenOnSocket();
	static int AcceptOnSocket(uint32_t &ip_address);
	static void StartThread(int socketToClient,uint32_t ip_address);
	static void* ConnectionHandler(ClientHandler* pCHandler);
		
	static int mServerSock;
};

#endif // NetworkManagement_H
