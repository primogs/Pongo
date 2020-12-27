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

#ifndef ELMTCANVAS_H
#define ELMTCANVAS_H

#include "ElmtBase.h"
#include <vector>
#include <list>
#include <tuple>
#include <cmath>
#include <math.h>
#include <iomanip>
#include <iostream>

class ElmtCanvas: public ElmtBase
{
public:
	explicit ElmtCanvas(const std::string &id);
	virtual ~ElmtCanvas();
	
	void ClearGraph();
	
	void  Paint(std::stringstream &site,HttpVariables &hVar) override;

	void SetCanvasSize(int width,int height);
	void DisableTimeline();
	
	void SetYLabel(std::list< std::tuple <double,std::string> > &labelList);
	
	void AddDataPoint(double x,double y, unsigned short graph=0);
	
	void AddUnit(std::string yAxis,std::string xAxis);
	void AddLegende(std::vector<std::string> label);
private:
	int MaxLabelInXRange();
	int MaxLabelInYRange();
	
	void DrawScaleX(std::stringstream &sstr);
	void DrawScaleY(std::stringstream &sstr);
	void DrawLegende(std::stringstream &sstr,unsigned int n);
	
	int MapX(double x, int pxmin, int pxmax);
	int MapY(double y, int pymin, int pymax);
	
	void DrawLabelX(std::stringstream &sstr,double x);
	void DrawLabelY(std::stringstream &sstr,double y);
	void DrawLabelY(std::stringstream &sstr,double y,std::string label);
	int Interpolate(double a,double amin,double amax,int bmin,int bmax);
	int InterpolateInverse(double a,double amin,double amax,int bmin,int bmax);
	
	void DrawGraph(std::stringstream &sstr,std::list< std::tuple <double,double> > &graph);
	std::vector< std::list< std::tuple <double,double> > > mGraph;
	
	int mCanvasWidth;
	int mCanvasHeight;
	int mFontSize = 20;
	int mBoarder = 5;
	int mScaleWidth;
	int mScaleHeight;
	
	double mMinX;
	double mMaxX;
	double mMinY;
	double mMaxY;
	
	std::string mId;
	
	bool mTime;
	
	std::list< std::tuple <double,std::string> > mLabelList;
	std::vector<std::string> mLegende;
	
	std::string mUnitOfXAxis;
	std::string mUnitOfYAxis;
	
	const std::vector<std::string> mPallet = {"000000","0000FF","00FF00","FF0000"};
};

#endif // ELMTCANVAS_H
