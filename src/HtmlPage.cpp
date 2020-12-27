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
#include "HtmlPage.h"

std::string HtmlPage::mPageRef;
std::string HtmlPage::mWebsiteTitle;
	
HtmlPage::HtmlPage()
{
	mTableColumns = 0;
	mCurrentTableColumn = 0;
	mTableRowOpen = false;
}

HtmlPage::~HtmlPage()
{
}

std::string HtmlPage::GetName()
{
	return mName;
}

std::string HtmlPage::GetFullName()
{
	return mFullName;
}

bool HtmlPage::AreYou(const std::string &fullName)
{
	return (fullName==GetFullName());
}

void HtmlPage::SetFullName(const std::string &fullName)
{
	mFullName = fullName;
	SetName();
}

void HtmlPage::SetName()
{
	const size_t slashpos =mFullName.find_last_of('/');
	if(slashpos == std::string::npos)
		mName = mFullName;
	else
		mName = mFullName.substr(slashpos+1,mFullName.length()-(slashpos+1));
}

bool HtmlPage::isPageReferenceSet()
{
	return (mPageRef.empty()==false);
}

void HtmlPage::AddPageReference(HtmlPage *page,const std::string &name)
{
	if(page != NULL)
	{
		mPageRef.append(" <a href='");
		mPageRef.append(page->GetFullName());
		mPageRef.append("' title='");
		if(name.empty())
		{
			mPageRef.append(page->GetName());
			mPageRef.append("'>");
			mPageRef.append(page->GetName());
		}
		else
		{
			mPageRef.append(name);
			mPageRef.append("'>");
			mPageRef.append(name);
		}
		mPageRef.append("</a> ");
	}
}

void HtmlPage::SetWebsiteTitle(const std::string &title)
{
	mWebsiteTitle = title;
}

void HtmlPage::AddHeader(std::stringstream &site)
{
	site <<
"<header>\n\
  <h1>" << mWebsiteTitle << "</h1>\n  " << 
  mPageRef
<< "\n</header>\n";
}

void HtmlPage::AddFooter(std::stringstream &site)
{
	site <<
"<footer>\n\
  <p style='text-align:right;'><a href='/impressum'>Impressum & Datenschutz</a></p>\n\
</footer>";
}

void HtmlPage::TableBegin(std::stringstream &site,std::vector<std::string> &columnNames,const std::string &style)
{
	TableBegin(site,columnNames.size(),style);
	TableNewRow(site);
	for(unsigned int i=0;i<columnNames.size();i++)
	{
		site <<"\t\t<th>"<< columnNames[i] << "</th>\n";
	}
	CloseRow(site);
}


void HtmlPage::TableBegin(std::stringstream &site,int columns,const std::string &style)
{
	if(mTableColumns==0)	// open table only if another not already open
	{
		mTableColumns=columns;
		mCurrentTableColumn = 0;
		site << "<table style='" << style << "'>\n";
	}
}

void HtmlPage::TableEnd(std::stringstream &site)
{
	if(mTableColumns>0)	// close table only if open
	{
		CloseRow(site);
		site << "</table>\n";
		mTableColumns=0;
		mCurrentTableColumn = 0;
	}
}

void HtmlPage::TableAdd(std::stringstream &site,const std::string &element,const std::string &style)
{
	mCurrentTableColumn++;
	if(mCurrentTableColumn >= mTableColumns or mTableRowOpen==false)
	{
		TableNewRow(site);
	}
	if(style.length()==0)
		site <<"\t\t<td>"<< element << "</td>\n";
	else
		site <<"\t\t<td style=" << style << ">"<< element << "</td>\n";
}

void HtmlPage::TableNewRow(std::stringstream &site)
{
	CloseRow(site);
	site << "\t<tr>\n";
	mTableRowOpen = true;
	mCurrentTableColumn = 0;
}

void HtmlPage::CloseRow(std::stringstream &site)
{
	if(mTableRowOpen)
		site << "\t</tr>\n";
	mTableRowOpen = false;

}

 

