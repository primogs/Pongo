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
#include "PageImpressum.h"

PageImpressum::PageImpressum()
{
	SetFullName("/impressum");
}

PageImpressum::~PageImpressum()
{
}

void PageImpressum::GetPage(std::stringstream &site,HttpVariables &hVar)
{	
	site << 
	"<!DOCTYPE html>\n\
<html>\n\
<head>\n\
<meta name='robots' content='noindex'>\n\
<meta http-equiv='content-type' content='text/html; charset=utf-8'>\n\
<title>" << GetName() << "</title>\n\
\n\
</head>\n\
<body>\n";

	AddHeader(site);
	site << "\
<h4>Impressum</h4>\n\
</body>\n\
</html>";
}