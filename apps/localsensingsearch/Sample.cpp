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
#include "UnitSimulation.h"
#include "EpisodicSimulation.h"
#include "Map2DEnvironment.h"
#include "RandomUnits.h"
#include "AStar.h"
#include "TemplateAStar.h"
#include "GraphEnvironment.h"
#include "LocalSensingUnit.h"
#include "RIBS.h"
#include "LRTAStarUnit.h"
#include "LSSLRTAStarUnit.h"
#include "ScenarioLoader.h"
#include "StatUtil.h"
#include "HeuristicLearningMeasure.h"
#include "FLRTAStarUnit.h"

bool mouseTracking = false;
bool runningSearch1 = false;
bool runningSearch2 = false;
int px1, py1, px2, py2;
int absType = 0;
int mazeSize = 21;

std::vector<EpisodicSimulation<xyLoc, tDirection, MapEnvironment> *> unitSims;
TemplateAStar<xyLoc, tDirection, MapEnvironment> a1;
TemplateAStar<xyLoc, tDirection, MapEnvironment> a2;
MapEnvironment *ma1 = 0;
MapEnvironment *ma2 = 0;
GraphDistanceHeuristic *gdh = 0;
std::vector<xyLoc> path;
double stepsPerFrame = 1.0/30.0;

HeuristicLearningMeasure<xyLoc, tDirection, MapEnvironment> measure;

void RunScenario(char *name, int which);
void RunScalingTest(int size, int which);
void RunWorkMeasureTest();
void RunSTPTest(int which);

void RunBigScenario(char *name, int which);

void CreateSimulation(int id);

int main(int argc, char* argv[])
{
	InstallHandlers();
	RunHOGGUI(argc, argv);
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
		map->SetTerrainType(1, mazeSize-2,
							mazeSize-2, mazeSize-2, kOutOfBounds);
		map->SetTerrainType(mazeSize-2, 1,
							mazeSize-2, mazeSize-2, kOutOfBounds);

//		map->SetRectHeight(0, 0, mazeSize-1, mazeSize-1, 0, kOutOfBounds);
//		for (int y = 0; y < mazeSize; y+=2)
//		{
//			map->SetTerrainType(0, y,
//								mazeSize-1, y, kGround);
//			if (0 == (y/2)%2)
//			{
//				map->SetTerrainType(mazeSize-1, y,
//									mazeSize-1, y+2, kGround);
//			}
//			else {
//				map->SetTerrainType(0, y,
//									0, y+2, kGround);
//			}
//		}
		
//		map = new Map(mazeSize, mazeSize*2);
//		map->SetRectHeight(0, 0, mazeSize-1, 2*mazeSize-1, 0, kOutOfBounds);
//		for (int x = 0; x < mazeSize; x++)
//		{
//			map->SetTerrainType(x, x,
//								x, x+3, kGround);
//			if (1 == x%2)
//				map->SetTerrainType(x, x,
//									x, x+20, kGround);
//			map->SetTerrainType(x, x+20,
//								x, x+21, kGround);
//		}
//		map->SetRectHeight(mazeSize-5*sqrt(mazeSize), 20+mazeSize, mazeSize-1, 20+mazeSize+5*sqrt(mazeSize), 0, kGround);
//		map->SetTerrainType(0, 20+mazeSize,
//							mazeSize-1, 20+mazeSize, kGround);
//		map->SetTerrainType(0, 20+mazeSize,
//							0, 2*mazeSize-1, kGround);
		
		
		//MakeMaze(map, 1);
//		BuildRandomRoomMap(map, 16);
//		map->Scale(mazeSize, mazeSize);
	}
	else
		map = new Map(gDefaultMap);
	map->SetTileSet(kWinter);
	MapEnvironment *me;
	unitSims.resize(id+1);
	unitSims[id] = new EpisodicSimulation<xyLoc, tDirection, MapEnvironment>(me = new MapEnvironment(map, false));
	me->SetDiagonalCost(1.5);
	unitSims[id]->SetStepType(kRealTime);//kUniTime
	unitSims[id]->SetThinkingPenalty(0);
	unitSims[id]->GetStats()->EnablePrintOutput(true);
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
	InstallKeyboardHandler(MyDisplayHandler, "Sum Distance", "Print total distance travelled by each unit", kNoModifier, 'd');
	InstallKeyboardHandler(MyDisplayHandler, "Step History", "If the simulation is paused, step forward .1 sec in history", kAnyModifier, '}');
	InstallKeyboardHandler(MyDisplayHandler, "Step History", "If the simulation is paused, step back .1 sec in history", kAnyModifier, '{');
	InstallKeyboardHandler(MyDisplayHandler, "Increase Sim Speed", "Increase Sim Speed", kAnyModifier, ']');
	InstallKeyboardHandler(MyDisplayHandler, "Decrease Sim Speed", "Decrease Sim Speed", kAnyModifier, '[');
	InstallKeyboardHandler(MyDisplayHandler, "Clear units", "Clear all units from sim", kAnyModifier, '|');

	InstallKeyboardHandler(MyRandomUnitKeyHandler, "Add A* Unit", "Deploys a simple a* unit", kNoModifier, 'a');
	InstallKeyboardHandler(MyRandomUnitKeyHandler, "Add simple Unit", "Deploys a randomly moving unit", kShiftDown, 'a');
	InstallKeyboardHandler(MyRandomUnitKeyHandler, "Add simple Unit", "Deploys a right-hand-rule unit", kControlDown, '1');

	InstallCommandLineHandler(MyCLHandler, "-map", "-map filename", "Selects the default map to be loaded.");
	InstallCommandLineHandler(MyCLHandler, "-size", "-size <integer>", "If size is set, we create a square maze with the x and y dimensions specified.");
	InstallCommandLineHandler(MyCLHandler, "-scenario", "-scenario <file>", "Load and run a scenario offline.");
	InstallCommandLineHandler(MyCLHandler, "-scaleTest", "-scaleTest <size>", "Run a scaling test with local minima <size>.");
	InstallCommandLineHandler(MyCLHandler, "-STPTest", "-STPTest", "Run a STP test.");
	
	InstallWindowHandler(MyWindowHandler);

	InstallMouseClickHandler(MyClickHandler);
}

void MyWindowHandler(unsigned long windowID, tWindowEventType eType)
{
	if (eType == kWindowDestroyed)
	{
		printf("Window %ld destroyed\n", windowID);
		RemoveFrameHandler(MyFrameHandler, windowID, 0);

		delete ma1;
		ma1 = 0;
		delete ma2;
		ma2 = 0;
		delete gdh;
		gdh = 0;
		delete unitSims[windowID];
		unitSims[windowID] = 0;
		runningSearch1 = false;
		runningSearch2 = false;
		mouseTracking = false;
	}
	else if (eType == kWindowCreated)
	{
		printf("Window %ld created\n", windowID);
		InstallFrameHandler(MyFrameHandler, windowID, 0);
		CreateSimulation(windowID);
		SetNumPorts(windowID, 1);
	}
}

//void startRecording();
//void stopRecording();
static bool recording = false;

void MyFrameHandler(unsigned long windowID, unsigned int viewport, void *)
{
	if (viewport == 0)
	{
		unitSims[windowID]->StepTime(stepsPerFrame);
	}
	
	if (unitSims[windowID]->GetNumUnits() > 0)
	{
		if (!unitSims[windowID]->GetPaused())
			stepsPerFrame*=1.002;//1.01;//
		if (unitSims[windowID]->Done() && recording)
		{
			recording = false;
//			stopRecording();
		}

	}

	if (GetNumPorts(windowID) == unitSims[windowID]->GetNumUnits())
	{
		unitSims[windowID]->GetEnvironment()->OpenGLDraw();
		unitSims[windowID]->OpenGLDraw(viewport);
		//measure.OpenGLDraw(unitSims[windowID]->GetEnvironment());
	}
	else {
		unitSims[windowID]->GetEnvironment()->OpenGLDraw();
		unitSims[windowID]->OpenGLDraw();
	}
	
	if (mouseTracking)
	{
		glBegin(GL_LINES);
		glColor3f(1.0f, 0.0f, 0.0f);
		Map *m = unitSims[windowID]->GetEnvironment()->GetMap();
		GLdouble x, y, z, r;
		m->GetOpenGLCoord(px1, py1, x, y, z, r);
		glVertex3f(x, y, z-3*r);
		m->GetOpenGLCoord(px2, py2, x, y, z, r);
		glVertex3f(x, y, z-3*r);
		glEnd();
	}
}

int MyCLHandler(char *argument[], int maxNumArgs)
{
	if (strcmp( argument[0], "-map" ) == 0 )
	{
		if (maxNumArgs <= 1)
			return 0;
		strncpy(gDefaultMap, argument[1], 1024);
		return 2;
	}
	else if (strcmp( argument[0], "-size" ) == 0 )
	{
		if (maxNumArgs <= 1)
			return 0;
		mazeSize = atoi(argument[1]);
		assert( mazeSize > 0 );
		return 2;
	}
	else if (strcmp(argument[0], "-scenario") == 0)
	{
		RunScenario(argument[1], atoi(argument[2]));
		//RunBigScenario(argument[1], atoi(argument[2]));
	}
	else if (strcmp(argument[0], "-scaleTest") == 0)
	{
		RunScalingTest(atoi(argument[1]), atoi(argument[2]));
	}
	else if (strcmp(argument[0], "-STPTest") == 0)
	{
		RunSTPTest(atoi(argument[1]));
	}
	return 2; //ignore typos
}

void MyDisplayHandler(unsigned long windowID, tKeyboardModifier mod, char key)
{
	switch (key)
	{
		case '[': stepsPerFrame/=2.0; break; 
		case ']': stepsPerFrame*=2.0; break; 
		case '|':
			unitSims[windowID]->ClearAllUnits();
			break;
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
			if (unitSims[windowID]->GetPaused())
			{
				unitSims[windowID]->SetPaused(false);
				unitSims[windowID]->StepTime(.02);
				unitSims[windowID]->SetPaused(true);
			}
			else
				unitSims[windowID]->StepTime(1.0/30.0);
		}
			break;
		case 'd':
		{
			printf("Total travel:\n");
			for (unsigned int x = 0; x < unitSims[windowID]->GetNumUnits(); x++)
			{
				printf("%s\t%f\n", unitSims[windowID]->GetUnit(x)->GetName(),
					   SumStatEntries(unitSims[windowID]->GetStats(),
									  "trialDistanceMoved",
									  unitSims[windowID]->GetUnit(x)->GetName()));
			}

		}
		default:
			break;
	}
}

void MyRandomUnitKeyHandler(unsigned long windowID, tKeyboardModifier mod, char)
{
//	if (mod == kShiftDown)
//	{
//		RunWorkMeasureTest();
//		return;
//	}
	Map *m = unitSims[windowID]->GetEnvironment()->GetMap();
	unitSims[windowID]->ClearAllUnits();
	m->SetTileSet(kFast);
//	recording = true;
//	startRecording();

	int x1, y1, x2, y2;
	if (mod == kShiftDown)
	{
		x1 = 0; y1 = 0;
		x2 = m->GetMapWidth()-1;
//		x2 = 0;//m->GetMapWidth()-1;
		y2 = m->GetMapHeight()-1;
	}
	else {
		do {
			x2 = random()%m->GetMapWidth();
			y2 = random()%m->GetMapHeight();
			x1 = random()%m->GetMapWidth();
			y1 = random()%m->GetMapHeight();
		} while ((m->GetTerrainType(x1, y1) != kGround) || (m->GetTerrainType(x2, y2) != kGround));

		do {
			x2 = random()%m->GetMapWidth();
			y2 = random()%m->GetMapHeight();
			x1 = random()%m->GetMapWidth();
			y1 = random()%m->GetMapHeight();
		} while ((m->GetTerrainType(x1, y1) != kGround) || (m->GetTerrainType(x2, y2) != kGround));
	}
	
	xyLoc a(x1, y1), b(x2, y2);
	//xyLoc a(0, 0), b(mazeSize-1, mazeSize-1);
	//	xyLoc a(0, 0), b(mazeSize-1, 0);
	GLdouble a1, b1, c1, r1;
	m->GetOpenGLCoord((x1+x2)/2, (y1+y2)/2, a1, b1, c1, r1);
	
//	cameraMoveTo(a1, b1, c1-600*r1, 1.0);
//	cameraLookAt(a1, b1, c1, 1.0);

//	measure.MeasureDifficultly(unitSims[windowID]->GetEnvironment(), a, b);
//	measure.ShowHistogram();
	
//	LocalSensing::RIBS<xyLoc, tDirection, MapEnvironment> *u1 = new LocalSensing::RIBS<xyLoc, tDirection, MapEnvironment>(a, b);
//	u1->SetWeight(1.0);
//	u1->SetSpeed(0.02);
//	unitSims[windowID]->AddUnit(u1);

//	LSSLRTAStarUnit<xyLoc, tDirection, MapEnvironment> *u2 = new LSSLRTAStarUnit<xyLoc, tDirection, MapEnvironment>(a, b, new LSSLRTAStar<xyLoc, tDirection, MapEnvironment>(100));
//	u2->SetSpeed(0.02);
//	unitSims[windowID]->AddUnit(u2);
//	
//	LSSLRTAStarUnit<xyLoc, tDirection, MapEnvironment> *u3 = new LSSLRTAStarUnit<xyLoc, tDirection, MapEnvironment>(a, b, new LSSLRTAStar<xyLoc, tDirection, MapEnvironment>(10));
//	u3->SetSpeed(0.02);
//	unitSims[windowID]->AddUnit(u3);

//	FLRTAStarUnit<xyLoc, tDirection, MapEnvironment> *u4 = new FLRTAStarUnit<xyLoc, tDirection, MapEnvironment>(a, b, new FLRTA::FLRTAStar<xyLoc, tDirection, MapEnvironment>(100));
//	u4->SetSpeed(0.02);
//	unitSims[windowID]->AddUnit(u4);

//	FLRTAStarUnit<xyLoc, tDirection, MapEnvironment> *u5 = new FLRTAStarUnit<xyLoc, tDirection, MapEnvironment>(a, b, new FLRTA::FLRTAStar<xyLoc, tDirection, MapEnvironment>(1));
//	u5->SetSpeed(0.02);
//	unitSims[windowID]->AddUnit(u5);

	FLRTAStarUnit<xyLoc, tDirection, MapEnvironment> *u6 = new FLRTAStarUnit<xyLoc, tDirection, MapEnvironment>(a, b, new FLRTA::FLRTAStar<xyLoc, tDirection, MapEnvironment>(10));
	u6->SetSpeed(0.02);
	unitSims[windowID]->AddUnit(u6);
	
	unitSims[windowID]->GetStats()->AddFilter("trialDistanceMoved");
	unitSims[windowID]->GetStats()->AddFilter("TotalLearning");
	unitSims[windowID]->GetStats()->EnablePrintOutput(true);
	unitSims[windowID]->SetTrialLimit(50000);
}

void MyPathfindingKeyHandler(unsigned long windowID, tKeyboardModifier , char)
{
	std::vector<graphState> thePath;
	MapEnvironment *env = unitSims[windowID]->GetEnvironment();
	Map *m = env->GetMap();
	Graph *g = GraphSearchConstants::GetGraph(m);

//	GraphDistanceHeuristic diffHeuristic(g);
//	diffHeuristic.UseSmartPlacement(true);
//	for (int x = 0; x < 20; x++)
//		diffHeuristic.AddHeuristic();

	GraphMapInconsistentHeuristic diffHeuristic(m, g);
	diffHeuristic.UseSmartPlacement(true);
	for (int x = 0; x < 20; x++)
		diffHeuristic.AddHeuristic();
	
	GraphEnvironment gEnv(g, &diffHeuristic);
	gEnv.SetDirected(true);

	diffHeuristic.SetMode(kRandom);
	diffHeuristic.SetNumUsedHeuristics(2);

	TemplateAStar<graphState, graphMove, GraphEnvironment> taNew;
	TemplateAStar<graphState, graphMove, GraphEnvironment> taOld;
	Timer t;
	
	for (int x = 0; x < 500; x++)
	{
		graphState s1, g1;
		do {
			s1 = g->GetRandomNode()->GetNum();
			g1 = g->GetRandomNode()->GetNum();
		} while (gEnv.HCost(s1, g1) < 100);
		double firstLength;

		taNew.SetUseBPMX(false);
		t.StartTimer();
		taNew.GetPath(&gEnv, s1, g1, thePath);
		t.EndTimer();
		printf("old: %lld nodes expanded. Path length %d / %f. Time: %f\nrate0: %lld %f\n", 
			   taNew.GetNodesExpanded(), (int)thePath.size(), gEnv.GetPathLength(thePath), t.GetElapsedTime(),
			   taNew.GetNodesExpanded(), taNew.GetNodesExpanded()/t.GetElapsedTime());
		firstLength = gEnv.GetPathLength(thePath);

		for (int y = 1; y < 5; y++)
		{
			taNew.SetUseBPMX(y);
			t.StartTimer();
			taNew.GetPath(&gEnv, s1, g1, thePath);
			t.EndTimer();
			printf("new: %lld nodes expanded. Path length %d / %f. Time: %f\nrate%d: %lld %f\n", 
				   taNew.GetNodesExpanded(), (int)thePath.size(), gEnv.GetPathLength(thePath), t.GetElapsedTime(),
				   y, taNew.GetNodesExpanded(), taNew.GetNodesExpanded()/t.GetElapsedTime());

			if (gEnv.GetPathLength(thePath) != firstLength)
				printf("\n\n\n!!!!!!!!!!!!!!!!!! IT FAILED!!!!!!!!!!!!!!!!!!!!!!!!\n\n\n\n");
		}
		printf("--\n");
	}	
	delete g;
}

bool MyClickHandler(unsigned long windowID, int, int, point3d loc, tButtonType button, tMouseEventType mType)
{
	mouseTracking = false;
	if (button == kRightButton)
	{
		switch (mType)
		{
			case kMouseDown:
				unitSims[windowID]->GetEnvironment()->GetMap()->GetPointFromCoordinate(loc, px1, py1);
				//printf("Mouse down at (%d, %d)\n", px1, py1);
				break;
			case kMouseDrag:
				mouseTracking = true;
				unitSims[windowID]->GetEnvironment()->GetMap()->GetPointFromCoordinate(loc, px2, py2);
				//printf("Mouse tracking at (%d, %d)\n", px2, py2);
				break;
			case kMouseUp:
			{
				if ((px1 == -1) || (px2 == -1))
					break;
				unitSims[windowID]->GetEnvironment()->GetMap()->GetPointFromCoordinate(loc, px2, py2);
				printf("Searching from (%d, %d) to (%d, %d)\n", px1, py1, px2, py2);
				
				if (ma1 == 0)
				{
					ma1 = new MapEnvironment(unitSims[windowID]->GetEnvironment()->GetMap());
					gdh = new GraphDistanceHeuristic(GraphSearchConstants::GetGraph(ma1->GetMap()));
					gdh->UseSmartPlacement(true);
					ma1->SetGraphHeuristic(gdh);
					for (int x = 0; x < 10; x++)
						gdh->AddHeuristic();
				}
				if (ma2 == 0)
					ma2 = new MapEnvironment(unitSims[windowID]->GetEnvironment()->GetMap());

				a1.SetStopAfterGoal(true);
				a2.SetStopAfterGoal(true);
				xyLoc s1;
				xyLoc g1;
				s1.x = px1; s1.y = py1;
				g1.x = px2; g1.y = py2;
					   
				a1.InitializeSearch(ma1, s1, g1, path);
				a2.InitializeSearch(ma2, s1, g1, path);
				runningSearch1 = true;
				runningSearch2 = true;
				
			}
			break;
		}
		return true;
	}
	return false;
}


typedef EpisodicSimulation<xyLoc, tDirection, MapEnvironment> EpSim;
void RunSingleTest(EpSim *es, const Experiment &e, int which);

void RunScenario(char *name, int which)
{
	ScenarioLoader *sl = new ScenarioLoader(name);
	printf("Loading map: %s\n", sl->GetNthExperiment(0).GetMapName());
	
	Map *map = new Map(sl->GetNthExperiment(0).GetMapName());
	map->Scale(sl->GetNthExperiment(0).GetXScale(),
			   sl->GetNthExperiment(0).GetYScale());
	MapEnvironment *me;
	EpSim *es = new EpSim(me = new MapEnvironment(map, false));
	me->SetDiagonalCost(1.5);
	es->SetStepType(kRealTime);
	es->SetThinkingPenalty(0);

	for (int x = 0; x < sl->GetNumExperiments(); x++)
	{
		printf("Experiment %d of %d\n", x+1, sl->GetNumExperiments());
		RunSingleTest(es, sl->GetNthExperiment(x), which);
	}
	exit(0);
}

void RunSingleTest(EpSim *es, const Experiment &e, int which)
{
	es->ClearAllUnits();
	// add units
	es->GetStats()->ClearAllStats();
	es->GetStats()->AddFilter("trialDistanceMoved");
	es->GetStats()->AddFilter("nodesTouched");
	es->GetStats()->AddFilter("nodesExpanded");
	es->GetStats()->AddFilter("TotalLearning");
	es->GetStats()->AddFilter("Trial End");
	es->GetStats()->EnablePrintOutput(false);
	xyLoc a(e.GetStartX(), e.GetStartY()), b(e.GetGoalX(), e.GetGoalY());

	HeuristicLearningMeasure<xyLoc, tDirection, MapEnvironment> measure;
	double requiredLearning = measure.MeasureDifficultly(es->GetEnvironment(), a, b);
	
	if (which == 0)
	{
		printf("Running RIBS\n");
		LocalSensing::RIBS<xyLoc, tDirection, MapEnvironment> *u1 = new LocalSensing::RIBS<xyLoc, tDirection, MapEnvironment>(a, b);
		u1->SetSpeed(1.0);
		es->AddUnit(u1); // go to goal and stop
	}
	else if (which == 1)
	{
		printf("Running LRTA*\n");
		LRTAStarUnit<xyLoc, tDirection, MapEnvironment> *u1 = new LRTAStarUnit<xyLoc, tDirection, MapEnvironment>(a, b, new LRTAStar<xyLoc, tDirection, MapEnvironment>());
		u1->SetSpeed(1.0);
		es->AddUnit(u1);
	}
	else if (which == 2)
	{
		printf("Running LSS-LRTA*(10)\n");
		LSSLRTAStarUnit<xyLoc, tDirection, MapEnvironment> *u1 = new LSSLRTAStarUnit<xyLoc, tDirection, MapEnvironment>(a, b, new LSSLRTAStar<xyLoc, tDirection, MapEnvironment>(10));
		u1->SetSpeed(1.0);
		es->AddUnit(u1);
	}
	else if (which == 3)
	{
		printf("Running LSS-LRTA*(100)\n");
		LSSLRTAStarUnit<xyLoc, tDirection, MapEnvironment> *u1 = new LSSLRTAStarUnit<xyLoc, tDirection, MapEnvironment>(a, b, new LSSLRTAStar<xyLoc, tDirection, MapEnvironment>(100));
		u1->SetSpeed(1.0);
		es->AddUnit(u1);
	}
	else if (which == 4)
	{
		printf("Running FLRTA*(1)\n");
		FLRTAStarUnit<xyLoc, tDirection, MapEnvironment> *u5 = new FLRTAStarUnit<xyLoc, tDirection, MapEnvironment>(a, b, new FLRTA::FLRTAStar<xyLoc, tDirection, MapEnvironment>(1));
		u5->SetSpeed(1.0);
		es->AddUnit(u5);
	}
	else if (which == 5)
	{
		printf("Running FLRTA*(10)\n");
		FLRTAStarUnit<xyLoc, tDirection, MapEnvironment> *u5 = new FLRTAStarUnit<xyLoc, tDirection, MapEnvironment>(a, b, new FLRTA::FLRTAStar<xyLoc, tDirection, MapEnvironment>(10));
		u5->SetSpeed(1.0);
		es->AddUnit(u5);
	}
	else if (which == 6)
	{
		printf("Running FLRTA*(100)\n");
		FLRTAStarUnit<xyLoc, tDirection, MapEnvironment> *u5 = new FLRTAStarUnit<xyLoc, tDirection, MapEnvironment>(a, b, new FLRTA::FLRTAStar<xyLoc, tDirection, MapEnvironment>(100));
		u5->SetSpeed(1.0);
		es->AddUnit(u5);
	}
	
	es->SetTrialLimit(500000);
	while (!es->Done())
	{
		es->StepTime(10.0);
	}
	statValue v;
	printf("Done\n");
	int choice = es->GetStats()->FindNextStat("trialDistanceMoved", es->GetUnit(0)->GetName(), 0);
	es->GetStats()->LookupStat(choice, v);
	printf("dist-first\t%s\t%d\t%f\n", es->GetUnit(0)->GetName(), e.GetBucket(), v.fval);
	printf("dist-sum\t%s\t%d\t%f\n", es->GetUnit(0)->GetName(), e.GetBucket(), SumStatEntries(es->GetStats(), "trialDistanceMoved", es->GetUnit(0)->GetName()));
	es->GetStats()->LookupStat("TotalLearning", es->GetUnit(0)->GetName(), v);
	printf("learning\t%s\t%f\t%f\t%f\n", es->GetUnit(0)->GetName(), v.fval, requiredLearning, v.fval/requiredLearning);
	
	choice = es->GetStats()->FindNextStat("nodesExpanded", es->GetUnit(0)->GetName(), 0);
	if (choice != -1)
	{
		es->GetStats()->LookupStat(choice, v);
		printf("first-nodesExpanded\t%s\t%d\t%ld\n", es->GetUnit(0)->GetName(), e.GetBucket(), v.lval);
		printf("sum-nodesExpanded\t%s\t%d\t%f\n", es->GetUnit(0)->GetName(), e.GetBucket(), SumStatEntries(es->GetStats(), "nodesExpanded", es->GetUnit(0)->GetName()));
	}
	choice = es->GetStats()->FindNextStat("nodesTouched", es->GetUnit(0)->GetName(), 0);
	if (choice != -1)
	{
		es->GetStats()->LookupStat(choice, v);
		printf("first-nodesTouched\t%s\t%d\t%ld\n", es->GetUnit(0)->GetName(), e.GetBucket(), v.lval);
		printf("sum-nodesTouched\t%s\t%d\t%f\n", es->GetUnit(0)->GetName(), e.GetBucket(), SumStatEntries(es->GetStats(), "nodesTouched", es->GetUnit(0)->GetName()));
	}
	
	es->GetStats()->LookupStat("Trial End", "Race Simulation", v);
	printf("trials\t%s\t%d\t%ld\n", es->GetUnit(0)->GetName(), e.GetBucket(), v.lval+1);
}

void RunBigTest(EpSim *es, const Experiment &e, int which);

void RunBigScenario(char *name, int which)
{
	ScenarioLoader *sl = new ScenarioLoader(name);
	printf("Loading map: %s\n", sl->GetNthExperiment(0).GetMapName());
	
	Map *map = new Map(sl->GetNthExperiment(0).GetMapName());
	map->Scale(sl->GetNthExperiment(0).GetXScale()*2,
			   sl->GetNthExperiment(0).GetYScale()*2);
	MapEnvironment *me;
	EpSim *es = new EpSim(me = new MapEnvironment(map, false));
	me->SetDiagonalCost(1.5);
	es->SetStepType(kRealTime);
	es->SetThinkingPenalty(0);
	
	for (int x = 0; x < sl->GetNumExperiments(); x++)
	{
		printf("Experiment %d of %d\n", x+1, sl->GetNumExperiments());
		RunBigTest(es, sl->GetNthExperiment(x), which);
	}
	exit(0);
}

void RunBigTest(EpSim *es, const Experiment &e, int which)
{
	if (e.GetBucket() != 127)
		return;
	
	es->ClearAllUnits();
	// add units
	es->GetStats()->ClearAllStats();
	es->GetStats()->AddFilter("trialDistanceMoved");
	es->GetStats()->AddFilter("nodesTouched");
	es->GetStats()->AddFilter("nodesExpanded");
	es->GetStats()->AddFilter("TotalLearning");
	es->GetStats()->AddFilter("Trial End");
	es->GetStats()->EnablePrintOutput(false);
	xyLoc a(2*e.GetStartX(), 2*e.GetStartY()), b(2*e.GetGoalX(), 2*e.GetGoalY());
	
	//HeuristicLearningMeasure<xyLoc, tDirection, MapEnvironment> measure;
	double requiredLearning = 1;//measure.MeasureDifficultly(es->GetEnvironment(), a, b);
	
	if (which == 0)
	{
		printf("Running RIBS\n");
		LocalSensing::RIBS<xyLoc, tDirection, MapEnvironment> *u1 = new LocalSensing::RIBS<xyLoc, tDirection, MapEnvironment>(a, b);
		u1->SetSpeed(1.0);
		es->AddUnit(u1); // go to goal and stop
	}
	else if (which == 1)
	{
		printf("Running LRTA*\n");
		LRTAStarUnit<xyLoc, tDirection, MapEnvironment> *u1 = new LRTAStarUnit<xyLoc, tDirection, MapEnvironment>(a, b, new LRTAStar<xyLoc, tDirection, MapEnvironment>());
		u1->SetSpeed(1.0);
		es->AddUnit(u1);
	}
	else if (which == 2)
	{
		printf("Running LSS-LRTA*(10)\n");
		LSSLRTAStarUnit<xyLoc, tDirection, MapEnvironment> *u1 = new LSSLRTAStarUnit<xyLoc, tDirection, MapEnvironment>(a, b, new LSSLRTAStar<xyLoc, tDirection, MapEnvironment>(10));
		u1->SetSpeed(1.0);
		es->AddUnit(u1);
	}
	else if (which == 3)
	{
		printf("Running LSS-LRTA*(100)\n");
		LSSLRTAStarUnit<xyLoc, tDirection, MapEnvironment> *u1 = new LSSLRTAStarUnit<xyLoc, tDirection, MapEnvironment>(a, b, new LSSLRTAStar<xyLoc, tDirection, MapEnvironment>(100));
		u1->SetSpeed(1.0);
		es->AddUnit(u1);
	}
	
	es->SetTrialLimit(500000);
	while (!es->Done())
	{
		es->StepTime(10.0);
	}
	statValue v;
	printf("Done\n");
	int choice = es->GetStats()->FindNextStat("trialDistanceMoved", es->GetUnit(0)->GetName(), 0);
	es->GetStats()->LookupStat(choice, v);
	printf("first\t%s\t%d\t%f\n", es->GetUnit(0)->GetName(), e.GetBucket(), v.fval);
	printf("sum\t%s\t%d\t%f\n", es->GetUnit(0)->GetName(), e.GetBucket(), SumStatEntries(es->GetStats(), "trialDistanceMoved", es->GetUnit(0)->GetName()));
	es->GetStats()->LookupStat("TotalLearning", es->GetUnit(0)->GetName(), v);
	printf("learning\t%s\t%f\t%f\t%f\n", es->GetUnit(0)->GetName(), v.fval, requiredLearning, v.fval/requiredLearning);
	
	choice = es->GetStats()->FindNextStat("nodesExpanded", es->GetUnit(0)->GetName(), 0);
	if (choice != -1)
	{
		es->GetStats()->LookupStat(choice, v);
		printf("first-nodesExpanded\t%s\t%d\t%ld\n", es->GetUnit(0)->GetName(), e.GetBucket(), v.lval);
		printf("sum-nodesExpanded\t%s\t%d\t%f\n", es->GetUnit(0)->GetName(), e.GetBucket(), SumStatEntries(es->GetStats(), "nodesExpanded", es->GetUnit(0)->GetName()));
	}
	choice = es->GetStats()->FindNextStat("nodesTouched", es->GetUnit(0)->GetName(), 0);
	if (choice != -1)
	{
		es->GetStats()->LookupStat(choice, v);
		printf("first-nodesTouched\t%s\t%d\t%ld\n", es->GetUnit(0)->GetName(), e.GetBucket(), v.lval);
		printf("sum-nodesTouched\t%s\t%d\t%f\n", es->GetUnit(0)->GetName(), e.GetBucket(), SumStatEntries(es->GetStats(), "nodesTouched", es->GetUnit(0)->GetName()));
	}
	
	es->GetStats()->LookupStat("Trial End", "Race Simulation", v);
	printf("trials\t%s\t%d\t%ld\n", es->GetUnit(0)->GetName(), e.GetBucket(), v.lval+1);
}

void RunScalingTest(int size, int which)
{
	Map *map = new Map(size, size);
	map->SetTerrainType(1, size-2,
						size-2, size-2, kOutOfBounds);
	map->SetTerrainType(size-2, 1,
						size-2, size-2, kOutOfBounds);

	MapEnvironment *me;
	EpSim *es = new EpSim(me = new MapEnvironment(map, false));
	me->SetDiagonalCost(1.5);
	es->SetStepType(kRealTime);
	es->SetThinkingPenalty(0);
	
	es->ClearAllUnits();
	// add units
	es->GetStats()->AddFilter("trialDistanceMoved");
	es->GetStats()->AddFilter("nodesTouched");
	es->GetStats()->AddFilter("nodesExpanded");
	es->GetStats()->AddFilter("TotalLearning");
	es->GetStats()->AddFilter("Trial End");
	es->GetStats()->EnablePrintOutput(false);
	xyLoc a(0, 0), b(size-1, size-1);
	
	HeuristicLearningMeasure<xyLoc, tDirection, MapEnvironment> measure;
	double requiredLearning = measure.MeasureDifficultly(es->GetEnvironment(), a, b);

	if (which == 0)
	{
		printf("Running RIBS\n");
		LocalSensing::RIBS<xyLoc, tDirection, MapEnvironment> *u1 = new LocalSensing::RIBS<xyLoc, tDirection, MapEnvironment>(a, b);
		u1->SetSpeed(1.0);
		es->AddUnit(u1); // go to goal and stop
	}
	else if (which == 1)
	{
		printf("Running LRTA*\n");
		LRTAStarUnit<xyLoc, tDirection, MapEnvironment> *u1 = new LRTAStarUnit<xyLoc, tDirection, MapEnvironment>(a, b, new LRTAStar<xyLoc, tDirection, MapEnvironment>());
		u1->SetSpeed(1.0);
		es->AddUnit(u1);
	}
	else if (which == 2)
	{
		printf("Running LSS-LRTA*(10)\n");
		LSSLRTAStarUnit<xyLoc, tDirection, MapEnvironment> *u1 = new LSSLRTAStarUnit<xyLoc, tDirection, MapEnvironment>(a, b, new LSSLRTAStar<xyLoc, tDirection, MapEnvironment>(10));
		u1->SetSpeed(1.0);
		es->AddUnit(u1);
	}
	else if (which == 3)
	{
		printf("Running LSS-LRTA*(100)\n");
		LSSLRTAStarUnit<xyLoc, tDirection, MapEnvironment> *u1 = new LSSLRTAStarUnit<xyLoc, tDirection, MapEnvironment>(a, b, new LSSLRTAStar<xyLoc, tDirection, MapEnvironment>(100));
		u1->SetSpeed(1.0);
		es->AddUnit(u1);
	}
	else if (which == 4)
	{
		printf("Running FLRTA*(1)\n");
		FLRTAStarUnit<xyLoc, tDirection, MapEnvironment> *u5 = new FLRTAStarUnit<xyLoc, tDirection, MapEnvironment>(a, b, new FLRTA::FLRTAStar<xyLoc, tDirection, MapEnvironment>(1));
		u5->SetSpeed(0.02);
		es->AddUnit(u5);
	}
	else if (which == 5)
	{
		printf("Running FLRTA*(10)\n");
		FLRTAStarUnit<xyLoc, tDirection, MapEnvironment> *u5 = new FLRTAStarUnit<xyLoc, tDirection, MapEnvironment>(a, b, new FLRTA::FLRTAStar<xyLoc, tDirection, MapEnvironment>(10));
		u5->SetSpeed(0.02);
		es->AddUnit(u5);
	}
	else if (which == 6)
	{
		printf("Running FLRTA*(100)\n");
		FLRTAStarUnit<xyLoc, tDirection, MapEnvironment> *u5 = new FLRTAStarUnit<xyLoc, tDirection, MapEnvironment>(a, b, new FLRTA::FLRTAStar<xyLoc, tDirection, MapEnvironment>(100));
		u5->SetSpeed(0.02);
		es->AddUnit(u5);
	}
	fflush(stdout);
	es->SetTrialLimit(500000);
	while (!es->Done())
	{
		es->StepTime(10.0);
	}
	statValue v;
	printf("Done\n");
	int choice = es->GetStats()->FindNextStat("trialDistanceMoved", es->GetUnit(0)->GetName(), 0);
	es->GetStats()->LookupStat(choice, v);
	printf("dist-first\t%s\t%d\t%f\n", es->GetUnit(0)->GetName(), size, v.fval);
	printf("dist-sum\t%s\t%d\t%f\n", es->GetUnit(0)->GetName(), size, SumStatEntries(es->GetStats(), "trialDistanceMoved", es->GetUnit(0)->GetName()));

	if (es->GetStats()->LookupStat("TotalLearning", es->GetUnit(0)->GetName(), v))
		printf("learning\t%s\t%f\t%f\t%f\n", es->GetUnit(0)->GetName(), v.fval, requiredLearning, v.fval/requiredLearning);

	choice = es->GetStats()->FindNextStat("nodesExpanded", es->GetUnit(0)->GetName(), 0);
	if (choice != -1)
	{
		es->GetStats()->LookupStat(choice, v);
		printf("first-nodesExpanded\t%s\t%d\t%ld\n", es->GetUnit(0)->GetName(), size, v.lval);
		printf("sum-nodesExpanded\t%s\t%d\t%f\n", es->GetUnit(0)->GetName(), size, SumStatEntries(es->GetStats(), "nodesExpanded", es->GetUnit(0)->GetName()));
	}
	choice = es->GetStats()->FindNextStat("nodesTouched", es->GetUnit(0)->GetName(), 0);
	if (choice != -1)
	{
		es->GetStats()->LookupStat(choice, v);
		printf("first-nodesTouched\t%s\t%d\t%ld\n", es->GetUnit(0)->GetName(), size, v.lval);
		printf("sum-nodesTouched\t%s\t%d\t%f\n", es->GetUnit(0)->GetName(), size, SumStatEntries(es->GetStats(), "nodesTouched", es->GetUnit(0)->GetName()));
	}
	
	es->GetStats()->LookupStat("Trial End", "Race Simulation", v);
	printf("trials\t%s\t%d\t%ld\n", es->GetUnit(0)->GetName(), size, v.lval+1);
	exit(0);
}

#include "MNPuzzle.h"

void RunWorkMeasureTest()
{
	MNPuzzle *mnp = new MNPuzzle(4, 4);
	srandom(81);
	for (int x = 0; x < 50; x++)
	{
		HeuristicLearningMeasure<MNPuzzleState, slideDir, MNPuzzle> measure;
		MNPuzzleState s(4, 4);
		MNPuzzleState g(4, 4);
		std::vector<slideDir> acts;
		for (unsigned int y = 0; y < 100; y++)
		{
			mnp->GetActions(s, acts);
			mnp->ApplyAction(s, acts[random()%acts.size()]);
		}
		std::cout << "Start: " << s << std::endl << "Goal: " << g << std::endl;
		std::cout << "Heuristic: " << mnp->HCost(s, g) << std::endl;

		double requiredLearning = measure.MeasureDifficultly(mnp, s, g);
		std::cout << "Required learning: " << requiredLearning << std::endl;
		measure.ShowHistogram();
	}
}


void RunSTPTest(int which)
{
	typedef EpisodicSimulation<MNPuzzleState, slideDir, MNPuzzle> STPSim;
	
	MNPuzzle *mnp = new MNPuzzle(4, 4);

	srandom(81);

	STPSim *es = new STPSim(mnp);
	es->SetStepType(kRealTime);
	es->SetThinkingPenalty(0);
	
	// add units
	es->GetStats()->AddFilter("trialDistanceMoved");
	es->GetStats()->AddFilter("nodesTouched");
	es->GetStats()->AddFilter("nodesExpanded");
	es->GetStats()->AddFilter("TotalLearning");
	es->GetStats()->AddFilter("Trial End");
	es->GetStats()->EnablePrintOutput(false);
	
	for (int x = 0; x < 500; x++)
	{
		es->ClearAllUnits();
		es->GetStats()->ClearAllStats();

		MNPuzzleState s(4, 4);
		MNPuzzleState g(4, 4);
		std::vector<slideDir> acts;
		for (unsigned int y = 0; y < 100; y++)
		{
			mnp->GetActions(s, acts);
			mnp->ApplyAction(s, acts[random()%acts.size()]);
		}
		std::cout << "Start: " << s << std::endl << "Goal: " << g << std::endl;
		std::cout << "Heuristic: " << mnp->HCost(s, g) << std::endl;
		
		double requiredLearning;
		{
			HeuristicLearningMeasure<MNPuzzleState, slideDir, MNPuzzle> measure;
			requiredLearning = measure.MeasureDifficultly(mnp, s, g);
			std::cout << "Required learning: " << requiredLearning << std::endl;
		}
		
		if (which == 0)
		{
			LocalSensing::RIBS<MNPuzzleState, slideDir, MNPuzzle> *u1 = new LocalSensing::RIBS<MNPuzzleState, slideDir, MNPuzzle>(s, g);
			u1->SetSpeed(1.0);
			es->AddUnit(u1); // go to goal and stop
		}
		else if (which == 1)
		{
			LRTAStarUnit<MNPuzzleState, slideDir, MNPuzzle> *u1 = new LRTAStarUnit<MNPuzzleState, slideDir, MNPuzzle>(s, g, new LRTAStar<MNPuzzleState, slideDir, MNPuzzle>());
			u1->SetSpeed(1.0);
			es->AddUnit(u1);
		}
		else if (which == 2)
		{
			LSSLRTAStarUnit<MNPuzzleState, slideDir, MNPuzzle> *u1 = new LSSLRTAStarUnit<MNPuzzleState, slideDir, MNPuzzle>(s, g, new LSSLRTAStar<MNPuzzleState, slideDir, MNPuzzle>(10));
			u1->SetSpeed(1.0);
			es->AddUnit(u1);
		}
		else if (which == 3)
		{
			LSSLRTAStarUnit<MNPuzzleState, slideDir, MNPuzzle> *u1 = new LSSLRTAStarUnit<MNPuzzleState, slideDir, MNPuzzle>(s, g, new LSSLRTAStar<MNPuzzleState, slideDir, MNPuzzle>(100));
			u1->SetSpeed(1.0);
			es->AddUnit(u1);
		}
		else if (which == 4)
		{
			FLRTAStarUnit<MNPuzzleState, slideDir, MNPuzzle> *u4 = new FLRTAStarUnit<MNPuzzleState, slideDir, MNPuzzle>(s, g, new FLRTA::FLRTAStar<MNPuzzleState, slideDir, MNPuzzle>(1));
			u4->SetSpeed(1.0);
			es->AddUnit(u4);
		}
		else if (which == 5)
		{
			FLRTAStarUnit<MNPuzzleState, slideDir, MNPuzzle> *u4 = new FLRTAStarUnit<MNPuzzleState, slideDir, MNPuzzle>(s, g, new FLRTA::FLRTAStar<MNPuzzleState, slideDir, MNPuzzle>(10));
			u4->SetSpeed(1.0);
			es->AddUnit(u4);
		}
		else if (which == 6)
		{
			FLRTAStarUnit<MNPuzzleState, slideDir, MNPuzzle> *u4 = new FLRTAStarUnit<MNPuzzleState, slideDir, MNPuzzle>(s, g, new FLRTA::FLRTAStar<MNPuzzleState, slideDir, MNPuzzle>(100));
			u4->SetSpeed(1.0);
			es->AddUnit(u4);
		}
		
		es->SetTrialLimit(100000);
		while (!es->Done())
		{
			es->StepTime(10.0);
		}

		statValue v;
		printf("Done\n");
		int choice = es->GetStats()->FindNextStat("trialDistanceMoved", es->GetUnit(0)->GetName(), 0);
		es->GetStats()->LookupStat(choice, v);
		printf("dist-first\t%s\t%d\t%f\n", es->GetUnit(0)->GetName(), x, v.fval);
		printf("dist-sum\t%s\t%d\t%f\n", es->GetUnit(0)->GetName(), x, SumStatEntries(es->GetStats(), "trialDistanceMoved", es->GetUnit(0)->GetName()));
		es->GetStats()->LookupStat("TotalLearning", es->GetUnit(0)->GetName(), v);
		printf("learning\t%s\t%f\t%f\t%f\n", es->GetUnit(0)->GetName(), v.fval, requiredLearning, v.fval/requiredLearning);
		
		choice = es->GetStats()->FindNextStat("nodesExpanded", es->GetUnit(0)->GetName(), 0);
		if (choice != -1)
		{
			es->GetStats()->LookupStat(choice, v);
			printf("first-nodesExpanded\t%s\t%d\t%ld\n", es->GetUnit(0)->GetName(), x, v.lval);
			printf("sum-nodesExpanded\t%s\t%d\t%f\n", es->GetUnit(0)->GetName(), x, SumStatEntries(es->GetStats(), "nodesExpanded", es->GetUnit(0)->GetName()));
		}
		choice = es->GetStats()->FindNextStat("nodesTouched", es->GetUnit(0)->GetName(), 0);
		if (choice != -1)
		{
			es->GetStats()->LookupStat(choice, v);
			printf("first-nodesTouched\t%s\t%d\t%ld\n", es->GetUnit(0)->GetName(), x, v.lval);
			printf("sum-nodesTouched\t%s\t%d\t%f\n", es->GetUnit(0)->GetName(), x, SumStatEntries(es->GetStats(), "nodesTouched", es->GetUnit(0)->GetName()));
		}
		
		es->GetStats()->LookupStat("Trial End", "Race Simulation", v);
		printf("trials\t%s\t%d\t%ld\n", es->GetUnit(0)->GetName(), x, v.lval+1);
	}
	exit(0);
}
