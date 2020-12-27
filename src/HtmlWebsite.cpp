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
#include "HtmlWebsite.h"
#include <sstream>
#include "PageRoot.h"
#include "PageSecond.h"
#include "PageImpressum.h"


HtmlWebsite::HtmlWebsite()
{
	mAvail = false;
	
	HtmlPage::SetWebsiteTitle("Example Website");
	
	PageRoot *pr = new PageRoot;
	mPages.push_back(pr);
	
	PageImpressum *pi = new PageImpressum;
	mPages.push_back(pi);
	
	PageSecond *p1 = new PageSecond;
	mPages.push_back(p1);
	
	if(HtmlPage::isPageReferenceSet()==false)
	{
		HtmlPage::AddPageReference(pr,"Home");
		HtmlPage::AddPageReference(p1,"Second");
	}
}

HtmlWebsite::~HtmlWebsite()
{
	for(std::list<HtmlPage*>::iterator it = mPages.begin();it != mPages.end();++it)
	{
		delete *it;
	}
	mPages.clear();
}


std::string HtmlWebsite::GenerateHtml(std::string path,HttpVariables &hVar)
{
	mAvail = true;
	std::stringstream site;
	
	std::list<HtmlPage*>::iterator it = mPages.begin();
	while(it != mPages.end() and ((*it)->AreYou(path)==false))
	{
		++it;
	}
	
	if(it != mPages.end())
	{
		(*it)->GetPage(site,hVar);
	}
	else
	{
		mAvail = false;
	}
	return site.str();
}

bool HtmlWebsite::isAvailable()
{
	return mAvail;
}
