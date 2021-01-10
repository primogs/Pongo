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

#ifndef NetworkManagementSSL_H
#define NetworkManagementSSL_H

#include <sys/socket.h> 
#include <errno.h>
#include <iostream>
#include <pthread.h>
#include <list>
#include <tuple>
#include <signal.h>
#include "ClientHandler.h"

class NetworkManagementSsl
{
public:
	NetworkManagementSsl();
	virtual ~NetworkManagementSsl();
	
	static void SetCertName(const char* cert);
	static void SetBaseFolder(char* resFolder);
	
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
	static int AcceptOnSocket(int sockfd,struct tls **tlsConnection);
	static void StartThread(struct tls *tlsToClient,int socketToClient);
	static void* ConnectionHandler(void *pHandler);
	static void BlockUntilAllThreadsFinished(unsigned int timeout);
	
	static std::list<std::tuple<pthread_t,ClientHandler*> > mHandler;
	static int mSockfd;
	
	static struct tls_config *mTlsConfig;
	static struct tls * mTlsConnection;

	static std::string mBaseFolder;
	static std::string mCertName;
};

#endif // NetworkManagementSSL_H
