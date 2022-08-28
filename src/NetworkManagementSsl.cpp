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

int NetworkManagementSsl::mSslServerSock=-1;
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
		std::cout << "\033[0;31m" << "thread2 ssl web server closed\n" << "\033[0m" << std::endl;
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
    mSslServerSock = socket(AF_INET, SOCK_STREAM, 0); 
    if (mSslServerSock == -1) 
	{ 
        std::cout << "socket creation failed..." << std::endl; 
        result = -1; 
    } 
    else
	{
		int enable = 1;		// avoid "socket bind failed with 98" after program restart
		result = setsockopt(mSslServerSock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
		if (result < 0)
		{
			std::cout << "setsockopt(SO_REUSEADDR) failed ..." << std::endl; 
		}
		
		struct timeval tv = {0};	// add timeout, otherwise thread will run forever
		tv.tv_sec = 20;
		result = setsockopt(mSslServerSock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
		if (result < 0)
		{
			std::cout << "setsockopt(SO_RCVTIMEO) ssl failed ..." << std::endl; 
		}
		
		BindToSocket(port);
	}
	return true;
}

void NetworkManagementSsl::RunServerLoop()
{
	while(true)
	{
		if(ListenOnSocket() != -1)
		{
			SSL* sslConnection = nullptr;
			uint32_t ip_address = 0;
			int clientSock = NetworkManagementSsl::AcceptOnSocket(ip_address,&sslConnection);
			if(clientSock >=0 and sslConnection != nullptr)
			{
				StartThread(sslConnection,clientSock,ip_address);
			}
		}
		else
		{
			break;
		}
	}
	CloseServerSocket();
}

void NetworkManagementSsl::CloseServerSocket()
{
	NetworkManagerBase::CloseSocket(mSslServerSock,nullptr);
	if(mSslContext != nullptr)
	{
		SSL_CTX_free(mSslContext);
		mSslContext = nullptr;
	}
}

int NetworkManagementSsl::BindToSocket(int port)
{
	struct sockaddr_in servaddr; 
    memset(&servaddr,0, sizeof(servaddr)); 
  
    // assign IP, PORT 
    servaddr.sin_family 		= AF_INET; 
    servaddr.sin_addr.s_addr 	= htonl(INADDR_ANY); 
    servaddr.sin_port 			= htons(port); 
    // Binding newly created socket to given IP and verification 
    if ((bind(mSslServerSock, (sockaddr*)&servaddr, sizeof(servaddr))) != 0) 
	{ 
        std::cout << "socket bind failed with " << std::endl; 
		std::cout << '\t' << strerror(errno) << std::endl;
        return -1;
    } 
	return 0;
}

int NetworkManagementSsl::ListenOnSocket()
{
	// Now server is ready to listen and verification
	const int maxPendingConnection = 16;
	if ((listen(mSslServerSock, maxPendingConnection)) != 0) 
	{
		std::cout << "ssl server listen failed with:\n\t" << strerror(errno) << std::endl;
		return -1; 
	}
	return 0;
}

int NetworkManagementSsl::AcceptOnSocket(uint32_t &ip_address,SSL **sslConnection)
{
	*sslConnection = nullptr;
	struct sockaddr_in cli;
	int len = sizeof(cli);
		  
	// Accept the data packet from client and verification 
	int clientSock = accept(mSslServerSock, (sockaddr*)(&cli),(socklen_t*) &len); 
	if (clientSock < 0) 
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
			NetworkManagerBase::CloseSocket(clientSock,sslConnection);
		}
		else
		{
			int result  = 0;
			
			if(mSslContext==nullptr)
			{
				HandleError("mSslContext invalid... ",clientSock);
				NetworkManagerBase::CloseSocket(clientSock,sslConnection);
				return -1;
			}
			
			*sslConnection = SSL_new(mSslContext);
			if((*sslConnection) == nullptr)
			{
				HandleError("SSL_new failed... ",clientSock);
				NetworkManagerBase::CloseSocket(clientSock,sslConnection);
				return -1;
			}
				
			result = SSL_set_fd(*sslConnection, clientSock);
			if (result <= 0)
			{
				HandleError("SSL_set_fd failed... ",clientSock);
				NetworkManagerBase::CloseSocket(clientSock,sslConnection);
				return -1;
			}
			
			result = SSL_accept(*sslConnection);
			if (result <= 0)
			{
				NetworkManagerBase::CloseSocket(clientSock,sslConnection);
				return -1;
			}
				
			const time_t  now = time(nullptr);
			const std::tm * ptm = std::localtime(&now);
			char buffer[32];
			std::strftime(buffer, 32, "%d.%m.%Y %H:%M:%S", ptm);

			char ipaddr[INET_ADDRSTRLEN];
			inet_ntop( AF_INET, &cli.sin_addr, ipaddr, sizeof( ipaddr ));
			std::cout << "\033[0;32m" << "server acccept ssl client " << ipaddr << " nr " << NetworkManagerBase::GetHandlerListSize() << " " << buffer << "\033[0m" << std::endl;
		}
	}
	return clientSock;
}

void NetworkManagementSsl::StartThread(SSL *sslToClient,int clientSock,uint32_t ip_address)
{
	ClientHandler * pHandler = nullptr;
	try
	{
		pHandler = new ClientHandler(sslToClient,clientSock,ip_address);
	}
	catch(...)
	{
		close(clientSock);
		SSL_free(sslToClient);
		std::cout << "StartThread memory allocation failed!!!" << std::endl;
		return;
	}
		
	try
	{
		std::thread thread(NetworkManagementSsl::ConnectionHandler, pHandler);
		thread.detach();
	}
	catch(...)
	{
		close(clientSock);
		SSL_free(sslToClient);
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


