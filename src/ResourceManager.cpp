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
#include "ResourceManager.h"
#include <cstring>
#include <fstream>
#include <iostream>


std::string ResourceManager::mBaseFolder = "resources";
std::map<std::string, std::tuple<char*,size_t> > ResourceManager::mCache;
std::mutex ResourceManager::mLoadMutex;

ResourceManager::~ResourceManager()
{
}

void ResourceManager::SetBaseFolder(char* resFolder)
{
	mBaseFolder = resFolder;
	mBaseFolder += "resources";
	std::cout << "res path " << mBaseFolder << std::endl;
}

void ResourceManager::Clear()
{
	for(std::map<std::string, std::tuple<char*,size_t> >::iterator it = mCache.begin();it != mCache.end();++it)
	{
		std::tuple<char*,int> resource = it->second;
		delete[] (std::get<0>(resource));
	}
	mCache.clear();
}
	
bool ResourceManager::isAvailable(const std::string &name)
{
	if(name.length()<5 or name.find("..") != std::string::npos or name.find("./") != std::string::npos)
		return false;
	
	if(InCache(name))
		return true;
		
	return DoesFileExist(name);
}

bool ResourceManager::DoesFileExist(const std::string &name)
{
	std::ifstream file;
	std::string path = mBaseFolder+name;
	file.open(path);
	bool fileExist = file.good();
	file.close();
	return fileExist;
}

char* ResourceManager::Get(const std::string &name, size_t &size)
{
	size = 0;
	if(!InCache(name))
	{
		bool result = false;
		if (mLoadMutex.try_lock())	// load only one file at time
		{
			result = LoadResource(name);
			mLoadMutex.unlock();
		}
		if(result==false)
			return nullptr;
	}
	return GetResourceFromCache(name,size);
}

char* ResourceManager::GetResourceFromCache(const std::string &name, size_t &size)
{
	std::tuple<char*,int> resource = mCache[name];
	size = std::get<1>(resource);
	return std::get<0>(resource);
}

bool ResourceManager::LoadResource(const std::string &name)
{	
	// valid file type ?
	resType type = DetermineType(name);
	if(type == NONE)
		return false;
	
	// open file
	std::string path = mBaseFolder+name;
	std::ifstream file(path,std::ifstream::ate | std::ifstream::binary);
	if(!file.good())
		return false;
	
	// get file size
	std::ifstream::pos_type fileSize = file.tellg(); 
	if(fileSize>0)
	{
		// generate header
		std::string header = GenerateHttpHeader(type,fileSize);
		size_t memSize = header.length() + fileSize;
		
		file.seekg(0,file.beg);
		char *data = nullptr;
		try 
		{
			data = new char[memSize];
		}
		catch(...)
		{
			std::cout << "LoadResource memory allocation failed!!!" << std::endl;
			file.close();
			return false;
		}
		
		// copy header to memory
		memcpy(data,header.c_str(),header.length());
		// copy file content to memory
		file.read(data+header.length(),fileSize);
		mCache[name]= std::make_tuple(data,memSize);
	}
	file.close();
	return true;

}

std::string ResourceManager::GenerateHttpHeader(resType type,size_t size)
{
	std::stringstream sstr;
	HttpHeaderResponse resHeader;
	resHeader.addArgument("Server","Pongo");
	resHeader.addArgument("Content-type",ContentTypeStr(type));
	resHeader.addArgument("Connection","keep-alive");
	resHeader.addArgument("Content-length",size);
	resHeader.getHeader(sstr,OK);
	return sstr.str();
}

bool ResourceManager::InCache(const std::string &name)
{
	return (mCache.find(name) != mCache.end());
}

std::string ResourceManager::ContentTypeStr(const resType &type)
{
	std::string res;
	switch(type)
	{
		case JPEG:
		{
			res = "image/jpeg";
		}
		break;
		case PNG:
		{
			res = "image/png";
		}
		break;
		case ICON:
		{
			res = "image/ico";	// icons should be jpeg
		}
		break;
		case SVG:
		{
			res = "image/svg+xml";	// icons should be jpeg
		}
		break;
		case ZIP:
		{
			res = "application/zip";
		}
		break;
		default:
		{
			res = "";
		}
		break;
	}
	return res;
}

resType ResourceManager::DetermineType(const std::string &name)
{
	 size_t dotpos = name.find_last_of('.');
	 if( dotpos != std::string::npos)
	 {
		std::string appendix = name.substr(dotpos+1,name.length()-dotpos);
		if(appendix== "jpg")
		{
			return JPEG;
		}
		else if(appendix== "png")
		{
			return PNG;
		}
		else if(appendix== "ico")
		{
			return ICON;
		}
		else if(appendix== "svg")
		{
			return SVG;
		}
		else if(appendix== "zip")
		{
			return ZIP;
		}
		
	 }
	 return NONE;
}
 