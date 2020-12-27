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

#ifndef ELMTPICTURE_H
#define ELMTPICTURE_H

#include "HtmlPage.h"
#include "ElmtBase.h"
#include <vector>

class ElmtPicture : public ElmtBase
{
public:
	explicit ElmtPicture(HtmlPage &parent);
	virtual ~ElmtPicture();
	
	void AddPictureSeries(const std::string &name,const std::string &appendix,const std::vector<std::string> descriptions,int width,int height);
	void Paint(std::stringstream &site,HttpVariables &hVar) override;
private:
	void AddDescriptionLine(std::stringstream &site,int currentElmt);
	void AddSwitcher(std::stringstream &site,int currentElmt);
	int GetValidNumber(int i);
	
	std::string mName;
	std::string mAppendix;
	std::vector<std::string> mDescription;
	int mWidth;
	int mHeight;
	HtmlPage & mParent;
};

#endif // ELMTPICTURE_H
