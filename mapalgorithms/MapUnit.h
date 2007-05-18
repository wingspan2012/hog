/*
 *  MapUnit.h
 *  hog2
 *
 *  Created by Nathan Sturtevant on 4/23/07.
 *  Copyright 2007 Nathan Sturtevant, University of Alberta. All rights reserved.
 *
 */

#include "Unit.h"
#include "Map2DEnvironment.h"

#ifndef MAPUNIT_H
#define MAPUNIT_H

/**
* A simple map-based unit
 */

class MapUnit : public Unit<xyLoc, tDirection, MapEnvironment> {
public:
	MapUnit(int x, int y)
	:loc(x, y) { r = 1.0; g = 0; b = 0;}

	virtual void UpdateLocation(MapEnvironment *, xyLoc &l, bool, SimulationInfo *)
	{ loc = l; }
	virtual void GetLocation(xyLoc &l)
	{ l = loc; }
	virtual void OpenGLDraw(int window, MapEnvironment *, SimulationInfo *);
protected:
	GLfloat r, g, b;
	xyLoc loc;
};

#endif
