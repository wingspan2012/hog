/*
 *  ConfigEnvironment.cpp
 *  hog2
 *
 *  Created by Nathan Sturtevant on 1/9/09.
 *  Copyright 2009 NS Software. All rights reserved.
 *
 */

#include "ConfigEnvironment.h"
#include "FPUtil.h"
#include <iostream>
#include <math.h>

ConfigEnvironment::ConfigEnvironment()
{
}

ConfigEnvironment::~ConfigEnvironment()
{
}

void ConfigEnvironment::GetSuccessors(recVec &nodeID, std::vector<recVec> &neighbors)
{
	//[(-0.300000, -0.320000) to (0.000000, 0.000000)] does not cross [(-0.320000, -0.300000) to (0.320000, -0.300000)]
//	line2d a(recVec(-0.300000, -0.320000, 0), recVec(0, -0.4, 0));
//	line2d b(recVec(-0.320000, -0.300000, 0), recVec(0.320000, -0.300000, 0));
//	line2d c(recVec(-0.300000, -0.320000, 0), recVec(-0.300000, 0.320000, 0));
//	if (a.crosses(b))
//		printf("1 They Cross!\n");
//	else
//		printf("1 No crossing here!\n");
//	if (b.crosses(a))
//		printf("2 They Cross!\n");
//	else
//		printf("2 No crossing here!\n");
//	if (a.crosses(c))
//		printf("3 They Cross!\n");
//	else
//		printf("3 No crossing here!\n");
//	if (c.crosses(a))
//		printf("4 They Cross!\n");
//	else
//		printf("4 No crossing here!\n");
	
	neighbors.resize(0);
	neighbors.push_back(goal);
	for (unsigned int x = 0; x < obstacles.size(); x++)
	{
		neighbors.push_back(obstacles[x].start);
		neighbors.push_back(obstacles[x].end);
	}
	for (unsigned int x = 0; x < neighbors.size(); x++)
	{
		if (!Legal(nodeID, neighbors[x]))
		{
			neighbors[x] = neighbors.back();
			neighbors.pop_back();
			x--;
		}
	}
//	printf("Returning:\n");
//	for (unsigned int x = 0; x < neighbors.size(); x++)
//		std::cout << neighbors[x] << " ";
//	std::cout << std::endl;
}

void ConfigEnvironment::GetActions(recVec &nodeID, std::vector<line2d> &actions)
{
	actions.resize(0);
	actions.push_back(line2d(nodeID, goal));
	for (unsigned int x = 0; x < obstacles.size(); x++)
	{
		actions.push_back(line2d(nodeID, obstacles[x].start));
		actions.push_back(line2d(nodeID, obstacles[x].end));
	}
	for (unsigned int x = 0; x < actions.size(); x++)
	{
		if (!Legal(actions[x].start, actions[x].end))
		{
			actions[x] = actions.back();
			actions.pop_back();
			x--;
		}
	}
}

bool ConfigEnvironment::Legal(recVec &a, recVec &b)
{
	line2d l(a, b);
	for (unsigned int x = 0; x < obstacles.size(); x++)
	{
		if (obstacles[x].crosses(l))
		{
//			printf("[(%f, %f) to (%f, %f)] CROSSES [(%f, %f) to (%f, %f)]\n",
//				   a.x, a.y, b.x, b.y,
//				   obstacles[x].start.x, obstacles[x].start.y,
//				   obstacles[x].end.x, obstacles[x].end.y);
			return false;
		}
//		printf("[(%f, %f) to (%f, %f)] does not cross [(%f, %f) to (%f, %f)]\n",
//			   a.x, a.y, b.x, b.y,
//			   obstacles[x].start.x, obstacles[x].start.y,
//			   obstacles[x].end.x, obstacles[x].end.y);
	}
	return true;
}

line2d ConfigEnvironment::GetAction(recVec &s1, recVec &s2)
{
	line2d d;
	d.start = s1;
	d.end = s2;
	return d;
}

void ConfigEnvironment::ApplyAction(recVec &s, line2d dir)
{
	s = dir.end;
}


bool ConfigEnvironment::InvertAction(line2d &a)
{
	line2d b = a;
	a.start = b.end;
	a.end = b.start;
	return true;
}


double ConfigEnvironment::HCost(recVec &node1, recVec &node2)
{
	return sqrt((node1.x-node2.x)*(node1.x-node2.x) + 
				(node1.y-node2.y)*(node1.y-node2.y) + 
				(node1.z-node2.z)*(node1.z-node2.z));
}

double ConfigEnvironment::GCost(recVec &node1, recVec &node2)
{
	return sqrt((node1.x-node2.x)*(node1.x-node2.x) + 
				(node1.y-node2.y)*(node1.y-node2.y) + 
				(node1.z-node2.z)*(node1.z-node2.z));
}

double ConfigEnvironment::GCost(recVec &node1, line2d &act)
{
	return sqrt((node1.x-act.end.x)*(node1.x-act.end.x) + 
				(node1.y-act.end.y)*(node1.y-act.end.y) + 
				(node1.z-act.end.z)*(node1.z-act.end.z));
}

bool ConfigEnvironment::GoalTest(recVec &node, recVec &goal)
{
	return (fequal(node.x, goal.x) && fequal(node.y, goal.y) && fequal(node.z, goal.z));
}

uint64_t ConfigEnvironment::GetStateHash(recVec &node)
{
	int x, y, z;
	bool nx = node.x<0, ny = node.y<0;
	x = (int)((double)fabs(node.x)*10000.0);
	y = (int)((double)fabs(node.y)*10000.0);
	uint64_t val = 0;
	val = x;
	val = (val<<14)|y;
	val = (val<<1)|(nx?1:0);
	val = (val<<1)|(ny?1:0);
	return val;
}

uint64_t ConfigEnvironment::GetActionHash(line2d act)
{
	return (GetStateHash(act.start)^GetStateHash(act.end));
}


void ConfigEnvironment::OpenGLDraw(int )
{
	glColor3f(0, 0, 1.0);
	for (unsigned int x = 0; x < obstacles.size(); x++)
		DrawLine(obstacles[x]);
}

void ConfigEnvironment::OpenGLDraw(int , recVec &l)
{
	glBegin(GL_POINT);
	glVertex3f(l.x, l.y, l.z);
	glEnd();
}

void ConfigEnvironment::OpenGLDraw(int, recVec &, line2d &)
{
}

void ConfigEnvironment::OpenGLDraw(int, recVec &, line2d &, GLfloat r, GLfloat g, GLfloat b)
{
}

void ConfigEnvironment::OpenGLDraw(int, recVec &l, GLfloat r, GLfloat g, GLfloat b)
{
}


void ConfigEnvironment::GetNextState(recVec &currents, line2d dir, recVec &news)
{
	news = dir.end;
}

void ConfigEnvironment::DrawLine(line2d l)
{
	glLineWidth(3);
	glBegin(GL_LINES);
	glVertex3f(l.start.x, l.start.y, 0);
	glVertex3f(l.end.x, l.end.y, 0);
	glEnd();
	glLineWidth(1);
}
