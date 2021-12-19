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
#include <thread>
#include <list>
#include <tuple>
#include <signal.h>
#include <arpa/inet.h>
#include "NetworkManagerBase.h"

class NetworkManagementSsl
{
public:
	NetworkManagementSsl();
	virtual ~NetworkManagementSsl();
	
	static void CloseSocket();
	
	static void SetCertName(const char* cert);
	static void SetBaseFolder(char* resFolder);
	
	static void StartWebServer(int port);
private:
	static bool Init(int port);
	static void RunServerLoop();
	
	static void ShutdownThreads();
	static int BindToSocket(int sockfd,int port);
	static int ListenOnSocket(int sockfd);
	static int AcceptOnSocket(int sockfd,uint32_t &ip_address,SSL **sslConnection);
	static void StartThread(SSL *sslToClient,int socketToClient,uint32_t ip_address);
	static void* ConnectionHandler(ClientHandler* pCHandler);
	static void BlockUntilAllThreadsFinished(unsigned int timeout);
	static void HandleErrorSSL(std::string msg,int connfd,SSL **sslConnection,int result);
	static void HandleError(std::string msg,int connfd);

	static int mSockfd;
	
	static SSL_CTX * mSslContext;

	static std::string mBaseFolder;
	static std::string mCertName;
	
	static std::string mSslLogName;
};

#endif // NetworkManagementSSL_H
