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

#include <sys/socket.h> 
#include <errno.h>
#include <iostream>
#include <pthread.h>
#include <list>
#include <tuple>
#include <signal.h>
#include "ClientHandler.h"

class NetworkManagement
{
public:
	NetworkManagement();
	virtual ~NetworkManagement();
	
	static void StartWebServer(int port);
	static void ShutdownSocket();
private:
	static bool Init(int port);
	static void RunServerLoop();
	static bool CheckJoinReturnValue(int value);
	static void CloseSocket();
	static void ShutdownThreads();
	static void KillThreads();
	static void CheckForThreadsFinished();
	static int BindToSocket(int sockfd,int port);
	static int ListenOnSocket(int sockfd);
	static int AcceptOnSocket(int sockfd);
	static void StartThread(int socketToClient);
	static void* ConnectionHandler(void *pHandler);
	static void BlockUntilAllThreadsFinished(unsigned int timeout);
	
	static std::list<std::tuple<pthread_t,ClientHandler*> > mHandler;
	static int mSockfd;
};

#endif // NetworkManagement_H
