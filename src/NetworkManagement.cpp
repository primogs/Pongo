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
#include "NetworkManagement.h"

extern volatile bool keepRunning;
std::list<ClientHandler*> NetworkManagement::mHandler;
std::mutex NetworkManagement::mHandlerMutex;

int NetworkManagement::mSockfd=0;

NetworkManagement::NetworkManagement()
{
}

NetworkManagement::~NetworkManagement()
{
}

void NetworkManagement::StartWebServer(int port)
{
	if(Init(port))
	{
		std::cout << "web server listen on port "<< port << std::endl;
		RunServerLoop();
		ShutdownThreads();
		BlockUntilAllThreadsFinished(30u);
		CloseSocket();
		std::cout << "web server closed" << std::endl;
	}
}

bool NetworkManagement::Init(int port)
{
	int result = 0;
    // socket create and verification 
    mSockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (mSockfd == -1) 
	{ 
        std::cout << "socket creation failed..." << std::endl; 
        result = -1;
		return false;
    } 
    else
	{
		int enable = 1;																	// avoid "socket bind failed with 98" after program restart
		result = setsockopt(mSockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
		if (result < 0)
		{
			std::cout << "setsockopt(SO_REUSEADDR) failed ..." << std::endl; 
		}
		
		BindToSocket(mSockfd,port);
	}
	return true;
}

void NetworkManagement::RunServerLoop()
{
	int result = 0;
	while(result == 0 and keepRunning==true)
	{
		result = ListenOnSocket(mSockfd);
		if(result != -1 and keepRunning==true)
		{
			int newConnection = AcceptOnSocket(mSockfd);
			if(newConnection >= 0)
			{
				StartThread(newConnection);
			}
			else
			{
				result = -1;
			}
		}
	}
}
	
void NetworkManagement::ShutdownSocket()
{
	shutdown(mSockfd, SHUT_RD);
}

void NetworkManagement::CloseSocket()
{
	if(mSockfd!=-1)
	{
		if(close(mSockfd)== -1)
		{
			std::cout << "error in network manager closing... " << errno << std::endl;
		}
		mSockfd = -1;
	}
}

void NetworkManagement::ShutdownThreads()
{
	mHandlerMutex.lock();
	for(std::list<ClientHandler*>::iterator it = mHandler.begin();it != mHandler.end();++it)
	{ 
		ClientHandler* pHandler = (*it);
		pHandler->ShutdownSocket();
	}
	mHandlerMutex.unlock();
}

void NetworkManagement::BlockUntilAllThreadsFinished(unsigned int timeout)
{
	while(mHandler.empty() == false and timeout>0)
	{
		sleep(1);
		timeout--;
	}
	if(mHandler.empty() == false)
	{
		std::cout << "not all connection closed!!!" << std::endl;
	}
}

int NetworkManagement::BindToSocket(int sockfd,int port)
{
	struct sockaddr_in servaddr; 
    memset(&servaddr,0, sizeof(servaddr)); 
  
    // assign IP, PORT 
    servaddr.sin_family 		= AF_INET; 
    servaddr.sin_addr.s_addr 	= htonl(INADDR_ANY); 
    servaddr.sin_port 			= htons(port); 
    // Binding newly created socket to given IP and verification 
    if ((bind(sockfd, (sockaddr*)&servaddr, sizeof(servaddr))) != 0) 
	{ 
        std::cout << "socket bind failed with " << errno << std::endl; 
        return -1;
    } 
	return 0;
}

int NetworkManagement::ListenOnSocket(int sockfd)
{
	// Now server is ready to listen and verification
	const int maxPendingConnection = 16;
	if ((listen(sockfd, maxPendingConnection)) != 0) 
	{ 
		std::cout << "listen failed..." << std::endl; 
		return -1; 
	} 
	return 0;
}

int NetworkManagement::AcceptOnSocket(int sockfd)
{
	struct sockaddr_in cli; 
	int len = sizeof(cli); 
		  
	// Accept the data packet from client and verification 
	int connfd = accept(sockfd, (sockaddr*)(&cli),(socklen_t*) &len); 
	if (connfd < 0) 
	{ 
		std::cout << "server acccept failed..." << std::endl; 
	} 
	else
	{
		time_t rawtime;
		struct tm * timeinfo;

		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		char ipaddr[INET_ADDRSTRLEN];
		inet_ntop( AF_INET, &cli.sin_addr, ipaddr, sizeof( ipaddr ));
		std::cout << "server acccept the client " << ipaddr << " nr " << mHandler.size() << " "  << asctime (timeinfo) << std::endl;
	}
	return connfd;
}

void NetworkManagement::StartThread(int socketToClient)
{
	ClientHandler * pHandler = new ClientHandler(socketToClient);
	if(pHandler==nullptr)
	{
		std::cout << "memory allocation failed!!!" << std::endl;
		return;
	}
	try
	{
		std::thread thread(NetworkManagement::ConnectionHandler, pHandler);
		thread.detach();
		mHandler.push_back(pHandler);
	}
	catch(...)
	{
		delete pHandler;
		std::cout << "thread creation failed!!!" << std::endl;
	}
}

void NetworkManagement::RemoveFromHandlerList(ClientHandler * targetHandle)
{
	bool found = false;
	mHandlerMutex.lock();
	for(std::list<ClientHandler*>::iterator it = mHandler.begin();it != mHandler.end();++it)
	{ 
		ClientHandler* pHandler = (*it);
		if(pHandler==targetHandle)
		{
			mHandler.erase(it);
			found = true;
			break;
		}
	}
	mHandlerMutex.unlock();
	if(found==false)
		std::cout << "warning client handler not found!!!" << std::endl;
}


void* NetworkManagement::ConnectionHandler(ClientHandler* pCHandler)
{
	pCHandler->ServeClient();
	RemoveFromHandlerList(pCHandler);
	delete pCHandler;
	return NULL;
}

