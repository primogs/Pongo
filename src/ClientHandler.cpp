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

#include "ClientHandler.h"
#include "NetworkManagerBase.h"

#define CLOSE_THREAD (-1)
#define REPEAT_READ (0)

ClientHandler::ClientHandler(int socket,uint32_t ip_addr):
mClientSock(socket), mSslConnection(nullptr),mIpAddr(ip_addr)
{		
	char ipaddr[INET_ADDRSTRLEN];
	inet_ntop( AF_INET, &mIpAddr, ipaddr, sizeof( ipaddr ));
	mIpStr = ipaddr;
}

ClientHandler::ClientHandler(SSL* connection,int socket,uint32_t ip_addr):
mClientSock(socket), mSslConnection(connection),mIpAddr(ip_addr)
{
	char ipaddr[INET_ADDRSTRLEN];
	inet_ntop( AF_INET, &mIpAddr, ipaddr, sizeof( ipaddr ));
	mIpStr = ipaddr;
}

ClientHandler::~ClientHandler()
{
}

void ClientHandler::CloseSocket()
{
	NetworkManagerBase::CloseSocket(mClientSock,&mSslConnection);
}

time_t ClientHandler::GetStartupTime()
{
	return mStartupTime;
}

ssize_t ClientHandler::Write(const void *buffer, size_t n)
{
	if(mSslConnection != nullptr)
	{
		const char *cbuffer = reinterpret_cast<const char *>(buffer);	// to avoid "warning: pointer of type ‘void *’ used in arithmetic"
		while (n > 0) 
		{
			ssize_t ret; 
			ret = SSL_write(mSslConnection, cbuffer, n);
			
			if (ret <= 0)
			{
				CatchSslError(ret,"tls_write failed with ");
				return n;
			}
			cbuffer += ret;
			n -= ret;
		}
	}
	else if(mClientSock != -1)
		return write(mClientSock, buffer, n);
	return 0;
}

ssize_t ClientHandler::Read(void *buffer, size_t n)
{
	int res = 0;
	if(mSslConnection != nullptr)
	{
		res = SSL_read(mSslConnection, buffer, n);
		if(res <= 0)	 
			res = CatchSslError(res,"recved failed with ");
		else
			res = n;
	}
	else if(mClientSock != -1)
	{
		res = read(mClientSock, buffer, n);
		if(res==CLOSE_THREAD and errno != 0)
		{
			std::cout << "recved failed with " << std::endl;
			std::cout << '\t' << strerror(errno) << std::endl;
		}
		else if(res==0)
		{
			res = CLOSE_THREAD;
		}
	}
	return res;
}

int ClientHandler::CatchSslError(int res,std::string msg)
{
	int err = SSL_get_error(mSslConnection,res);
	res = CLOSE_THREAD;
	switch (err) 
	{
		case SSL_ERROR_WANT_READ:
		case SSL_ERROR_WANT_WRITE:
			res = REPEAT_READ;	// don't close thread
			break;

		case SSL_ERROR_ZERO_RETURN:
		case SSL_ERROR_SYSCALL:
			SSL_free(mSslConnection);	// no further I/O operations should be performed on the connection
			mSslConnection = nullptr;	
			break;

		default:
			while ((err = ERR_get_error())) 
			{
				char * err_msg = ERR_error_string(err, NULL);
				std::cout << msg << err_msg << std::endl;
			}
	}
	return res;
}

void ClientHandler::ServeClient()
{
	const int bufferSize = 4096;
    char buff[bufferSize];
	
	mStartupTime = time(nullptr);

    // infinite loop
    while(true) 
	{
        // read the message from client and copy it in buffer 
		const ssize_t recved = Read(buff, bufferSize-1);
		if(recved==-1)
		{
			std::cout << "recved connection closed" << std::endl; 
			break;
		}
		else if(recved==0)
		{
			break;
		}
		else if(recved<bufferSize)
		{
			buff[recved]='\0';

			std::stringstream sstr;
			sstr << buff;
			Process(sstr);
		}
    }
	CloseSocket();
}

void ClientHandler::Process(std::stringstream &sstr)
{
	if(mRequest.parseHeader(sstr)==false)
	{
		SendBadRequest();
	}
	else
	{
		switch(mRequest.getRequest())
		{
			case GET:
			{
				const std::string path = mRequest.getPath();
				if(ResourceManager::isAvailable(path))
				{
					LogRequest(path,true);
					size_t resSize=0;
					char* resData = ResourceManager::Get(path,resSize);
					if(resData != nullptr)
					{
						Write(resData, resSize);
					}
					else
					{
						SendTooManyRequests();
					}
				}
				else
				{
					ProcessHtmlRequest(mRequest.getQuery());
				}
			}
			break;
			case POST:
			{
				ProcessHtmlRequest(sstr.str());
			}
			break;
			default:
			{
				SendNotImplemented();
			}
		}
	}
}

void ClientHandler::LogRequest(std::string req,bool vaild)
{
	std::string::size_type pos = 0;
	while ( ( pos = req.find ("\n",pos) ) != std::string::npos )
	{
		req.replace ( pos, 1,";");
	}
	std::cout << "request(" << mIpStr << ";" << vaild << "): " << req << std::endl;
}

void ClientHandler::ProcessHtmlRequest(std::string varStr)
{
	mVariables.Parse(varStr);
	const std::string content = mWebsite.GenerateHtml(mRequest.getPath(),mVariables);
	if(mWebsite.isAvailable())
	{
		LogRequest(mRequest.getPath(),true);
		SendHtmlPage(content);
	}
	else
	{
		LogRequest(mRequest.getPath(),false);
		SendNotFound();
		NetworkManagerBase::AddToBlacklist(mIpAddr);
	}
}

std::string ClientHandler::BuildResponse(std::string &htmlText,httpStatus status)
{
	std::stringstream sstr;
	HttpHeaderResponse resHeader;
	resHeader.addArgument("Server","Pongo");
	resHeader.addArgument("Content-type","text/html; charset=utf-8");
	resHeader.addArgument("Connection","keep-alive");
	resHeader.addArgument("Content-length",htmlText.length());
	resHeader.getHeader(sstr,status);
	sstr << htmlText;
	return sstr.str();
}

void ClientHandler::SendHtmlPage(std::string htmlText)
{
	std::string res = BuildResponse(htmlText);
	Write(res.c_str(), res.length()); 
}

std::string ClientHandler::GenerateErrorPage(const std::string &errorMsg)
{
	std::stringstream sstr;
	sstr << "<!DOCTYPE html>\n\
	<html>\n\
	<head>\n\
	<meta http-equiv='content-type' content='text/html; charset=utf-8'>\n\
	<title>" << errorMsg << "</title>\n\
	</head>\n\
	<body>\n\
	<h1>" << errorMsg << "</h1>\n\
	</body>\n\
	</html>\n";
	
	return sstr.str();
}

void ClientHandler::SendNotImplemented()
{
	std::string page = GenerateErrorPage("Not Implemented 501");
	std::string resp =  BuildResponse(page,NotImplemented);
	Write(resp.c_str(), resp.length());
}

void ClientHandler::SendBadRequest()
{	
	std::string page = GenerateErrorPage("Bad Request 400");
	std::string resp =  BuildResponse(page,BadRequest);
	Write( resp.c_str(), resp.length());
}

void ClientHandler::SendNotFound()
{
	std::string page = GenerateErrorPage("Not Found 404");
	std::string resp =  BuildResponse(page,NotFound);
	Write(resp.c_str(), resp.length());
}


void ClientHandler::SendTooManyRequests()
{
	std::string page = GenerateErrorPage("Too Many Requests 429");
	std::string resp =  BuildResponse(page,TooManyRequests);
	Write(resp.c_str(), resp.length());
}
