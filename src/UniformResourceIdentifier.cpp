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
#include "UniformResourceIdentifier.h"

UniformResourceIdentifier::UniformResourceIdentifier()
{
}

UniformResourceIdentifier::~UniformResourceIdentifier()
{
}

void UniformResourceIdentifier::Clear()
{
	mScheme.clear();
	mAuthority.clear();
	mPath.clear();
	mQuery.clear();
	mFragment.clear();
}

bool UniformResourceIdentifier::Parse(std::string uri)
{
	const size_t colonpos = uri.find_first_of(':');
	const size_t spacepos = uri.find_first_of(' ');
	if(spacepos != std::string::npos)
		return false;
		
	if(colonpos == std::string::npos)
		return ParseRel(uri);
	else
		return ParseAbs(uri);
}

bool UniformResourceIdentifier::ParseAbs(std::string uri)
{
	const size_t colonpos = uri.find_first_of(':');
	const size_t spacepos = uri.find_first_of(' ');
	if(spacepos != std::string::npos or colonpos == std::string::npos)
		return false;
	
	mScheme = uri.substr(0,colonpos);
	
	const size_t authoritypos = uri.find_first_of("://");
	if(authoritypos == std::string::npos)
	{
		ParseRel(uri.substr(colonpos,uri.length()-colonpos));
	}
	else
	{
		const size_t slashpos = uri.find_first_of('/',authoritypos+3);
		if(slashpos==std::string::npos)
			return false;
			
		mAuthority = uri.substr(authoritypos+3,slashpos-(authoritypos+3));
		ParseRel(uri.substr(slashpos,uri.length()-slashpos));
	}
	
	return true;
}


bool UniformResourceIdentifier::ParseRel(std::string uri)
{
	if(uri.empty())
		return false;
		
	const size_t 	questionmarkpos = uri.find_first_of('?');
	size_t 		hashtagpos;
	if(questionmarkpos == std::string::npos)
		hashtagpos = uri.find_first_of('#');
	else
		hashtagpos = uri.find_first_of('#',questionmarkpos+1);
	
	size_t pathendpos = uri.length();
	if(questionmarkpos != std::string::npos)
		pathendpos = questionmarkpos;
	else if(hashtagpos != std::string::npos)
		pathendpos = hashtagpos;
	
	mPath = uri.substr(0,pathendpos);
	
	size_t hashfragmentlength = 0;
	if(hashtagpos != std::string::npos)
	{
		mFragment = uri.substr(hashtagpos+1,uri.length()-(hashtagpos+1));
		hashfragmentlength = mFragment.length()+1;
	}
	
	if(questionmarkpos != std::string::npos)
	{
		mQuery = uri.substr(questionmarkpos+1,uri.length()-(questionmarkpos+1)-hashfragmentlength);
	}
	return true;
}

std::string UniformResourceIdentifier::getQuery()
{
	return mQuery;
}

std::string UniformResourceIdentifier::getPath()
{
	return mPath;
}

