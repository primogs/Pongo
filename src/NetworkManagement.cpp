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

int NetworkManagement::mServerSock=-1;

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
		std::cout << "\033[0;31m" << "thread1 web server closed\n" << "\033[0m" << std::endl;
	}
}

bool NetworkManagement::Init(int port)
{
	int result = 0;
    // socket create and verification 
    mServerSock = socket(AF_INET, SOCK_STREAM, 0); 
    if (mServerSock == -1) 
	{ 
        std::cout << "socket creation failed..." << std::endl; 
        result = -1;
		return false;
    } 
    else
	{
		int enable = 1;		// avoid "socket bind failed with 98" after program restart
		result = setsockopt(mServerSock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
		if (result < 0)
		{
			std::cout << "setsockopt(SO_REUSEADDR) failed ..." << std::endl; 
		}
		
		BindToSocket(port);
	}
	return true;
}

void NetworkManagement::RunServerLoop()
{
	while(true)
	{
		if(ListenOnSocket() != -1)
		{
			uint32_t ip_address = 0;
			int clientSock = AcceptOnSocket(ip_address);
			if(clientSock >= 0)
			{
				StartThread(clientSock,ip_address);
			}
		}
		else
		{
			break;
		}
	}
	CloseServerSocket();
}

void NetworkManagement::CloseServerSocket()
{
	NetworkManagerBase::CloseSocket(mServerSock,nullptr);
}

int NetworkManagement::BindToSocket(int port)
{
	struct sockaddr_in servaddr; 
    memset(&servaddr,0, sizeof(servaddr)); 
  
    // assign IP, PORT 
    servaddr.sin_family 		= AF_INET; 
    servaddr.sin_addr.s_addr 	= htonl(INADDR_ANY); 
    servaddr.sin_port 			= htons(port); 
    // Binding newly created socket to given IP and verification 
    if ((bind(mServerSock, (sockaddr*)&servaddr, sizeof(servaddr))) != 0) 
	{ 
        std::cout << "socket bind failed with " << errno << std::endl; 
        return -1;
    } 
	return 0;
}

int NetworkManagement::ListenOnSocket()
{
	// Now server is ready to listen and verification
	const int maxPendingConnection = 16;
	if ((listen(mServerSock, maxPendingConnection)) != 0) 
	{ 
		std::cout << "listen failed with:\n\t" << strerror(errno) << std::endl; 
		return -1; 
	} 
	return 0;
}

int NetworkManagement::AcceptOnSocket(uint32_t &ip_address)
{
	struct sockaddr_in cli;
	int len = sizeof(cli);

	// Accept the data packet from client and verification
	int clientSock = accept(mServerSock, (sockaddr*)(&cli),(socklen_t*) &len);
	if (clientSock < 0)
	{
		if(errno == EAGAIN)
			return -1;
			
		std::cout << "server acccept failed..." << std::endl;
		std::cout << '\t' << strerror(errno) << std::endl;
	}
	else
	{	
		ip_address = cli.sin_addr.s_addr;
		if(NetworkManagerBase::IsBlacklisted(ip_address)==true)
		{
			if(close(clientSock)== -1)
			{
				std::cout << "error in client handler closing socket... " << std::endl;
				std::cout << '\t' << strerror(errno) << std::endl;
			}
			clientSock = -1;
		}
		else
		{
			const time_t now = time(nullptr);
			const std::tm * ptm = std::localtime(&now);
			char buffer[32];
			std::strftime(buffer, 32, "%d.%m.%Y %H:%M:%S", ptm);

			char ipaddr[INET_ADDRSTRLEN];
			inet_ntop( AF_INET, &cli.sin_addr, ipaddr, sizeof( ipaddr ));
			std::cout << "\033[0;32m" << "server acccept the client " << ipaddr << " nr " << NetworkManagerBase::GetHandlerListSize() << " "  << buffer << "\033[0m" << std::endl;
		}
	}
	return clientSock;
}

void NetworkManagement::StartThread(int socketToClient,uint32_t ip_address)
{
	ClientHandler * pHandler = nullptr;
	try
	{
		pHandler = new ClientHandler(socketToClient,ip_address);
	}
	catch(...)
	{
		close(socketToClient);
		std::cout << "StartThread memory allocation failed!!!" << std::endl;
		return;
	}
	
	try
	{
		std::thread thread(NetworkManagement::ConnectionHandler, pHandler);
		thread.detach();
	}
	catch(...)
	{
		delete pHandler;
		std::cout << "thread creation failed!!!" << std::endl;
	}
}

void* NetworkManagement::ConnectionHandler(ClientHandler* pCHandler)
{
	NetworkManagerBase::AddClientHandler(pCHandler);
	pCHandler->ServeClient();
	NetworkManagerBase::RemoveFromHandlerList(pCHandler);
	delete pCHandler;
	return NULL;
}

