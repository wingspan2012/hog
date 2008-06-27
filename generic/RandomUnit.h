/*
 *  RandomUnit.h
 *  hog2
 *
 *  Created by Nathan Sturtevant on 5/14/07.
 *  Copyright 2007 Nathan Sturtevant, University of Alberta. All rights reserved.
 *
 */

#include "Unit.h"

#ifndef RANDOMUNIT_H
#define RANDOMUNIT_H

template <class state, class action, class environment>
class RandomUnit : public Unit<state, action, environment>
{
public:
	RandomUnit(state startLoc) :loc(startLoc) {}
	virtual ~RandomUnit() {}
	virtual const char *GetName() { return "Random Unit"; }
	virtual bool MakeMove(environment *env, OccupancyInterface<state, action> *, SimulationInfo<state,action,environment> *, action& a)
	{
		std::vector<action> acts;
		env->GetActions(loc, acts);
		a = acts[random()%acts.size()];
		return true;
	}
	virtual void UpdateLocation(environment *, state &newloc, bool success, SimulationInfo<state,action,environment> *)
	{
		if (success)
			loc = newloc;
	}
	virtual void GetLocation(state &l)
	{ l = loc; }
	virtual void OpenGLDraw(int window, environment *env, SimulationInfo<state,action,environment> *)
	{ env->OpenGLDraw(window, loc); }
	virtual void GetGoal(state &s)
	{ s = loc; }
private:
	state loc;
};

#endif

