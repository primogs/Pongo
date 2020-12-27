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
#include "HttpVariables.h"

HttpVariables::HttpVariables()
{
}

HttpVariables::~HttpVariables()
{
}

void HttpVariables::Parse(const std::string &query)
{
	mVariables.clear();
	
	size_t equalpos = query.find_first_of('=');
	size_t andpos = std::string::npos;
	if(equalpos != std::string::npos)
	{
		andpos = query.find_first_of('&',equalpos+1);
	}
	size_t begin = 0;
	while(equalpos != std::string::npos)
	{
		size_t end = equalpos;
		std::string name = query.substr(begin,end-begin);
		std::string value;
		begin = end+1;
		if(andpos == std::string::npos)
		{
			end=query.length();
			value = query.substr(begin,end-begin);
			
			equalpos = std::string::npos;
		}
		else
		{
			end=andpos;
			value = query.substr(begin,end-begin);
			begin = end+1;
			
			equalpos = query.find_first_of('=',end+1);
			if(equalpos != std::string::npos)
			{
				andpos = query.find_first_of('&',equalpos+1);
			}
			else
			{
				andpos = std::string::npos;
			}
		}
		mVariables[name] = value;
	}
}

bool HttpVariables::Exists(const std::string& name)
{
    return mVariables.find(name) != mVariables.end();
}

std::string HttpVariables::getVariable(const std::string& name)
{
	return mVariables[name];
}