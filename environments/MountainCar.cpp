/*
 *  MountainCar.cpp
 *  hog2
 *
 *  Created by Nathan Sturtevant on 11/29/10.
 *  Copyright 2010 University of Denver. All rights reserved.
 *
 */

#include "MountainCar.h"

MountainCarEnvironment::MountainCarEnvironment()
{
    minPosition = -1.2;
    maxPosition = 0.6;
    minVelocity = -0.07;
    maxVelocity = 0.07;
    goalPosition = 0.5;
    accelerationFactor = 0.001;
    gravityFactor = -0.0025;
    hillPeakFrequency = 3.0;
}

void MountainCarEnvironment::GetSuccessors(const MountainCarState &nodeID, std::vector<MountainCarState> &neighbors) const
{
	neighbors.resize(0);
	neighbors.push_back(nodeID);
	neighbors.push_back(nodeID);
	neighbors.push_back(nodeID);
	ApplyAction(neighbors[0], 1);
	ApplyAction(neighbors[1], 0);
	ApplyAction(neighbors[2], -1);
}

void MountainCarEnvironment::GetActions(const MountainCarState &nodeID, std::vector<MountainCarAction> &actions) const
{
	actions.resize(0);
	actions.push_back(1);
	actions.push_back(0);
	actions.push_back(-1);
}

MountainCarAction MountainCarEnvironment::GetAction(const MountainCarState &s1, const MountainCarState &s2) const
{
	assert(!"GetAction not implemented");
}

void MountainCarEnvironment::ApplyAction(MountainCarState &s, MountainCarAction a) const
{
	s.speed += a * accelerationFactor + GetSlope(s.loc) * (gravityFactor);
	if (s.speed > maxVelocity) {
		s.speed = maxVelocity;
	}
	if (s.speed < minVelocity)
	{
		s.speed = minVelocity;
	}
	s.loc += s.speed;
	if (s.loc > maxPosition)
	{
		s.loc = maxPosition;
	}
	if (s.loc < minPosition)
	{
		s.loc = minPosition;
	}
	if (s.loc == minPosition && s.speed < 0)
	{
		s.speed = 0;
	}
}

double MountainCarEnvironment::GetHeightAtPosition(double queryPosition) const
{
	return -sin(hillPeakFrequency * (queryPosition));
}

double MountainCarEnvironment::GetSlope(double queryPosition) const
{
	/*The curve is generated by cos(hillPeakFrequency(x-pi/2)) so the 
	 * pseudo-derivative is cos(hillPeakFrequency* x) 
	 */
	return cos(hillPeakFrequency * queryPosition);
}


void MountainCarEnvironment::GetNextState(MountainCarState &s1, MountainCarAction a, MountainCarState &s2) const
{
	s2 = s1;
	ApplyAction(s2, a);
}

bool MountainCarEnvironment::InvertAction(MountainCarAction &a) const
{
	a = -a;
	return false; // actions aren't really invertable
}

/** Heuristic value between two arbitrary nodes. **/
double MountainCarEnvironment::HCost(const MountainCarState &node1, const MountainCarState &node2)
{ return 0; }

double MountainCarEnvironment::GCost(const MountainCarState &node1, const MountainCarState &node2)
{
	return 1;
}

double MountainCarEnvironment::GCost(const MountainCarState &node, const MountainCarAction &act)
{
	return 1;
}

bool MountainCarEnvironment::GoalTest(const MountainCarState &node, const MountainCarState &goal)
{
	return fgreater(node.loc, goalPosition);
}

uint64_t MountainCarEnvironment::GetStateHash(const MountainCarState &node) const
{
	return (((int)(node.loc*1000))<<10) + node.speed*1000;
}

uint64_t MountainCarEnvironment::GetActionHash(MountainCarAction act) const
{
	return act+1;
}

void MountainCarEnvironment::OpenGLDraw() const
{
//	minPosition = -1.2;
//    maxPosition = 0.6;
	glBegin(GL_LINE_STRIP);
	glColor3f(0, 1.0, 0);
	for (double x = minPosition; x < maxPosition; x += 0.1)
	{
		glVertex3f(x, GetHeightAtPosition(x), 0);
	}	
	glEnd();
}

void MountainCarEnvironment::OpenGLDraw(const MountainCarState&) const { }
void MountainCarEnvironment::OpenGLDraw(const MountainCarState&, const MountainCarState&, float) const { }
void MountainCarEnvironment::OpenGLDraw(const MountainCarState&, const MountainCarAction&) const { }

