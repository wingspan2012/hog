/*
 *  Map2DEnvironment.cpp
 *  hog2
 *
 *  Created by Nathan Sturtevant on 4/20/07.
 *  Copyright 2007 Nathan Sturtevant, University of Alberta. All rights reserved.
 *
 */

#include "Map2DEnvironment.h"
#include "FPUtil.h"

MapEnvironment::MapEnvironment(Map *_m)
{
	map = _m;
}

MapEnvironment::~MapEnvironment()
{
	delete map;
}

void MapEnvironment::GetSuccessors(xyLoc &loc, std::vector<xyLoc> &neighbors)
{
	bool up=false, down=false;
	if ((map->getTerrainType(loc.x, loc.y+1) == kGround))
	{
		down = true;
		neighbors.push_back(xyLoc(loc.x, loc.y+1));
	}
	if ((map->getTerrainType(loc.x, loc.y-1) == kGround))
	{
		up = true;
		neighbors.push_back(xyLoc(loc.x, loc.y-1));
	}
	if ((map->getTerrainType(loc.x-1, loc.y) == kGround))
	{
		if ((up && (map->getTerrainType(loc.x-1, loc.y-1) == kGround)))
			neighbors.push_back(xyLoc(loc.x-1, loc.y-1));
		if ((down && (map->getTerrainType(loc.x-1, loc.y+1) == kGround)))
			neighbors.push_back(xyLoc(loc.x-1, loc.y+1));
		neighbors.push_back(xyLoc(loc.x-1, loc.y));
	}
	if ((map->getTerrainType(loc.x+1, loc.y) == kGround))
	{
		if ((up && (map->getTerrainType(loc.x+1, loc.y-1) == kGround)))
			neighbors.push_back(xyLoc(loc.x+1, loc.y-1));
		if ((down && (map->getTerrainType(loc.x+1, loc.y+1) == kGround)))
			neighbors.push_back(xyLoc(loc.x+1, loc.y+1));
		neighbors.push_back(xyLoc(loc.x+1, loc.y));
	}
}

void MapEnvironment::GetActions(xyLoc &loc, std::vector<tDirection> &actions)
{
	bool up=false, down=false;
	if ((map->getTerrainType(loc.x, loc.y+1) == kGround))
	{
		down = true;
		actions.push_back(kS);
	}
	if ((map->getTerrainType(loc.x, loc.y-1) == kGround))
	{
		up = true;
		actions.push_back(kN);
	}
	if ((map->getTerrainType(loc.x-1, loc.y) == kGround))
	{
		if ((up && (map->getTerrainType(loc.x-1, loc.y-1) == kGround)))
			actions.push_back(kNW);
		if ((down && (map->getTerrainType(loc.x-1, loc.y+1) == kGround)))
			actions.push_back(kSW);
		actions.push_back(kW);
	}
	if ((map->getTerrainType(loc.x+1, loc.y) == kGround))
	{
		if ((up && (map->getTerrainType(loc.x+1, loc.y-1) == kGround)))
			actions.push_back(kNE);
		if ((down && (map->getTerrainType(loc.x+1, loc.y+1) == kGround)))
			actions.push_back(kSE);
		actions.push_back(kE);
	}
}

tDirection MapEnvironment::GetAction(xyLoc &s1, xyLoc &s2)
{
	int result = kStay;
	switch (s1.x-s2.x)
	{
		case -1: result = kE; break;
		case 0: break;
		case 1: result = kW; break;
		default: return kTeleport;
	}
	
	// Tack the vertical move onto it
	// Notice the exploit of particular encoding of kStay, kE, etc. labels
	switch (s1.y-s2.y)
	{
		case -1: result = result|kS; break;
		case 0: break;
		case 1: result = result|kN; break;
		default: return kTeleport;
	}
	return (tDirection)result;
}

bool MapEnvironment::InvertAction(tDirection &a)
{
	switch (a)
	{
		case kN: a = kS; break;
		case kNE: a = kSW; break;
		case kE: a = kW; break;
		case kSE: a = kNW; break;
		case kS: a = kN; break;
		case kSW: a = kNE; break;
		case kW: a = kE; break;
		case kNW: a = kSE; break;
		default: break;
	}
	return true;
}

void MapEnvironment::ApplyAction(xyLoc &s, tDirection dir)
{
	xyLoc old = s;
	switch (dir)
	{
		case kN: s.y-=1; break;
		case kS: s.y+=1; break;
		case kE: s.x+=1; break;
		case kW: s.x-=1; break;
		case kNW: s.y-=1; s.x-=1; break;
		case kSW: s.y+=1; s.x-=1; break;
		case kNE: s.y-=1; s.x+=1; break;
		case kSE: s.y+=1; s.x+=1; break;
		default: break;
	}
	if (map->canStep(s.x, s.y, old.x, old.y))
		return;
	s = old;
}

double MapEnvironment::HCost(xyLoc &l1, xyLoc &l2)
{
	double a = ((l1.x>l2.x)?(l1.x-l2.x):(l2.x-l1.x));
	double b = ((l1.y>l2.y)?(l1.y-l2.y):(l2.y-l1.y));
	return (a>b)?(b*ROOT_TWO+a-b):(a*ROOT_TWO+b-a);
}

double MapEnvironment::GCost(xyLoc &l1, xyLoc &l2)
{
	double h = HCost(l1, l2);
	if (fgreater(h, ROOT_TWO))
		return DBL_MAX;
	return h;
}

bool MapEnvironment::GoalTest(xyLoc &node, xyLoc &goal)
{
	return ((node.x == goal.x) && (node.y == goal.y));
}

uint64_t MapEnvironment::GetStateHash(xyLoc &node)
{
	return (node.x<<16)|node.y;
}

uint64_t MapEnvironment::GetActionHash(tDirection act)
{
	return (uint32_t) act;
}

void MapEnvironment::OpenGLDraw(int window)
{
	map->OpenGLDraw(window);
}

void MapEnvironment::OpenGLDraw(int , xyLoc &l)
{
	GLdouble xx, yy, zz, rad;
	map->getOpenGLCoord(l.x, l.y, xx, yy, zz, rad);
	glColor3f(0.5, 0.5, 0.5);
	DrawSphere(xx, yy, zz, rad);
}


/************************************************************/

AbsMapEnvironment::AbsMapEnvironment(MapAbstraction *_ma)
:MapEnvironment(_ma->GetMap())
{
	ma = _ma;
}

AbsMapEnvironment::~AbsMapEnvironment()
{
	map = 0;
	delete ma;
}
