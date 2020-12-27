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
#include "ElmtPicture.h"

ElmtPicture::ElmtPicture(HtmlPage &parent):mWidth(0),mHeight(0),mParent(parent)
{
	
}

ElmtPicture::~ElmtPicture()
{
}

void ElmtPicture::AddPictureSeries(const std::string &name,const std::string &appendix,const std::vector<std::string> descriptions,int width,int height)
{
	mName = name;
	mAppendix = appendix;
	mDescription = descriptions;
	mWidth = width;
	mHeight = height;
}

void ElmtPicture::Paint(std::stringstream &site,HttpVariables &hVar)
{
	int i=0;
	if(hVar.Exists(mName))
	{
		try
		{
			i = std::stoi(hVar.getVariable(mName));
		}
		catch(...)
		{
			i=0;
		}
		i = GetValidNumber(i);
	}
	site << "<a name='Anker" << mName << "'></a>\n";
	site << "<img src='" << mName << i << mAppendix << "' width='" << mWidth << "' height='" << mHeight << "'><br>\n";
	
	AddDescriptionLine(site,i);
	if(mDescription.size()>1)
	{
		AddSwitcher(site,i);
	}
}

void ElmtPicture::AddDescriptionLine(std::stringstream &site,int currentElmt)
{
	std::stringstream width;
	width << "width:" << mWidth << "px";
	mParent.TableBegin(site,1,width.str());
	std::stringstream description;
	description << "Bild " << mName << ": " << mDescription[currentElmt];
	mParent.TableAdd(site,description.str());
	mParent.TableEnd(site);
}
	
void ElmtPicture::AddSwitcher(std::stringstream &site,int currentElmt)
{
	std::stringstream width;
	width << "width:" << mWidth << "px";
	mParent.TableBegin(site,2,width.str());
	std::stringstream back;
	back << "<a href='" << mParent.GetFullName() <<"?" << mName << "=" << GetValidNumber(currentElmt-1) << "#Anker" << mName <<  "'>zurück</a>";
	mParent.TableAdd(site,back.str());
	std::stringstream further;
	further << "<a href='" << mParent.GetFullName() << "?" << mName << "=" << GetValidNumber(currentElmt+1)<< "#Anker" << mName << "'>weiter</a>";
	mParent.TableAdd(site,further.str(),"text-align:right");
	mParent.TableEnd(site);
}

int ElmtPicture::GetValidNumber(int i)
{
	int mNumberOfElements = mDescription.size();
	i = i%mNumberOfElements;
	if(i<0)
		i +=mNumberOfElements;
	return i;
}