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
#include "ElmtCanvas.h"

ElmtCanvas::ElmtCanvas(const std::string &id):mId(id)
{
	mCanvasWidth = 1366;
	mCanvasHeight = 700;
	mFontSize = 20;
	mBoarder = 5;
	mScaleWidth = mFontSize*5;
	mScaleHeight = mFontSize;
	mTimeAxis = true;
		
	mMinX = nan("");
	mMaxX = nan("");
	mMinY = nan("");
	mMaxY = nan("");
}

ElmtCanvas::~ElmtCanvas()
{
}

void ElmtCanvas::ClearGraph()
{
	mGraph.clear();
	
	mMinX = nan("");
	mMaxX = nan("");
	mMinY = nan("");
	mMaxY = nan("");
}

void ElmtCanvas::AddLegende(std::vector<std::string> &label)
{
	mLegende = label;
}

void ElmtCanvas::Paint(std::stringstream &sstr,HttpVariables &hVar)
{
	if(mGraph.size()>0)
	{
	sstr << "<canvas id='" << mId << "' width='"<< mCanvasWidth << "' height='" << mCanvasHeight << "' style='border:1px solid #000000;'>\n\
</canvas>\n\n\
<script>\n\
var c = document.getElementById('" << mId << "');\n\
var ctx = c.getContext('2d');\n\
ctx.font = '" << mFontSize <<"px sans-serif';\n\
ctx.strokeStyle = '#9A9A9A';\n";
	
	DrawScaleX(sstr);
	DrawScaleY(sstr);
	
	sstr << "ctx.fillText('" << mUnitOfXAxis << "', " << mCanvasWidth-mScaleWidth << ", "<< mCanvasHeight- mScaleHeight-2*mBoarder << ");\n";
	sstr << "ctx.fillText('" << mUnitOfYAxis << "', " << mBoarder+mScaleWidth << ", "<< mScaleHeight << ");\n";
	

	unsigned int color = 0;
	for(std::vector< std::list< std::tuple <double,double> > >::iterator it = mGraph.begin();it!=mGraph.end() and color < mPallet.size();++it,++color)
	{
		sstr << "ctx.strokeStyle = '#" << mPallet[color] << "';\n\
ctx.lineWidth = 3;\n";
		DrawLegende(sstr,color);
		DrawGraph(sstr,*it);
	}
	sstr << "</script>\n";
	}
}

void ElmtCanvas::DrawScaleX(std::stringstream &sstr)
{
	int nx = MaxLabelInXRange();
	double incx = (mMaxX-mMinX)/nx;
	double minSpace = incx*0.75;
	for(double x = mMinX;x<(mMaxX-minSpace);x=x+incx)
		DrawLabelX(sstr, x);
	DrawLabelX(sstr, mMaxX);
	
	
}
void ElmtCanvas::DrawScaleY(std::stringstream &sstr)
{
	if(mLabelList.size()>0)
	{
		for(std::list< std::tuple <double,std::string> >::iterator it = mLabelList.begin();it != mLabelList.end();++it)
		{
			double y = std::get<0>(*it);
			if(y <= mMaxY and y >= mMinY)
			{
				std::string label =std::get<1>(*it);
				DrawLabelY(sstr, y,label);
			}
		}
	}
	else
	{
		int ny = MaxLabelInYRange();
		double incy = (mMaxY-mMinY)/ny;
		double minSpace = incy*0.75;
		for(double y = mMinY;y<(mMaxY-minSpace);y=y+incy)
			DrawLabelY(sstr, y);
		DrawLabelY(sstr, mMaxY);
	}
}

int ElmtCanvas::MaxLabelInXRange()
{
	return mCanvasWidth/(9*mFontSize);
}

int ElmtCanvas::MaxLabelInYRange()
{
	return mCanvasWidth/(4*mFontSize);
}

void ElmtCanvas::DrawLabelX(std::stringstream &sstr,double x)
{
	int xpx = Interpolate(x,mMinX,mMaxX,mBoarder+mScaleWidth,mCanvasWidth-mBoarder-mScaleWidth);
	if(mTimeAxis)
	{
		int minutes = std::abs(static_cast<int>(x-mMaxX))/60;
		int hours = minutes/60;
		int days = hours/24;
		minutes = minutes-hours*60;
		hours = hours-days*24;
		sstr << "ctx.fillText('-" << days << ":" << std::setw(2) << std::setfill('0') << hours << ":" << std::setw(2) << std::setfill('0') << minutes << "', " << xpx << ", "<< mCanvasHeight-mBoarder << ");\n";
	}
	else
	{
		sstr << "ctx.fillText('" << std::fixed << std::setprecision(2) << x << "', " << xpx << ", "<< mCanvasHeight-mBoarder << ");\n";
	}
	
	sstr << "ctx.beginPath();\n";
	sstr << "ctx.moveTo(" << xpx <<", "<< mBoarder+mScaleHeight << ");\n";
	sstr << "ctx.lineTo(" << xpx <<", "<< mCanvasHeight-mBoarder-mScaleHeight << ");\n";
	sstr << "ctx.stroke();\n\n";
}

void ElmtCanvas::DrawLabelY(std::stringstream &sstr,double y,std::string &label)
{
	int ypx = InterpolateInverse(y,mMinY,mMaxY,mFontSize+mBoarder,mCanvasHeight-mBoarder-mScaleHeight);
	sstr << "ctx.fillText('" << label << "', " << mBoarder << ", "<< ypx << ");\n";
	
	sstr << "ctx.beginPath();\n";
	sstr << "ctx.moveTo(" << mBoarder+mScaleWidth <<", "<< ypx << ");\n";
	sstr << "ctx.lineTo(" << mCanvasWidth-mBoarder-mScaleWidth <<", "<< ypx << ");\n";
	sstr << "ctx.stroke();\n\n";
}

void ElmtCanvas::DrawLabelY(std::stringstream &sstr,double y)
{
	int ypx = InterpolateInverse(y,mMinY,mMaxY,mFontSize+mBoarder,mCanvasHeight-mBoarder-mScaleHeight);
	sstr << "ctx.fillText('" << std::fixed << std::setprecision(2) << y << "', " << mBoarder << ", "<< ypx << ");\n";
	
	sstr << "ctx.beginPath();\n";
	sstr << "ctx.moveTo(" << mBoarder+mScaleWidth <<", "<< ypx << ");\n";
	sstr << "ctx.lineTo(" << mCanvasWidth-mBoarder-mScaleWidth <<", "<< ypx << ");\n";
	sstr << "ctx.stroke();\n\n";
}

int ElmtCanvas::Interpolate(double a,double amin,double amax,int bmin,int bmax)
{
	if(fabs(amax-amin)<1E-9)
		return bmin;
	return static_cast<int>((((a-amin)*(bmax-bmin))/(amax-amin))+bmin);
}

int ElmtCanvas::InterpolateInverse(double a,double amin,double amax,int bmin,int bmax)
{
	if(fabs(amax-amin)<1E-9)
		return bmax;
	return static_cast<int>(bmax-(((a-amin)*(bmax-bmin))/(amax-amin)));
}

void ElmtCanvas::SetCanvasSize(int width,int height)
{
	mCanvasWidth=width;
	mCanvasHeight=height;
}


void ElmtCanvas::DisableTimeline()
{
	mTimeAxis = false;
}


void ElmtCanvas::SetYLabel(std::list< std::tuple <double,std::string> > &labelList)
{
	mLabelList = labelList;
}

void ElmtCanvas::AddUnit(std::string yAxis,std::string xAxis)
{
	mUnitOfXAxis = xAxis;
	mUnitOfYAxis = yAxis;
}

void ElmtCanvas::AddDataPoint(double x,double y, unsigned short graph)
{
	if(isnan(mMinX))
	{
		mMinX = x;
		mMaxX = x;
		mMinY = y;
		mMaxY = y;
	}
	if(x<mMinX)
		mMinX=x;
	if(x>mMaxX)
		mMaxX=x;
	if(y<mMinY)
		mMinY=y;
	if(y>mMaxY)
		mMaxY=y;
	if(graph >=mGraph.size())
	{
		mGraph.resize(graph+1);
	}
	mGraph[graph].push_back(std::make_tuple(x,y));
}

void ElmtCanvas::DrawGraph(std::stringstream &sstr,std::list< std::tuple <double,double> > &graph)
{
	sstr << "ctx.beginPath();\n";

	std::list< std::tuple <double,double> >::iterator it = graph.begin();
	int x = Interpolate(std::get<0>(*it),mMinX,mMaxX,mBoarder+mScaleWidth,mCanvasWidth-mBoarder-mScaleWidth);
	int y = InterpolateInverse(std::get<1>(*it),mMinY,mMaxY,mBoarder+mScaleHeight,mCanvasHeight-mBoarder-mScaleHeight);
		
	int prex = x;
	int avgy = y;
	int n=1;
	sstr << "ctx.moveTo(" << x <<", "<< y << ");\n";
	for(++it;it != graph.end();++it)
	{
		x = Interpolate(std::get<0>(*it),mMinX,mMaxX,mBoarder+mScaleWidth,mCanvasWidth-mBoarder-mScaleWidth);
		y = InterpolateInverse(std::get<1>(*it),mMinY,mMaxY,mBoarder+mScaleHeight,mCanvasHeight-mBoarder-mScaleHeight);
		if(x != prex)
		{
			sstr << "ctx.lineTo(" << prex <<", "<< avgy/n << ");\n";
			avgy = 0;
			n = 0;
		}
		prex = x;
		avgy += y;
		n++;
	}

	sstr << "ctx.stroke();\n\n";
}

void ElmtCanvas::DrawLegende(std::stringstream &sstr,unsigned int n)
{
	if(n<mLegende.size())
	{
		sstr << "ctx.fillStyle = '#" << mPallet[n] << "';\n\
ctx.fillText('" << mLegende[n] << "', " << mCanvasWidth-mScaleWidth << ", "<< mScaleHeight+mBoarder+((1+n)*(mFontSize+mBoarder)) << ");\n";
	}
}