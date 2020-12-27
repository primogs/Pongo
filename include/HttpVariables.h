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

#ifndef HTTPVARIABLES_H
#define HTTPVARIABLES_H

#include <string>
#include <map>

class HttpVariables
{
public:
	HttpVariables();
	virtual ~HttpVariables();
	
	std::string getVariable(const std::string &name);
	
	void Parse(const std::string &query);
	
	bool Exists(const std::string &name);
private:
	std::map<std::string, std::string> mVariables;
};

#endif // HTTPVARIABLES_H
