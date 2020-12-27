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
#include "PageRoot.h"

PageRoot::PageRoot():mCanvasExample("example")
{
	SetFullName("/");
	
	mCanvasExample.AddUnit("[a.u.]","Zeit");
	mCanvasExample.AddDataPoint(0.0,1.0,0);
	mCanvasExample.AddDataPoint(1.0,1.5,0);
	mCanvasExample.AddDataPoint(2.0,-2.0,0);
	mCanvasExample.AddDataPoint(3.0,0.5,0);
	mCanvasExample.AddDataPoint(4.0,4.0,0);
	
	mCanvasExample.DisableTimeline();
}

PageRoot::~PageRoot()
{
}


void PageRoot::GetPage(std::stringstream &site,HttpVariables &hVar)
{
	site << 
"<!DOCTYPE html>\n\
<html>\n\
<head>\n\
<meta http-equiv='content-type' content='text/html; charset=utf-8'>\n\
<title>" << GetName() << "</title>\n\
\n\
</head>\n\
<body>\n";

	AddHeader(site);
	
	
	site << "<h2>Root Page</h2>\n";

	site << "<h2>Canvas Example</h2>\n";
	mCanvasExample.Paint(site,hVar);
	
	AddFooter(site);
	site << 
"</body>\n\
</html>";

}
