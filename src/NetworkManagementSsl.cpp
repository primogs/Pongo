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
#include <openssl/ssl.h>
#include <openssl/err.h>

extern volatile bool keepRunning;

int NetworkManagementSsl::mSockfd=0;
SSL_CTX * NetworkManagementSsl::mSslContext= nullptr;
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
	SSL_load_error_strings();	
    OpenSSL_add_ssl_algorithms();

	const SSL_METHOD *method;

    method = SSLv23_server_method();

    mSslContext = SSL_CTX_new(method);
    if (!mSslContext) 
	{
		std::cout << "ssl server failed..." << std::endl;
		return false;
    }
	
	SSL_CTX_set_ecdh_auto(mSslContext, 1);

    std::string path;
	
	path = mBaseFolder+mCertName+".cert.pem";
    if (SSL_CTX_use_certificate_file(mSslContext, path.c_str(), SSL_FILETYPE_PEM) <= 0) 
	{
        std::cout << "tls_config_set_cert_file failed..." << std::endl;
    }
	path = mBaseFolder+mCertName+".pkey.pem";
    if (SSL_CTX_use_PrivateKey_file(mSslContext, path.c_str(), SSL_FILETYPE_PEM) <= 0 ) 
	{
		std::cout << "tls_config_set_key_file failed..." << std::endl;
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
		
		
		struct timeval tv;
		tv.tv_sec = 20;  // 30 Secs Timeout to  avoid blocking
		setsockopt(mSockfd, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,sizeof(struct timeval));
		if (result < 0)
		{
			std::cout << "setsockopt(SO_RCVTIMEO) failed ..." << std::endl; 
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
			SSL* sslConnection = NULL;
			uint32_t ip_address = 0;
			int newConnection = NetworkManagementSsl::AcceptOnSocket(mSockfd,ip_address,&sslConnection);
			if(newConnection >=0 and sslConnection != nullptr)
			{
				StartThread(sslConnection,newConnection,ip_address);
			}
		}
	}
}

void NetworkManagementSsl::CloseSocket()
{
	if(mSockfd!=-1)
	{
		if(close(mSockfd)== -1)
		{
			std::cout << "error in network manager ssl closing socket... " << std::endl;
			std::cout << '\t' << strerror(errno) << std::endl;
		}
		mSockfd = -1;
	}
	if(mSslContext != nullptr)
	{
		SSL_CTX_free(mSslContext);
		mSslContext = nullptr;
	}
}

void NetworkManagementSsl::ShutdownThreads()
{
	std::list<ClientHandler*> & handler = NetworkManagerBase::GetHandlerList();
	for(std::list<ClientHandler*>::iterator it = handler.begin();it != handler.end();++it)
	{ 
		ClientHandler* pHandler = (*it);
		pHandler->CloseSocket();
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
        std::cout << "socket bind failed with " << std::endl; 
		std::cout << '\t' << strerror(errno) << std::endl;
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
		std::cout << "ssl server listen failed... " << std::endl; 
		std::cout << '\t' << strerror(errno) << std::endl;
		return -1; 
	}
	return 0;
}

int NetworkManagementSsl::AcceptOnSocket(int sockfd,uint32_t &ip_address,SSL **sslConnection)
{
	*sslConnection = nullptr;
	struct sockaddr_in cli;
	int len = sizeof(cli);
		  
	// Accept the data packet from client and verification 
	int connfd = accept(sockfd, (sockaddr*)(&cli),(socklen_t*) &len); 
	if (connfd < 0) 
	{ 
		if(errno == EAGAIN)
			return -1;
			
		std::cout << "ssl server acccept failed... " << std::endl; 
		std::cout << '\t' << strerror(errno) << std::endl;
	} 
	else
	{
		ip_address = cli.sin_addr.s_addr;
		if(NetworkManagerBase::IsBlacklisted(ip_address)==true)
		{
			if(close(connfd)== -1)
			{
				std::cout << "error in client handler closing socket... " << std::endl;
				std::cout << '\t' << strerror(errno) << std::endl;
			}
			connfd = -1;
		}
		else
		{
			int result  = 0;
			
			if(mSslContext==nullptr)
			{
				HandleError("mSslContext invalid... ",connfd);
				return -1;
			}
			
			*sslConnection = SSL_new(mSslContext);
			if((*sslConnection) == nullptr)
			{
				HandleError("SSL_new failed... ",connfd);
				return -1;
			}
				
			result = SSL_set_fd(*sslConnection, connfd);
			if (result <= 0)
			{
				HandleError("SSL_set_fd failed... ",connfd);
				return -1;
			}
			
			result = SSL_accept(*sslConnection);
			if (result <= 0)
			{
				HandleErrorSSL("SSL_accept failed... ",connfd,sslConnection,result);
				return -1;
			}
				
			const time_t  now = time(nullptr);
			const std::tm * ptm = std::localtime(&now);
			char buffer[32];
			std::strftime(buffer, 32, "%d.%m.%Y %H:%M:%S", ptm);

			char ipaddr[INET_ADDRSTRLEN];
			inet_ntop( AF_INET, &cli.sin_addr, ipaddr, sizeof( ipaddr ));
			std::cout << "server acccept ssl client " << ipaddr << " nr " << NetworkManagerBase::GetHandlerListSize() << " " << buffer  << std::endl;
		}
	}
	return connfd;
}

void NetworkManagementSsl::StartThread(SSL *sslToClient,int socketToClient,uint32_t ip_address)
{
	ClientHandler * pHandler = new ClientHandler(socketToClient,ip_address);
	if(pHandler==nullptr)
	{
		std::cout << "memory allocation failed!!!" << std::endl;
		return;
	}
		
	pHandler->SetSocketOfClient(sslToClient);
	try
	{
		std::thread thread(NetworkManagementSsl::ConnectionHandler, pHandler);
		thread.detach();
	}
	catch(...)
	{
		delete pHandler;
		std::cout << "thread creation failed!!!" << std::endl;
	}
}


void* NetworkManagementSsl::ConnectionHandler(ClientHandler* pCHandler)
{
	NetworkManagerBase::AddClientHandler(pCHandler);
	pCHandler->ServeClient();
	NetworkManagerBase::RemoveFromHandlerList(pCHandler);
	delete pCHandler;
	return NULL;
}

void NetworkManagementSsl::HandleErrorSSL(std::string msg,int connfd,SSL **sslConnection,int result)
{
	std::cout << msg << std::endl;
	
	int sslError = SSL_get_error(*sslConnection,result);
	if(sslError == SSL_ERROR_SYSCALL)
		std::cout << '\t' << strerror(errno) << std::endl;
	else
		std::cout << '\t' << sslError << std::endl;
	
	if(connfd != -1)
	{
		if(close(connfd)== -1)
		{
			std::cout << "ssl error in client handler closing socket... "  << std::endl;
			std::cout << '\t' << strerror(errno) << std::endl;
		}
		connfd = -1;
	}
}

void NetworkManagementSsl::HandleError(std::string msg,int connfd)
{
	std::cout << msg << std::endl;
	int result = 0;
	while ((result = ERR_get_error()))
	{
		char * err_msg = ERR_error_string(result, NULL);
		std::cout << '\t' << err_msg << std::endl;
	}
	if(connfd != -1)
	{
		if(close(connfd)== -1)
		{
			std::cout << "ssl error in client handler closing socket... " << std::endl;
			std::cout << '\t' << strerror(errno) << std::endl;
		}
		connfd = -1;
	}
}


