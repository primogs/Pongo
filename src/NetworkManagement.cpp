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
std::list<std::tuple<pthread_t,ClientHandler*> > NetworkManagement::mHandler;

int NetworkManagement::mSockfd=0;

NetworkManagement::NetworkManagement()
{
}

NetworkManagement::~NetworkManagement()
{
}
// SO_REUSEADDR
void NetworkManagement::StartWebServer(int port)
{
	int result = 0;
    // socket create and verification 
    mSockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (mSockfd == -1) 
	{ 
        std::cout << "socket creation failed..." << std::endl; 
        result = -1; 
    } 
    else
	{
        std::cout << "Socket successfully created.." << std::endl;
		
		int enable = 1;																	// avoid "socket bind failed with 98" after program restart
		result = setsockopt(mSockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
		if (result < 0)
		{
			std::cout << "setsockopt(SO_REUSEADDR) failed ..." << std::endl; 
		}
		
		result = BindToSocket(mSockfd,port);
	}
	
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
			CheckForThreadsFinished();
		}
	}
	
	ShutdownThreads();
	BlockUntilAllThreadsFinished();
	CloseSocket();
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

void NetworkManagement::CheckForThreadsFinished()
{
	std::list<std::list<std::tuple<pthread_t,ClientHandler*>>::iterator> threadsfinished;
	for(std::list<std::tuple<pthread_t,ClientHandler*>>::iterator it = mHandler.begin();it != mHandler.end();++it)
	{ 
		void *retval;
		if(pthread_tryjoin_np(std::get<0>(*it),&retval)==0)
		{
			std::cout << "found thread that has finished\n" << std::endl;
			ClientHandler* pHandler = std::get<1>(*it);
			
			pHandler->CloseSocket();
				
			delete pHandler;
			threadsfinished.push_back(it);
		}
	}
	for(std::list<std::list<std::tuple<pthread_t,ClientHandler*>>::iterator>::iterator it = threadsfinished.begin();it != threadsfinished.end();++it)
	{ 
		mHandler.erase(*it);
	}
}

void NetworkManagement::ShutdownThreads()
{
	for(std::list<std::tuple<pthread_t,ClientHandler*>>::iterator it = mHandler.begin();it != mHandler.end();++it)
	{ 
		ClientHandler* pHandler = std::get<1>(*it);
		pHandler->ShutdownSocket();
	}
}

void NetworkManagement::BlockUntilAllThreadsFinished()
{
	while(mHandler.empty() == false)
	{
		CheckForThreadsFinished();
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
    else
	{
        std::cout << "Socket successfully binded.." << std::endl;
	}
	return 0;
}

int NetworkManagement::ListenOnSocket(int sockfd)
{
	// Now server is ready to listen and verification
	const int maxPendingConnection = 16;
	if ((listen(sockfd, maxPendingConnection)) != 0) 
	{ 
		std::cout << "Listen failed..." << std::endl; 
		return -1; 
	} 
	else
	{
		std::cout << "Server listening.." << std::endl; 
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
		std::cout << "server acccept the client..." << std::endl;
	}
	return connfd;
}

void NetworkManagement::StartThread(int socketToClient)
{
	ClientHandler * pHandler = new ClientHandler;
	pHandler->SetSocketOfClient(socketToClient);
	pthread_t ThreadHandle = -1;
	if( pthread_create( &ThreadHandle,NULL,&NetworkManagement::ConnectionHandler,(void*)pHandler ) < 0)
	{
		delete pHandler;
		std::cout << "create thread failed..." << std::endl;
	}
	else
	{
		mHandler.push_back(std::make_tuple(ThreadHandle,pHandler));
	}
}


void* NetworkManagement::ConnectionHandler(void *pHandler)
{
	ClientHandler * pCHandler = reinterpret_cast<ClientHandler*>(pHandler);
	pCHandler->ServeClient();
	return NULL;
}

