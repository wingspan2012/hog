/*
 * $Id: sample.cpp,v 1.23 2006/11/01 23:33:56 nathanst Exp $
 *
 *  sample.cpp
 *  hog
 *
 *  Created by Nathan Sturtevant on 5/31/05.
 *  Copyright 2005 Nathan Sturtevant, University of Alberta. All rights reserved.
 *
 * This file is part of HOG.
 *
 * HOG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * HOG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with HOG; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "Common.h"
#include "Sample.h"
//#include "AStar.h"
#include "PRAStar.h"
#include "SearchUnit.h"
//#include "SharedAMapGroup.h"
#include "MapCliqueAbstraction.h"
#include "NodeLimitAbstraction.h"
#include "MapQuadTreeAbstraction.h"
//#include "RadiusAbstraction.h"
//#include "MapFlatAbstraction.h"
//#include "ClusterAbstraction.h"
#include "UnitSimulation.h"
#include "EpisodicSimulation.h"
#include "Plot2D.h"
#include "Map2DEnvironment.h"
#include "RandomUnits.h"
#include "CFOptimalRefinement.h"
#include "IRDijkstra.h"
#include "IRAStar.h"
//#include "GenericAStar.h"
//#include "FringeSearch.h"
#include "AStar.h"
#include "TemplateAStar.h"
#include "GraphEnvironment.h"

bool mouseTracking;
int px1, py1, px2, py2;
int absType = 0;

std::vector<UnitAbsMapSimulation *> unitSims;
//aStar *CFOR = 0;
//CFOptimalRefinement *CFOR = 0;
//IRDijkstra *CFOR = 0;
IRAStar *CFOR = 0;
//unit *cameraTarget = 0;

// Use batch if you want to run a series of tests
bool batch = false;
int numInstances = 1;
path * p = NULL;
int mazeSize = 100;

Plotting::Plot2D *plot = 0;
Plotting::Line *distLine = 0;

int main(int argc, char* argv[])
{
	InstallHandlers();
	RunHOGGUI(argc, argv);
	//RunHOG(argc, argv);
}


/**
 * This function is used to allocate the unit simulated that you want to run.
 * Any parameters or other experimental setup can be done at this time.
 */
void CreateSimulation(int id)
{
	Map *map;
	if (gDefaultMap[0] == 0)
	{
		map = new Map(mazeSize, mazeSize);
		MakeMaze(map, 1);
	}
	else
		map = new Map(gDefaultMap);
	map->setTileSet(kWinter);
	
	unitSims.resize(id+1);
	//unitSims[id] = new EpisodicSimulation<xyLoc, tDirection, AbsMapEnvironment>(new AbsMapEnvironment(new MapQuadTreeAbstraction(map, 2)));
	//unitSims[id] = new EpisodicSimulation<xyLoc, tDirection, AbsMapEnvironment>(new AbsMapEnvironment(new NodeLimitAbstraction(map, 8)));
	unitSims[id] = new EpisodicSimulation<xyLoc, tDirection, AbsMapEnvironment>(new AbsMapEnvironment(new MapCliqueAbstraction(map)));
	unitSims[id]->SetStepType(kMinTime);
//	unitSim = new UnitSimulation<xyLoc, tDirection, MapEnvironment>(new MapEnvironment(map),
//																																 (OccupancyInterface<xyLoc, tDirection>*)0);
//	if (absType == 0)
//		unitSim = new unitSimulation(new MapCliqueAbstraction(map));
//	else if (absType == 1)
//		unitSim = new unitSimulation(new RadiusAbstraction(map, 1));
//	else if (absType == 2)
//		unitSim = new unitSimulation(new MapQuadTreeAbstraction(map, 2));
//	else if (absType == 3)
//		unitSim = new unitSimulation(new ClusterAbstraction(map, 8));
	
	//unitSim->setCanCrossDiagonally(true);
	//unitSim = new unitSimulation(new MapFlatAbstraction(map));
}

/**
 * Allows you to install any keyboard handlers needed for program interaction.
 */
void InstallHandlers()
{
	InstallKeyboardHandler(MyDisplayHandler, "Toggle Abstraction", "Toggle display of the ith level of the abstraction", kAnyModifier, '0', '9');
	InstallKeyboardHandler(MyDisplayHandler, "Cycle Abs. Display", "Cycle which group abstraction is drawn", kAnyModifier, '\t');
	InstallKeyboardHandler(MyDisplayHandler, "Pause Simulation", "Pause simulation execution.", kNoModifier, 'p');
	InstallKeyboardHandler(MyDisplayHandler, "Step Simulation", "If the simulation is paused, step forward .1 sec.", kNoModifier, 'o');
	InstallKeyboardHandler(MyDisplayHandler, "Step History", "If the simulation is paused, step forward .1 sec in history", kAnyModifier, '}');
	InstallKeyboardHandler(MyDisplayHandler, "Step History", "If the simulation is paused, step back .1 sec in history", kAnyModifier, '{');
	InstallKeyboardHandler(MyDisplayHandler, "Step Abs Type", "Increase abstraction type", kAnyModifier, ']');
	InstallKeyboardHandler(MyDisplayHandler, "Step Abs Type", "Decrease abstraction type", kAnyModifier, '[');

	InstallKeyboardHandler(MyPathfindingKeyHandler, "Mapbuilding Unit", "Deploy unit that paths to a target, building a map as it travels", kNoModifier, 'd');
	InstallKeyboardHandler(MyRandomUnitKeyHandler, "Add A* Unit", "Deploys a simple a* unit", kNoModifier, 'a');
	InstallKeyboardHandler(MyRandomUnitKeyHandler, "Add simple Unit", "Deploys a randomly moving unit", kShiftDown, 'a');
	InstallKeyboardHandler(MyRandomUnitKeyHandler, "Add simple Unit", "Deploys a right-hand-rule unit", kControlDown, 1);

	InstallCommandLineHandler(MyCLHandler, "-map", "-map filename", "Selects the default map to be loaded.");
	InstallCommandLineHandler(MyCLHandler, "-seed", "-seed integer", "Sets the randomized number generator to use specified key.");
	InstallCommandLineHandler(MyCLHandler, "-batch", "-batch numScenarios", "Runs a bunch of test scenarios.");
	InstallCommandLineHandler(MyCLHandler, "-size", "-batch integer", "If size is set, we create a square maze with the x and y dimensions specified.");
	
	InstallWindowHandler(MyWindowHandler);

	InstallMouseClickHandler(MyClickHandler);
}

void MyWindowHandler(unsigned long windowID, tWindowEventType eType)
{
	if (eType == kWindowDestroyed)
	{
		printf("Window %ld destroyed\n", windowID);
		RemoveFrameHandler(MyFrameHandler, windowID, 0);
		delete CFOR;
		CFOR = 0;
	}
	else if (eType == kWindowCreated)
	{
		printf("Window %ld created\n", windowID);
		InstallFrameHandler(MyFrameHandler, windowID, 0);
		CreateSimulation(windowID);
	}
}

void MyFrameHandler(unsigned long windowID, unsigned int viewport, void *)
{
	if ((windowID < unitSims.size()) && (unitSims[windowID] == 0))
		return;

	if (viewport == 0)
	{
		unitSims[windowID]->StepTime(1.0/30.0);
		// If batch mode is on, automatically start the test
		if( batch && !CFOR )
			MyDisplayHandler(windowID, (tKeyboardModifier)NULL, 'o');
		if ((!unitSims[windowID]->GetPaused()) && CFOR)
			for (int x = 0; x < 100; x++)
			{
				p = CFOR->DoOneSearchStep();
				if (p)
				{
					//printf("DONE!!!\n");
					printf("%d nodes expanded, %d nodes refined, %u pathlength \n",
								 CFOR->GetNodesExpanded(), CFOR->GetNodesRefined(),
								 p->length() );
				// New Instance
				if(--numInstances)
				{
					while (!CFOR->InitializeSearch(unitSims[windowID]->GetEnvironment()->GetMapAbstraction(),
						unitSims[windowID]->GetEnvironment()->GetMapAbstraction()->GetAbstractGraph(0)->GetRandomNode(),
						unitSims[windowID]->GetEnvironment()->GetMapAbstraction()->GetAbstractGraph(0)->GetRandomNode()))
					{}
				}
				else if(batch)
					exit(0);
			}
		}

		if (CFOR)
		{
			CFOR->OpenGLDraw();
		}
	}
	unitSims[windowID]->OpenGLDraw();
	
//	if (viewport == 0)
//	{
//		if (plot)
//		{
//			if (distLine && cameraTarget)
//			{
//				MapAbstraction *ma = unitSim->GetMapAbstraction();
//				int x, y;
//				cameraTarget->getLocation(x, y);
//				node *n1 = ma->GetNodeFromMap(x, y);
//				cameraTarget->GetGoal()->getLocation(x, y);
//				node *n2 = ma->GetNodeFromMap(x, y);
//				distLine->AddPoint(ma->h(n1, n2));
//			}
//			plot->OpenGLDraw();
//			return;
//		}
//	}
//	
//	unitSim->OpenGLDraw();
//	switch (viewport%3)
//	{
//		case 0: //unitSim->GetMap()->OpenGLDraw(kLines); break;
//		case 1: //unitSim->GetMap()->OpenGLDraw(kPoints); break;
//		case 2: unitSim->GetMap()->OpenGLDraw(kPolygons); break;
//	}
//	if ((mouseTracking) && (px1 != -1) && (px2 != -1) && (py1 != -1) && (py2 != -1))
//	{
//		glColor3f(1.0, 1.0, 1.0);
//		GLdouble x1, y1, z1, rad;
//		glBegin(GL_LINES);
//		unitSim->GetMap()->getOpenGLCoord(px1, py1, x1, y1, z1, rad);
//		glVertex3f(x1, y1, z1-rad);
//		unitSim->GetMap()->getOpenGLCoord(px2, py2, x1, y1, z1, rad);
//		glVertex3f(x1, y1, z1-rad);
//		glEnd();
//	}
//	
//	if (viewport != 0)
//		return;
//	
//	char tempStr[255];
//	sprintf(tempStr, "Simulation time elapsed: %1.2f, Display Time: %1.2f",
//					unitSim->getSimulationTime(), unitSim->getDisplayTime());
//	submitTextToBuffer(tempStr);
//	
//	if (cameraTarget)
//	{
//		GLdouble x, y, z, r;
//		GLdouble xx, yy, zz;
//		cameraTarget->getOpenGLLocation(unitSim->GetMap(), x, y, z, r);
//		if (cameraTarget->GetGoal())
//			cameraTarget->GetGoal()->getOpenGLLocation(unitSim->GetMap(), xx, yy, zz, r);
//		else {
//			xx = -x; 
//			yy = -y;
//		}
//		
//		int oldPort = GetActivePort(windowID);
//		SetActivePort(windowID, 1);
//		cameraMoveTo(xx+3*(xx-x), yy+3*(yy-y), z-0.25, 0.05);
//		cameraLookAt(x, y, z, .2);
//		SetActivePort(windowID, oldPort);
//
////		SetActivePort(windowID, 0);
////		cameraMoveTo(x, y, z-250*r, 0.05);
////		cameraLookAt(x, y, z, .2);
////		SetActivePort(windowID, oldPort);
//	}
}

int MyCLHandler(char *argument[], int maxNumArgs)
{
	if( strcmp( argument[0], "-map" ) == 0 )
	{
		if (maxNumArgs <= 1)
			return 0;
		strncpy(gDefaultMap, argument[1], 1024);
		return 2;
	}
	else if( strcmp( argument[0], "-seed" ) == 0 )
	{
		if (maxNumArgs <= 1)
			return 0;
		srand(atoi(argument[1]));
		return 2;
	}
	else if( strcmp( argument[0], "-batch" ) == 0 )
	{
		if (maxNumArgs <= 1)
			return 0;
		numInstances = atoi(argument[1]);
		batch = true;
		assert( numInstances > 0 );
		printf("numInstances = %d\n", numInstances);
		return 2;
	}
	else if( strcmp( argument[0], "-size" ) == 0 )
	{
		if (maxNumArgs <= 1)
			return 0;
		mazeSize = atoi(argument[1]);
		assert( mazeSize > 0 );
		printf("mazeSize = %d\n", numInstances);
		return 2;
	}
	return 2; //ignore typos
}

void MyDisplayHandler(unsigned long windowID, tKeyboardModifier mod, char key)
{
	switch (key)
	{
		case '\t':
			if (mod != kShiftDown)
				SetActivePort(windowID, (GetActivePort(windowID)+1)%GetNumPorts(windowID));
			else
			{
				SetNumPorts(windowID, 1+(GetNumPorts(windowID)%MAXPORTS));
			}
			break;
		case 'p': unitSims[windowID]->SetPaused(!unitSims[windowID]->GetPaused()); break;
		case 'o':
		{
			if (CFOR == 0)
			{
				//CFOR = new aStar();
				//CFOR = new CFOptimalRefinement();
				//CFOR = new IRDijkstra();
				CFOR = new IRAStar( IRAStarConstants::P_G_CACHING );
				printf("%d nodes in problem \n", unitSims[windowID]->GetEnvironment()->GetMapAbstraction()->GetAbstractGraph(0)->GetNumNodes());
				while (!CFOR->InitializeSearch(unitSims[windowID]->GetEnvironment()->GetMapAbstraction(),
					unitSims[windowID]->GetEnvironment()->GetMapAbstraction()->GetAbstractGraph(0)->GetRandomNode(),
					unitSims[windowID]->GetEnvironment()->GetMapAbstraction()->GetAbstractGraph(0)->GetRandomNode()))
				{}
			}
			p = CFOR->DoOneSearchStep();
			if (p)
			{
				//printf("DONE!!!\n");
				printf("%d nodes expanded, %d nodes refined, %u pathlength \n",
							 CFOR->GetNodesExpanded(), CFOR->GetNodesRefined(),
							 p->length() );
				// New Instance
				if(--numInstances)
				{
					// New Instance
					while (!CFOR->InitializeSearch(unitSims[windowID]->GetEnvironment()->GetMapAbstraction(),
						unitSims[windowID]->GetEnvironment()->GetMapAbstraction()->GetAbstractGraph(0)->GetRandomNode(),
						unitSims[windowID]->GetEnvironment()->GetMapAbstraction()->GetAbstractGraph(0)->GetRandomNode()))
					{}
				}
				else
					exit(0);
			}
		}
			if (unitSims[windowID]->GetPaused())
			{
				unitSims[windowID]->SetPaused(false);
				unitSims[windowID]->StepTime(1.0/30.0);
				unitSims[windowID]->SetPaused(true);
			}
			break;
		case ']': absType = (absType+1)%3; break;
		case '[': absType = (absType+4)%3; break;
//		case '{': unitSim->setPaused(true); unitSim->offsetDisplayTime(-0.5); break;
//		case '}': unitSim->offsetDisplayTime(0.5); break;
		default:
			if (unitSims[windowID])
			{
				printf("Abs level %d has %d nodes\n", key-'0',
					   unitSims[windowID]->GetEnvironment()->GetMapAbstraction()->GetAbstractGraph((key-'0'))->GetNumNodes());
				unitSims[windowID]->GetEnvironment()->GetMapAbstraction()->ToggleDrawAbstraction(((mod == kControlDown)?10:0)+(key-'0'));
			}
			break;
	}
}

void MyRandomUnitKeyHandler(unsigned long windowID, tKeyboardModifier , char)
{
	Map *m = unitSims[windowID]->GetEnvironment()->GetMap();
	
	int x1, y1, x2, y2;
	x2 = random()%m->getMapWidth();
	y2 = random()%m->getMapHeight();
	x1 = random()%m->getMapWidth();
	y1 = random()%m->getMapHeight();
	SearchUnit *su1 = new SearchUnit(x1, y1, 0, 0);
	//SearchUnit *su2 = new SearchUnit(random()%m->getMapWidth(), random()%m->getMapHeight(), su1, new praStar());
	SearchUnit *su2 = new SearchUnit(x2, y2, su1, new CFOptimalRefinement());
	//unitSim->AddUnit(su1);
	unitSims[windowID]->AddUnit(su2);
	
//	RandomerUnit *r = new RandomerUnit(random()%m->getMapWidth(), random()%m->getMapHeight());
//	int id = unitSim->AddUnit(r);
//	xyLoc loc;
//	r->GetLocation(loc);
//	printf("Added unit %d at (%d, %d)\n", id, loc.x, loc.y);

//	int x1, y1, x2, y2;
//	unit *u;
//	unitSim->getRandomLocation(x1, y1);
//	unitSim->getRandomLocation(x2, y2);
//	switch (mod)
//	{
//		case kControlDown: unitSim->addUnit(u=new rhrUnit(x1, y1)); break;
//		case kShiftDown: unitSim->addUnit(u=new randomUnit(x1, y1)); break;
//		default:
//			unit *targ;
//	unitSim->addUnit(targ = new unit(x2, y2));
//	unitSim->addUnit(u=new SearchUnit(x1, y1, targ, new praStar())); break;
//	}
//	delete plot;
//	plot = new Plotting::Plot2D();
//	delete distLine;
//	plot->AddLine(distLine = new Plotting::Line("distline"));
//	cameraTarget = u;
//	u->setSpeed(1.0/4.0);
}

void MyPathfindingKeyHandler(unsigned long , tKeyboardModifier , char)
{
	AbsMapEnvironment *env = unitSims[0]->GetEnvironment();
	MapAbstraction *mabs = env->GetMapAbstraction();
	Map *m = env->GetMap();
	MapEnvironment ma(m->clone());
	std::vector<xyLoc> path;
	TemplateAStar<xyLoc, tDirection, MapEnvironment> a;
	for (int x = 0; x < 10000; x++)
	{
		node *n1, *n2;
		do {
			Graph *g = mabs->GetAbstractGraph(0);
			n1 = g->GetRandomNode();
			n2 = g->GetRandomNode();
		} while (!mabs->Pathable(n1, n2));
		mabs->GetTileFromNode(n1, px1, py1);
		mabs->GetTileFromNode(n2, px2, py2);
		xyLoc s1, g1;
		s1.x = px1; s1.y = py1;
		g1.x = px2; g1.y = py2;
		a.GetPath(&ma, s1, g1, path);
		printf("%d\t%d\t%d\t%d\t%1.5f\n", px1, py1, px2, py2, ma.GetPathLength(path));
	}
	
	//	for (int x = 0; x < ((mod==kShiftDown)?(50):(1)); x++)
//	{
//		if (unitSim->getUnitGroup(1) == 0)
//		{
//			unitSim->addUnitGroup(new SharedAMapGroup(unitSim));
//			unitSim->setmapAbstractionDisplay(2);
//		}
//		int xx1, yy1, xx2, yy2;
//		unitSim->getRandomLocation(xx1, yy1);
//		unitSim->getRandomLocation(xx2, yy2);
//		
//		unit *u, *u2 = new unit(xx2, yy2, 0);
//		
//		praStar *pra = new praStar(); pra->setPartialPathLimit(4);
//		//aStar *pra = new aStar();
//		
//		unitSim->addUnit(u2);
//		u = new SearchUnit(xx1, yy1, u2, pra);
//		// just set the group of the unit, and it will share a map with those
//		// units.
//		unitSim->getUnitGroup(1)->addUnit(u);
//		unitSim->addUnit(u);
//		u->setSpeed(0.5); // time to go 1 distance						
//	}
}

bool MyClickHandler(unsigned long windowID, int, int, point3d loc, tButtonType button, tMouseEventType mType)
{
	if (mType != kMouseUp)
		return false;
	
	AbsMapEnvironment *env = unitSims[0]->GetEnvironment();
	Map *m = env->GetMap();
	env->GetMap()->getPointFromCoordinate(loc, px1, py1);
	printf("Mouse up at (%d, %d)\n", px1, py1);

	MapEnvironment ma(m->clone());
	std::vector<xyLoc> path;
	TemplateAStar<xyLoc, tDirection, MapEnvironment> a;
	a.SetStopAfterGoal(false);
	xyLoc s1;
	xyLoc g1;
	s1.x = px1; s1.y = py1;
	g1.x = px1; g1.y = py1;
	a.GetPath(&ma, s1, g1, path);
	printf("Solution: moves %d, length %f\n", path.size(), ma.GetPathLength(path));
//	for (unsigned int x = 0; x < path.size(); x++)
//		printf("%lu   ", path[x]);
//	printf("\n");
	
	printf("%d nodes expanded\n", a.GetNodesExpanded());
	
	GraphEnvironment ge(env->GetMapAbstraction()->GetAbstractGraph(1), 0);
	node *n = env->GetMapAbstraction()->GetNodeFromMap(px1, py1);
	n = env->GetMapAbstraction()->GetParent(n);
	TemplateAStar<graphState, graphMove, GraphEnvironment> a_g;
	std::vector<graphState> p1;
	graphState s2 = n->GetNum(), g2 = n->GetNum();
	a_g.SetStopAfterGoal(false);
	a_g.GetPath(&ge, s2, g2, p1);
	printf("Solution: moves %d, length %f\n", p1.size(), ge.GetPathLength(p1));
	printf("%d nodes expanded\n", a_g.GetNodesExpanded());
	
	return false;
//	mouseTracking = false;
//	if (button == kRightButton)
//	{
//		switch (mType)
//		{
//			case kMouseDown:
//				unitSim->GetMap()->getPointFromCoordinate(loc, px1, py1);
//				//printf("Mouse down at (%d, %d)\n", px1, py1);
//				break;
//			case kMouseDrag:
//				mouseTracking = true;
//				unitSim->GetMap()->getPointFromCoordinate(loc, px2, py2);
//				//printf("Mouse tracking at (%d, %d)\n", px2, py2);
//				break;
//			case kMouseUp:
//			{
//				if ((px1 == -1) || (px2 == -1))
//					break;
//				unitSim->GetMap()->getPointFromCoordinate(loc, px2, py2);
//				//printf("Mouse up at (%d, %d)\n", px2, py2);
//				unit *u, *u2 = new unit(px2, py2, 0);
//				//praStar *pra = new praStar(); pra->setPartialPathLimit(4);
//				aStar *pra = new aStar();
//				unitSim->addUnit(u2);
//				u = new SearchUnit(px1, py1, u2, pra);
//				unitSim->addUnit(u);
//				u->setSpeed(0.5); // time to go 1 distance						
//			}
//			break;
//		}
//		return true;
//	}
//	return false;
}
