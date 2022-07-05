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
#include "HttpHeaderRequest.h"
#include <iostream>

HttpHeaderRequest::HttpHeaderRequest(): mRequest(UNKOWN)
{
	
}

HttpHeaderRequest::~HttpHeaderRequest()
{
}

bool HttpHeaderRequest::parseHeader(std::istream &input)
{
	Clear();
	
	std::string line;
	std::getline(input,line);
	if(parseRequestLine(line)==false)
		return false;
	
	std::getline(input,line);
	while(input.good() and line.length()>0 and !(line.length()==1 and line.at(0)=='\r'))
	{
		size_t delimiter = line.find_first_of(':');
		if(delimiter != std::string::npos)
			appendPair(line.substr(0,delimiter),line.substr(delimiter+1,line.length()-delimiter-2));
		std::getline(input,line);
	}
	

	if(UniformResourceIdentifier::Parse(getUri())==false)
		return false;

	return true;
}

void HttpHeaderRequest::appendPair(const std::string &label,const std::string &content)
{
	mArgumentPairs[label] = content;
}

bool HttpHeaderRequest::Exists(const std::string& name)
{
    return mArgumentPairs.find(name) != mArgumentPairs.end();
}

std::string HttpHeaderRequest::getArgument(std::string label)
{
	return mArgumentPairs[label];
}

bool HttpHeaderRequest::parseRequestLine(std::string &line)
{
	size_t firstDelimiter = line.find_first_of(' ');
	size_t lastDelimiter = line.find_last_of(' ');
	if(firstDelimiter == std::string::npos or lastDelimiter == std::string::npos or firstDelimiter == lastDelimiter)
		return false;
		
	if(!parseRequestType(line.substr(0,firstDelimiter)))
		return false;
		
	mHttpVersion = line.substr(lastDelimiter+1,line.length()-lastDelimiter-2);
	if(mHttpVersion.length()<4 or mHttpVersion.substr(0,4)!="HTTP")
		return false;
	mURI = line.substr(firstDelimiter+1,lastDelimiter-firstDelimiter-1);
	return true;
}

bool HttpHeaderRequest::parseRequestType(const std::string &reqType)
{
	bool res = false;
	if(reqType=="GET")
	{
		mRequest=GET;
		res = true;
	}
	else if(reqType=="HEAD")
	{
		mRequest=HEAD;
		res = true;
	}
	else if(reqType=="POST")
	{
		mRequest=POST;
		res = true;
	}
	else if(reqType=="PUT")
	{
		mRequest=PUT;
		res = true;
	}
	else if(reqType=="DELETE")
	{
		mRequest=DELETE;
		res = true;
	}
	else if(reqType=="TRACE")
	{
		mRequest=TRACE;
		res = true;
	}
	else if(reqType=="CONNECT")
	{
		mRequest=CONNECT;
		res = true;
	}
	return res;
}

requestType HttpHeaderRequest::getRequest()
{
	return mRequest;
}

std::string HttpHeaderRequest::getUri()
{
	return mURI;
}

void HttpHeaderRequest::Clear()
{
	mArgumentPairs.clear();
	mRequest = UNKOWN;
	mHttpVersion.clear();
	mURI.clear();
	UniformResourceIdentifier::Clear();
}