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
#include "HttpHeaderResponse.h"

HttpHeaderResponse::HttpHeaderResponse():mHttpVersion("HTTP/1.1")
{
}

HttpHeaderResponse::~HttpHeaderResponse()
{
}

void HttpHeaderResponse::addArgument(const std::string &label,const std::string &content)
{
	mHeader << label << ':' << content << "\r\n";
}

void HttpHeaderResponse::addArgument(const std::string &label,int content)
{
	mHeader << label << ':' << content << "\r\n";
}

void HttpHeaderResponse::getHeader(std::stringstream &stream,httpStatus status)
{
	stream << mHttpVersion << ' ' << status << ' ' << getStatusLabel(status) << "\r\n" << mHeader.str() << "\r\n";
}

const std::string& HttpHeaderResponse::getStatusLabel(httpStatus status)
{
	for(unsigned int i=0;i<statusElments;i++)
	{
		if(status==statusNumber[i])
		{
			return statusLabel[i];
		}
	}
	return statusLabel[2];
}
