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

#ifndef HTTPHEADERREQUEST_H
#define HTTPHEADERREQUEST_H

#include <istream>
#include <map>
#include "UniformResourceIdentifier.h"

enum requestType {UNKOWN, GET, HEAD, POST, PUT, DELETE, TRACE, CONNECT};

class HttpHeaderRequest: public UniformResourceIdentifier
{
public:
	HttpHeaderRequest();
	virtual ~HttpHeaderRequest();
	
	bool 		parseHeader(std::istream& input);
	std::string getArgument(std::string label);
	requestType getRequest();
	std::string getUri();
	
	bool Exists(const std::string &name);
private:
	void Clear();
	bool parseRequestLine(std::string &line);
	bool parseRequestType(const std::string &reqType);
	void appendPair(const std::string &label,const std::string &content);
	
	std::map<std::string,std::string> mArgumentPairs;
	requestType mRequest;
	std::string mHttpVersion;
	std::string mURI;

};

#endif // HTTPHEADERREQUEST_H
