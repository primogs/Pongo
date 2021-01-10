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
#include <tls.h>

extern volatile bool keepRunning;

ClientHandler::ClientHandler():mSocket(-1), mTlsConnection(nullptr)
{
}

ClientHandler::~ClientHandler()
{
}

void ClientHandler::CloseSocket()
{
	if(mTlsConnection != nullptr)
	{
		tls_free(mTlsConnection);
		mTlsConnection = nullptr;
	}
	if(mSocket != -1)
	{
		if(close(mSocket)== -1)
		{
			std::cout << "error in client handler closing socket... " << errno << std::endl;;
		}
		mSocket = -1;
	}
}

void ClientHandler::ShutdownSocket()
{
	if(mTlsConnection != nullptr)
	{
		if(tls_close(mTlsConnection)<0)
		{
			std::cout << "error in client handler closing tls... " << tls_error(mTlsConnection) << std::endl;
		}
	}
	else
	{
		shutdown(mSocket, SHUT_RD);
	}
}

void ClientHandler::SetSocketOfClient(int socket)
{
	mSocket = socket;
}

void ClientHandler::SetSocketOfClient(struct tls* connection)
{
	mTlsConnection = connection;
}

ssize_t ClientHandler::Write(const void *buffer, size_t n)
{
	if(mTlsConnection != nullptr)
	{
		const char *cbuffer = reinterpret_cast<const char *>(buffer);	// to avoid "warning: pointer of type ‘void *’ used in arithmetic"
		while (n > 0) 
		{
			ssize_t ret; 
			ret = tls_write(mTlsConnection, cbuffer, n);
				
			if (ret == TLS_WANT_POLLIN || ret == TLS_WANT_POLLOUT)
				continue;
			else if (ret == -1)
			{
				std::cout << "tls_write failed with " << tls_error(mTlsConnection) << std::endl;
				return n;
			}
			cbuffer += ret;
			n -= ret;
		}
	}
	else if(mSocket != -1)
		return write(mSocket, buffer, n);
	return 0;
}

ssize_t ClientHandler::Read(void *buffer, size_t n)
{
	int res = 0;
	if(mTlsConnection != nullptr)
	{
		res = tls_read(mTlsConnection, buffer, n);
		if(res==-1)	// 
		{
			std::cout << "recved failed with " << tls_error(mTlsConnection) << std::endl; 
		}
		
	}
	else if(mSocket != -1)
	{
		res = read(mSocket, buffer, n);
		if(res==-1 and errno != 0)	// 
		{
			std::cout << "recved failed with " << errno << std::endl; 
		}
	}
	return res;
}

void ClientHandler::ServeClient()
{
	const int bufferSize = 4096;
    char buff[bufferSize];

    // infinite loop
    while(keepRunning == true) 
	{
        // read the message from client and copy it in buffer 
		const ssize_t recved = Read(buff, sizeof(buff));
		if(recved==-1)
		{
			break;
		}
		else if(recved==0)
		{
			std::cout << "recved connection closed" << std::endl; 
			break;
		}
		else
		{
			buff[recved]='\0';

			std::stringstream sstr;
			sstr << buff;
			Process(sstr);
		}
    }
	CloseSocket();
	pthread_exit(NULL);
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
					int resSize=0;
					char* resData = ResourceManager::Get(path,resSize);
					SendResource(ResourceManager::ContentType(path),resData,resSize);
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
void ClientHandler::ProcessHtmlRequest(std::string varStr)
{
	mVariables.Parse(varStr);
	const std::string content = mWebsite.GenerateHtml(mRequest.getPath(),mVariables);
	if(mWebsite.isAvailable())
	{
		SendHtmlPage(content);
	}
	else
	{
		SendNotFound();
	}
}

std::string ClientHandler::BuildResponse(std::string &htmlText,httpStatus status)
{
	std::stringstream sstr;
	HttpHeaderResponse resHeader;
	resHeader.addArgument("Server","Pongo");
	resHeader.addArgument("Content-type","text/html, text, plain");
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

void ClientHandler::SendResource(std::string type,char * resData,int resSize)
{
	std::stringstream sstr;
	HttpHeaderResponse resHeader;
	resHeader.addArgument("Server","Pongo");
	resHeader.addArgument("Content-type",type);
	resHeader.addArgument("Connection","keep-alive");
	resHeader.addArgument("Content-length",resSize);
	resHeader.getHeader(sstr,OK);
	
	sstr.write(resData,resSize);
	const std::string res = sstr.str();
	Write(res.c_str(), res.size());
}