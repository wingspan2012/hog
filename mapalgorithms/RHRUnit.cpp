/*
 *  RHRUnit.cpp
 *  hog2
 *
 *  Created by Nathan Sturtevant on 4/23/07.
 *  Copyright 2007 Nathan Sturtevant, University of Alberta. All rights reserved.
 *
 */

#include "RHRUnit.h"

/**
* Make the next move.
 *
 * Make the next move following the right hand rule.
 */
tDirection RHRUnit::MakeMove(MapEnvironment *, SimulationInfo *)
{
	tDirection where = possibleDir[lastIndex];
	return where;
}

/**
* set rhr unit location.
 *
 * To follow the right hand rule, we do the following:
 * 1) if the last move was successful, turn 90 degrees to the right and keep going.
 * 2) if the last move wasn't successful, turn 90 degrees to the left and keep going.
 */
void RHRUnit::UpdateLocation(MapEnvironment *, xyLoc l, bool success, SimulationInfo *)
{
	if (success)
	{ // I moved successfully
		lastIndex = (lastIndex+2)%8;
	}
	else {
		lastIndex = (lastIndex+6)%8;
	}
	loc = l;
}
