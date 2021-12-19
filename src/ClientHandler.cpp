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

extern volatile bool keepRunning;

ClientHandler::ClientHandler(int socket,uint32_t ip_addr):
mSocket(socket), mSslConnection(nullptr),mIpAddr(ip_addr)
{
	
}

ClientHandler::~ClientHandler()
{
	ShutdownSocket();
}

void ClientHandler::CloseSocket()
{
	if(mSslConnection != nullptr)
	{
		SSL_shutdown(mSslConnection);
        SSL_free(mSslConnection);
		mSslConnection = nullptr;
	}
	if(mSocket != -1)
	{
		if(close(mSocket)== -1)
		{
			std::cout << "error in client handler closing socket... " << std::endl;
			std::cout << '\t' << strerror(errno) << std::endl;
		}
		mSocket = -1;
	}
}

void ClientHandler::ShutdownSocket()
{
	if(mSslConnection != nullptr)
	{
		int result = 0;
		while(result==0)
		{
			result = SSL_shutdown(mSslConnection);
			usleep(10);
		}
		
		if(result<0)
		{
			long err = ERR_get_error();
			char * err_msg = ERR_error_string(err,NULL);
			std::cout << "error in client handler closing tls... " << err_msg << std::endl;
		}
	}
	else
	{
		shutdown(mSocket, SHUT_RD);
	}
}

time_t ClientHandler::GetStartupTime()
{
	return mStartupTime;
}

void ClientHandler::SetSocketOfClient(SSL* connection)
{
	mSslConnection = connection;
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
	else if(mSocket != -1)
		return write(mSocket, buffer, n);
	return 0;
}

ssize_t ClientHandler::Read(void *buffer, size_t n)
{
	int res = 0;
	if(mSslConnection != nullptr)
	{
		res = SSL_read(mSslConnection, buffer, n);
		if(res <= 0)	 
			CatchSslError(res,"recved failed with ");
		else
			res = n;
	}
	else if(mSocket != -1)
	{
		res = read(mSocket, buffer, n);
		if(res==-1 and errno != 0)
		{
			std::cout << "recved failed with " << std::endl;
			std::cout << '\t' << strerror(errno) << std::endl;
		}
	}
	return res;
}

void ClientHandler::CatchSslError(int res,std::string msg)
{
	int err = SSL_get_error(mSslConnection,res);
	switch (err) 
	{
		case SSL_ERROR_WANT_READ:
		case SSL_ERROR_WANT_WRITE:
			break;

		case SSL_ERROR_ZERO_RETURN:
		case SSL_ERROR_SYSCALL:
			mSslConnection = nullptr;	// SSL_shutdown() must not be called
			ShutdownSocket();
			break;

		default:
			while ((err = ERR_get_error())) 
			{
				char * err_msg = ERR_error_string(err, NULL);
				std::cout << msg << err_msg << std::endl;
			}
	}
}

void ClientHandler::ServeClient()
{
	const int bufferSize = 4096;
    char buff[bufferSize];
	
	mStartupTime = time(nullptr);

    // infinite loop
    while(keepRunning == true) 
	{
        // read the message from client and copy it in buffer 
		const ssize_t recved = Read(buff, bufferSize-1);
		if(recved==-1)
		{
			break;
		}
		else if(recved==0)
		{
			std::cout << "recved connection closed" << std::endl; 
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