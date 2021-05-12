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
#include "NetworkManagementSsl.h"
#include <tls.h>

extern volatile bool keepRunning;

int NetworkManagementSsl::mSockfd=0;
struct tls_config * NetworkManagementSsl::mTlsConfig = nullptr;
struct tls * NetworkManagementSsl::mTlsConnection= nullptr;
std::string NetworkManagementSsl::mBaseFolder = "certs/";
std::string NetworkManagementSsl::mCertName;

NetworkManagementSsl::NetworkManagementSsl()
{
}

NetworkManagementSsl::~NetworkManagementSsl()
{
}

void NetworkManagementSsl::SetBaseFolder(char* resFolder)
{
	mBaseFolder = resFolder;
	mBaseFolder += "certs/";
	std::cout << "cert path " << mBaseFolder << std::endl;
}

void NetworkManagementSsl::SetCertName(const char* cert)
{
	mCertName = cert;
}

void NetworkManagementSsl::StartWebServer(int port)
{
	if(Init(port))
	{
		std::cout << "web server listen on port "<< port << std::endl;
		RunServerLoop();
		
		ShutdownThreads();
		BlockUntilAllThreadsFinished(30u);

		CloseSocket();
		std::cout << "ssl web server closed" << std::endl;
	}
}

bool NetworkManagementSsl::Init(int port)
{
	if (tls_init() == -1) 
	{ 
        std::cout << "tls init failed..." << std::endl;
		return false;
    } 

	mTlsConnection =  tls_server();
	if(mTlsConnection==nullptr)
	{
		std::cout << "tls server failed..." << std::endl;
		return false;
	}
	
	mTlsConfig = tls_config_new();
	
	std::string path;
	path = mBaseFolder+mCertName+".pkey.pem";
	if(tls_config_set_key_file(mTlsConfig, path.c_str()) < 0) 
	{
		std::cout << "tls_config_set_key_file failed..." << std::endl;
	}
	
	path = mBaseFolder+mCertName+".cert.pem";
	if(tls_config_set_cert_file(mTlsConfig, path.c_str()) < 0) 
	{
		std::cout << "tls_config_set_cert_file failed..." << std::endl;
	}
	
	if(tls_configure(mTlsConnection,mTlsConfig)< 0) 
	{
		std::cout << "tls_configure failed..." << std::endl;
	}
		
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

void NetworkManagementSsl::RunServerLoop()
{
	int result = 0;
	while(result == 0 and keepRunning==true)
	{
		result = ListenOnSocket(mSockfd);
		if(result != -1 and keepRunning==true)
		{
			struct tls * tlsConnection = NULL;
			uint32_t ip_address = 0;
			int newConnection = NetworkManagementSsl::AcceptOnSocket(mSockfd,ip_address,&tlsConnection);
			if(newConnection >=0 and tlsConnection != nullptr)
			{
				StartThread(tlsConnection,newConnection,ip_address);
			}
		}
	}
}

void NetworkManagementSsl::ShutdownSocket()
{
	if(mTlsConnection != nullptr)
	{
		if(tls_close(mTlsConnection)<0)	// close tls before socket is shutdown
		{
			std::cout << "error in network manager ssl closing tls... " << tls_error(mTlsConnection) << std::endl;
		}
	}
	shutdown(mSockfd, SHUT_RD);
}

void NetworkManagementSsl::CloseSocket()
{
	if(mTlsConnection != nullptr)
	{
		tls_free(mTlsConnection);
		tls_config_free(mTlsConfig);
		
		mTlsConnection = nullptr;
		mTlsConfig = nullptr;
	}
	if(mSockfd!=-1)
	{
		if(close(mSockfd)== -1)
		{
			std::cout << "error in network manager ssl closing socket... " << errno << std::endl;
		}
		mSockfd = -1;
	}
}

void NetworkManagementSsl::ShutdownThreads()
{
	std::list<ClientHandler*> & handler = NetworkManagerBase::GetHandlerList();
	for(std::list<ClientHandler*>::iterator it = handler.begin();it != handler.end();++it)
	{ 
		ClientHandler* pHandler = (*it);
		pHandler->ShutdownSocket();
	}
	NetworkManagerBase::UnlockHandlerList();
}

void NetworkManagementSsl::BlockUntilAllThreadsFinished(unsigned int timeout)
{
	while(NetworkManagerBase::IsHandlerListEmpty() == false and timeout>0)
	{
		sleep(1);
		timeout--;
	}
	if(NetworkManagerBase::IsHandlerListEmpty() == false)
	{
		std::cout << "not all connection closed!!!" << std::endl;
	}
}

int NetworkManagementSsl::BindToSocket(int sockfd,int port)
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

int NetworkManagementSsl::ListenOnSocket(int sockfd)
{
	// Now server is ready to listen and verification
	const int maxPendingConnection = 16;
	if ((listen(sockfd, maxPendingConnection)) != 0) 
	{ 
		std::cout << "Listen failed..." << std::endl; 
		return -1; 
	}
	return 0;
}

int NetworkManagementSsl::AcceptOnSocket(int sockfd,uint32_t &ip_address,struct tls **tlsConnection)
{
	*tlsConnection = nullptr;
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
		ip_address = cli.sin_addr.s_addr;
		if(NetworkManagerBase::IsBlacklisted(ip_address)==true)
		{
			if(close(connfd)== -1)
			{
				std::cout << "error in client handler closing socket... " << errno << std::endl;;
			}
			connfd = -1;
		}
		else
		{
			int result = tls_accept_socket(mTlsConnection,tlsConnection,connfd);
			if (result < 0) 
			{ 
				std::cout << "server acccept failed..." << std::endl; 
			} 
			else
			{
				const std::chrono::time_point<std::chrono::system_clock>  now = std::chrono::system_clock::now();
				const std::time_t t_c = std::chrono::system_clock::to_time_t(now);

				char ipaddr[INET_ADDRSTRLEN];
				inet_ntop( AF_INET, &cli.sin_addr, ipaddr, sizeof( ipaddr ));
				std::cout << "server acccept ssl client " << ipaddr << " nr " << NetworkManagerBase::GetHandlerListSize() << " " << std::put_time(std::localtime(&t_c), "%F %T.\n")  << std::endl;
			}
		}
	}
	return connfd;
}

void NetworkManagementSsl::StartThread(struct tls *tlsToClient,int socketToClient,uint32_t ip_address)
{
	ClientHandler * pHandler = new ClientHandler(socketToClient,ip_address);
	if(pHandler==nullptr)
	{
		std::cout << "memory allocation failed!!!" << std::endl;
		return;
	}
		
	pHandler->SetSocketOfClient(tlsToClient);
	try
	{
		std::thread thread(NetworkManagementSsl::ConnectionHandler, pHandler);
		thread.detach();
		NetworkManagerBase::AddClientHandler(pHandler);
	}
	catch(...)
	{
		delete pHandler;
		std::cout << "thread creation failed!!!" << std::endl;
	}
}


void* NetworkManagementSsl::ConnectionHandler(ClientHandler* pCHandler)
{
	pCHandler->ServeClient();
	NetworkManagerBase::RemoveFromHandlerList(pCHandler);
	delete pCHandler;
	return NULL;
}
