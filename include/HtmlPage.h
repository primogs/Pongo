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

#ifndef HTMLPAGE_H
#define HTMLPAGE_H

#include <string>
#include <sstream>
#include <vector>
#include "HttpVariables.h"

class HtmlPage
{
public:
	HtmlPage();
	virtual ~HtmlPage();
	
	virtual void GetPage(std::stringstream &site,HttpVariables &hVar)=0;
	std::string GetName();
	std::string GetFullName();
	bool AreYou(const std::string &fullName);
	
	static bool isPageReferenceSet();
	static void AddPageReference(HtmlPage *page,const std::string &name="");
	static void SetWebsiteTitle(const std::string &title="No Title");
protected:
	void SetFullName(const std::string &fullName);
	
	void AddFooter(std::stringstream &site);
	void AddHeader(std::stringstream &site);
	
	void TableBegin(std::stringstream &site,std::vector<std::string> &columnNames,const std::string &style="width:100%");
	void TableNewRow(std::stringstream &site);
public:	
	void TableBegin(std::stringstream &site,int columns,const std::string &style="width:100%");
	void TableEnd(std::stringstream &site);
	void TableAdd(std::stringstream &site,const std::string &element,const std::string &style="");
private:
	void CloseRow(std::stringstream &site);
	void SetName();
	std::string 		mFullName;
	std::string 		mName;
	static std::string 	mPageRef;
	int					mTableColumns;
	int					mCurrentTableColumn;
	bool				mTableRowOpen;
	static std::string  mWebsiteTitle;
	
};

#endif // HTMLPAGE_H
