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

#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include <netinet/in.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <errno.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <ctime>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "HtmlWebsite.h"
#include "HttpHeaderRequest.h"
#include "HttpHeaderResponse.h"
#include "HttpVariables.h"
#include "ResourceManager.h"

class ClientHandler
{
public:
	ClientHandler(int socket,uint32_t ip_addr);
	virtual ~ClientHandler();
	
	void CloseSocket();
	void ShutdownSocket();
	time_t GetStartupTime();
	
	void SetSocketOfClient(SSL* connection);
	void SetParentHandlerList();
	void ServeClient();
private:
	ssize_t Write(const void *buffer, size_t n);
	ssize_t Read(void *buffer, size_t n);

	void Process(std::stringstream &sstr);
	void ProcessHtmlRequest(std::string varStr);
	void SendNotFound();
	void SendBadRequest();
	void SendNotImplemented();
	void SendHtmlPage(std::string htmlText);
	void SendResource(std::string type,char * resData,int resSize);
	std::string GenerateErrorPage(const std::string &errorMsg);
	std::string BuildResponse(std::string &htmlText,httpStatus status=OK);
	void CatchSslError(int res,std::string msg);
	
	bool CheckHttpArguments();

	HtmlWebsite 		mWebsite;
	HttpVariables 		mVariables;
	HttpHeaderRequest 	mRequest;
	int 				mSocket;
	SSL* 		        mSslConnection;
	uint32_t 			mIpAddr;
	time_t 				mStartupTime;

};

#endif // CLIENTHANDLER_H
