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

#ifndef RESSOURCEMANAGER_H
#define RESSOURCEMANAGER_H

#include <map>
#include <string>
#include <mutex>

enum resType {NONE,JPEG,PNG,ICON,SVG,ZIP};
class ResourceManager
{
public:
	ResourceManager();
	virtual ~ResourceManager();
	
	static void SetBaseFolder(char* resFolder);
	static void Clear();
	static bool isAvailable(const std::string &name);
	static std::string ContentType(const std::string &name);
	static char * Get(const std::string &name, int &size);
private:
	static char * GetResourceFromCache(const std::string &name, int &size);
	static bool LoadResource(const std::string &name);
	static bool InCache(const std::string &name);
	static resType DetermineType(const std::string &name);

	static std::string mBaseFolder;
	static std::map<std::string, std::tuple<char*,int> > mCache;
	static std::map<std::string,resType > mTypes;
	static std::mutex mLoadMutex;
};

#endif // RESSOURCEMANAGER_H
