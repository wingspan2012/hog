
/*
 * $Id: Inconsistency.cpp,v 1.23 2006/11/01 23:33:56 nathanst Exp $
 *
 *  Inconsistency.cpp
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
#include "Inconsistency.h"
#include "SearchUnit.h"
#include "UnitSimulation.h"
#include "EpisodicSimulation.h"
#include "Plot2D.h"
#include "Propagation.h"
#include "GraphEnvironment.h"
#include "GraphAlgorithm.h"
#include "AStarDelay.h"
#include "ScenarioLoader.h"
#include "TemplateAStar.h"

Graph *BuildInconsistentGraph(int d, int w, int h, int c1 = 1000, int c2 = 1000);
void RunInconsistencyExperiment();
void RunExperiments(ScenarioLoader *sl);

bool mouseTracking;
int px1, py1, px2, py2;
int absType = 0;
//int GraphSizeN = 10;

std::vector<GraphSimulation *> unitSims;

//extern bool drawtext;

unsigned int fig = 4;
unsigned int N = 5;
unsigned int vid = PROP_BP;//6;//PROP_A;//

double delta = 0;

graphState gFrom, gTo;
std::vector<graphState> gPath;
Graph* grp=0;
Map* mp=0;
GraphEnvironment* env = 0;

GraphAlgorithm* ALG = 0;

bool done = false;

int main(int argc, char* argv[])
{
	preProcessArgs(argc,argv);
	InstallHandlers();
	RunHOGGUI(argc, argv);
}


void preProcessArgs(int argc, char* argv[]) 
{
	// -fig fig -N n -v vid -delta del -map mapfile
	int i = 1;
	gDefaultMap[0] = 0;
	while(i<argc) {
		if(strcmp(argv[i],"-fig")==0) {
			fig = atoi(argv[i+1]);
			i += 2;
		}
		else if(strcmp(argv[i],"-N")==0) {
			N = atoi(argv[i+1]);
			i += 2;
		}
		else if(strcmp(argv[i],"-v")==0) {
			vid = atoi(argv[i+1]);
			i += 2;
		}
		else if(strcmp(argv[i],"-delta")==0) {
			sscanf(argv[i+1],"%lf",&delta);
			i += 2;
		}
		else if(strcmp(argv[i],"-map")==0) {
			strcpy(gDefaultMap,argv[i+1]);
			i += 2;
		}
		else if(strcmp(argv[i],"-dt")==0) {
		  //	drawtext = true;
			i++;
		}
		else
			i++;
	}

	if(strlen(gDefaultMap)) 
	{
		FILE * mf = fopen(gDefaultMap,"r");
		if(!mf)
		{
			printf("Map file not exists.\n");
			exit(-1);
		}
		else
		{
			fclose(mf);
		}
	}
}


Graph *BuildInconsistentGraph(int d, int w, int h, int c1, int c2)
{
	//int d = 2;
	gFrom = d*w*h-1;
	gTo = 0;
	
	Graph *g = new Graph();
	int cost[d][w][h];
	int index[d][w][h];
	for (int z = 0; z < d; z++)
	{
		for (int x = 0; x < w; x++)
		{
			for (int y = 0; y < h; y++)
			{
				node *n;
				cost[z][x][y] = MAXINT;
				index[z][x][y] = g->AddNode(n = new node(""));
				n->SetLabelF(GraphSearchConstants::kXCoordinate, -1.0+2.0*(double)x/(double)(w-1.0));
				n->SetLabelF(GraphSearchConstants::kYCoordinate, -1.0+2.0*(double)y/(double)(h-1.0));
				if (d > 1)
					n->SetLabelF(GraphSearchConstants::kZCoordinate, -1.0+2.0*(double)z/(double)(d-1.0));
				else
					n->SetLabelF(GraphSearchConstants::kZCoordinate, 0);
			}
		}
	}
	for (int z = 0; z < d; z++)
	{
		for (int x = 0; x < w; x++)
		{
			for (int y = 0; y < h; y++)
			{
				if ((x == 0) && (y == 0) && (z == 0))
				{
					if (d > 1)
						g->AddEdge(new edge(index[z+1][x][y], index[z][x][y], c2));
					if (w > 1)
						g->AddEdge(new edge(index[z][x+1][y], index[z][x][y], c2));
					g->AddEdge(new edge(index[z][x][y+1], index[z][x][y], c2));
					continue;
				}
				if (z+1 <d)
					g->AddEdge(new edge(index[z+1][x][y], index[z][x][y], 1+random()%c1));
				if (x+1 <w)
					g->AddEdge(new edge(index[z][x+1][y], index[z][x][y], 1+random()%c1));
				if (y+1 < h)
					g->AddEdge(new edge(index[z][x][y+1], index[z][x][y], 1+random()%c1));
			}
		}
	}
	cost[0][0][0] = 0;
	while (1)
	{
		int changes = 0;

		for (int z = 0; z < d; z++)
		{
			for (int x = 0; x < w; x++)
			{
				for (int y = 0; y < h; y++)
				{
					if (z+1 <d)
					{
						int from1 = index[z][x][y];
						int to1 = index[z+1][x][y];
						edge *e = g->FindEdge(from1, to1);
						if (cost[z+1][x][y] > cost[z][x][y] + e->GetWeight())
						{
							cost[z+1][x][y] = cost[z][x][y] + g->FindEdge(index[z][x][y], index[z+1][x][y])->GetWeight();
							changes++;
						}
					}
					if (x+1 <w)
					{
						if (cost[z][x+1][y] > cost[z][x][y] + g->FindEdge(index[z][x][y], index[z][x+1][y])->GetWeight())
						{
							cost[z][x+1][y] = cost[z][x][y] + g->FindEdge(index[z][x][y], index[z][x+1][y])->GetWeight();
							changes++;
						}
					}
					if (y+1 < h)
					{
						if (cost[z][x][y+1] > cost[z][x][y] + g->FindEdge(index[z][x][y], index[z][x][y+1])->GetWeight())
						{
							cost[z][x][y+1] = cost[z][x][y] + g->FindEdge(index[z][x][y], index[z][x][y+1])->GetWeight();
							changes++;
						}
					}
				}
			}
		}

		if (changes == 0)
			break;
	}
	
	for (int z = 0; z < d; z++)
	{
		for (int x = 0; x < w; x++)
		{
			for (int y = 0; y < h; y++)
			{
				//printf("cost of node %d (%d, %d) is %d\n", x, y, index[x][y], cost[x][y]);
				node *n = g->GetNode(index[z][x][y]);
				if (cost[z][x][y] == 0)
					n->SetLabelF(GraphSearchConstants::kHCost, 0);
				else {
					//int val = ((random()%4) == 0)?(random()%(cost[z][x][y]+1)):0;
					int val = random()%(cost[z][x][y]+1);
					//printf("Assigning h = %d\n", val);
					n->SetLabelF(GraphSearchConstants::kHCost, (double)val);
				}
			}
		}
	}
	return g;
}

/**
 * This function is used to allocate the unit simulated that you want to run.
 * Any parameters or other experimental setup can be done at this time.
 */
void CreateSimulation(int id)
{
	//Graph *g;

	if (gDefaultMap[0] != 0)
	{
		mp = new Map(gDefaultMap);
		grp = GraphSearchConstants::GetGraph(mp);
		env = new GraphEnvironment(grp, new GraphMapInconsistentHeuristic(mp, grp));
		
		while(gFrom==gTo) {
			gFrom = grp->GetRandomNode()->GetNum();
			gTo = grp->GetRandomNode()->GetNum();
		}
	}
	else if (fig == 0)
	{
		grp = BuildInconsistentGraph(5, N, N);
//		gFrom = 5*N*N-1;
//		to = 0;
		env = new GraphEnvironment(grp, new GraphLabelHeuristic(grp, gTo));
		env->SetDirected(true);
	}
	else if (fig == 1) 
	{
		grp = PropUtil::graphGenerator::genFig1(N);
		gFrom = N;
		gTo = 0;
		env = new GraphEnvironment(grp, new GraphLabelHeuristic(grp, gTo));
	}
	else if(fig == 2)
	{
		grp = PropUtil::graphGenerator::genFig2(N);
		gFrom = N;
		gTo = 0;
		env = new GraphEnvironment(grp, new GraphLabelHeuristic(grp, gTo));
	}	
	else if (fig == 3){
		grp = PropUtil::graphGenerator::genFig3(N);
		gFrom = 0;
		gTo = 2*N - 1;
		env = new GraphEnvironment(grp, new GraphLabelHeuristic(grp, gTo));
	}
	else {
		grp = PropUtil::graphGenerator::genFig4(N);
		gFrom = 0;
		gTo = 2*N + 1;
		env = new GraphEnvironment(grp, new GraphLabelHeuristic(grp, gTo));
	}
	
	printf("Environment ready.\n");

	unitSims.resize(id+1);
	unitSims[id] = new GraphSimulation(env);
	unitSims[id]->SetStepType(kMinTime);
	SetNumPorts(id, 1);
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
	InstallKeyboardHandler(MyDisplayHandler, "Non-interactive mode", "Run all 5 algorithms using the same environment.", kNoModifier, 'k');
	InstallKeyboardHandler(MyDisplayHandler, "Step History", "If the simulation is paused, step forward .1 sec in history", kAnyModifier, '}');
	InstallKeyboardHandler(MyDisplayHandler, "Step History", "If the simulation is paused, step back .1 sec in history", kAnyModifier, '{');
	InstallKeyboardHandler(MyDisplayHandler, "Step Abs Type", "Increase abstraction type", kAnyModifier, ']');
	InstallKeyboardHandler(MyDisplayHandler, "Step Abs Type", "Decrease abstraction type", kAnyModifier, '[');

	InstallKeyboardHandler(MyPathfindingKeyHandler, "Mapbuilding Unit", "Deploy unit that paths to a target, building a map as it travels", kNoModifier, 'd');
	InstallKeyboardHandler(MyRandomUnitKeyHandler, "Add A* Unit", "Deploys a simple a* unit", kNoModifier, 'a');
	InstallKeyboardHandler(MyRandomUnitKeyHandler, "Add simple Unit", "Deploys a randomly moving unit", kShiftDown, 'a');
	InstallKeyboardHandler(MyRandomUnitKeyHandler, "Add simple Unit", "Deploys a right-hand-rule unit", kControlDown, 1);

	//InstallCommandLineHandler(MyCLHandler, "-map", "-map filename", "Selects the default map to be loaded.");
	InstallCommandLineHandler(MyCLHandler, "-scenario", "-scenario filename", "Runs a specified scenario file.");
	//InstallCommandLineHandler(MyCLHandler, "-numHeuristics", "-numHeuristics <N>", "Uses the max of N diff heuristics.");
	
	InstallWindowHandler(MyWindowHandler);

	InstallMouseClickHandler(MyClickHandler);
}

void MyWindowHandler(unsigned long windowID, tWindowEventType eType)
{
	if (eType == kWindowDestroyed)
	{
		printf("Window %ld destroyed\n", windowID);
		RemoveFrameHandler(MyFrameHandler, windowID, 0);
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
//		unitSims[windowID]->StepTime(1.0/30.0);
//		if ((!unitSims[windowID]->GetPaused()) && CFOR)
//			CFOR->DoOneSearchStep();
//
//		if (CFOR)
//		{
//			CFOR->OpenGLDraw();
//		}

		unitSims[windowID]->StepTime(1.0/30.0);
		//if((!unitSims[windowID]->GetPaused()) && ALG)
		//	ALG->DoSingleSearchStep(gPath);

		if(ALG)
		{
			ALG->OpenGLDraw();
		}
	}
	unitSims[windowID]->OpenGLDraw();
}

int MyCLHandler(char *argument[], int maxNumArgs)
{
	if (maxNumArgs <= 1)
		return 0;
//	if (strcmp(argument[0], "-numHeuristics") == 0)
//		numHeuristics = atoi(argument[1]);
	if (strcmp(argument[0], "-scenario") == 0)
		RunExperiments(new ScenarioLoader(argument[1]));
	return 2;
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
			if (ALG == 0) 
			{
				if (vid != 6)
					ALG = new Prop(vid,delta);
				else
					ALG = new AStarDelay();
				ALG->InitializeSearch(env,grp,gFrom,gTo,gPath);
			}

			if(!done && ALG->DoSingleSearchStep(gPath)) 
			{
				printf("\nDone! Nodes expanded=%lld, Nodes touched=%lld, Reopenings=%lld.\n",ALG->GetNodesExpanded(),ALG->GetNodesTouched(),ALG->GetNodesReopened());
				printf("Algorithm %s, solution cost=%lf, solution edges=%d.\n", ALG->GetName(),ALG->GetSolutionCost(),ALG->GetSolutionEdges());
				
				done = true;
			}



		}
			if (unitSims[windowID]->GetPaused())
			{
				unitSims[windowID]->SetPaused(false);
				unitSims[windowID]->StepTime(1.0/30.0);
				unitSims[windowID]->SetPaused(true);
			}
			break;
		case 'k':
			RunInconsistencyExperiment();
			break;
			ALG = new Prop(0);
			ALG->GetPath(env,grp,gFrom,gTo,gPath);
			printf("\n");

			ALG = new Prop(1);
			ALG->GetPath(env,grp,gFrom,gTo,gPath);
			printf("\n");

			ALG = new Prop(2,delta);
			ALG->GetPath(env,grp,gFrom,gTo,gPath);
			printf("\n");

//			ALG = new Prop(3);
//			ALG->GetPath(env,grp,gFrom,gTo,gPath);
//			printf("\n");

//			ALG = new Prop(4);
//			ALG->GetPath(env,grp,gFrom,gTo,gPath);
//			printf("\n");

//			ALG = new Prop(5);
//			ALG->GetPath(env,grp,gFrom,gTo,gPath);
//			printf("\n");

			ALG = new AStarDelay();
			ALG->GetPath(env,grp,gFrom,gTo,gPath);
			printf("Delay\t%lld\t%lld\t%lld\n",ALG->GetNodesExpanded(),ALG->GetNodesTouched(),ALG->GetNodesReopened());

//			ALG = new Prop(7);
//			ALG->GetPath(env,grp,gFrom,gTo,gPath);
//			printf("\n");
//
//			ALG = new Prop(8);
//			ALG->GetPath(env,grp,gFrom,gTo,gPath);
//			printf("\n");
//
//			ALG = new Prop(9);
//			ALG->GetPath(env,grp,gFrom,gTo,gPath);
//			printf("\n");

			break;
		case ']': absType = (absType+1)%3; break;
		case '[': absType = (absType+4)%3; break;
//		case '{': unitSim->setPaused(true); unitSim->offsetDisplayTime(-0.5); break;
//		case '}': unitSim->offsetDisplayTime(0.5); break;
		default:
			// set N to be whatever size I press
			//GraphSizeN = key-'0';
			//if (GraphSizeN == 0)
			//	GraphSizeN = 10;
			break;
	}
}

void MyRandomUnitKeyHandler(unsigned long , tKeyboardModifier , char)
{
}

void MyPathfindingKeyHandler(unsigned long , tKeyboardModifier , char)
{
//	while (1)
//	{
//		Timer t;
//		
//		t.StartTimer();
//		a3.GetPath(ma1, s1, g1, path);
//		t.EndTimer();
//		printf("new: %d nodes expanded. Path length %d / %f. Time: %f\n", 
//			   a3.GetNodesExpanded(), (int)path.size(), ma2->GetPathLength(path), t.GetElapsedTime());
//		
//		t.StartTimer();
//		a2.GetPath(ma1, s1, g1, path);
//		t.EndTimer();
//		printf("old: %d nodes expanded. Path length %d / %f. Time: %f\n", 
//			   a2.GetNodesExpanded(), (int)path.size(), ma2->GetPathLength(path), t.GetElapsedTime());
//	}

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

bool MyClickHandler(unsigned long , int, int, point3d , tButtonType , tMouseEventType )
{
	return false;
//	mouseTracking = false;
//	if (button == kRightButton)
//	{
//		switch (mType)
//		{
//			case kMouseDown:
//				unitSim->GetMap()->GetPointFromCoordinate(loc, px1, py1);
//				//printf("Mouse down at (%d, %d)\n", px1, py1);
//				break;
//			case kMouseDrag:
//				mouseTracking = true;
//				unitSim->GetMap()->GetPointFromCoordinate(loc, px2, py2);
//				//printf("Mouse tracking at (%d, %d)\n", px2, py2);
//				break;
//			case kMouseUp:
//			{
//				if ((px1 == -1) || (px2 == -1))
//					break;
//				unitSim->GetMap()->GetPointFromCoordinate(loc, px2, py2);
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

void RunInconsistencyExperiment()
{
	printf("Running experiment\n");
	ALG = new Prop(0);
	GraphAlgorithm *asd = new AStarDelay();

	for (int size = 50; size <= 50; size++)
	{
		for (int cost = 50; cost <= 10000; cost *= 2)
		{
			for (int count = 0; count < 100; count++)
			{
				fflush(stdout);
				Graph *g = BuildInconsistentGraph(size, size, size, 50, cost);
				env = new GraphEnvironment(g, new GraphLabelHeuristic(g, gTo));
				env->SetDirected(true);
				
				//((Prop*)ALG)->SetAlgorithm(PROP_A);
				((Prop*)ALG)->verID = PROP_A;
				ALG->GetPath(env,g,gFrom,gTo,gPath);
				printf("A\t%lld\t%lld\t%lld\t%d\t%d\n",
					   ALG->GetNodesExpanded(),ALG->GetNodesTouched(),ALG->GetNodesReopened(),
					   size, cost);
				
				//((Prop*)ALG)->SetAlgorithm(PROP_B);
				((Prop*)ALG)->verID = PROP_B;

				ALG->GetPath(env,g,gFrom,gTo,gPath);
				printf("B\t%lld\t%lld\t%lld\t%d\t%d\n",
					   ALG->GetNodesExpanded(),ALG->GetNodesTouched(),ALG->GetNodesReopened(),
					   size, cost);
				
				//((Prop*)ALG)->SetAlgorithm(PROP_BP);
				((Prop*)ALG)->verID = PROP_BP;
				ALG->GetPath(env,g,gFrom,gTo,gPath);
				printf("P\t%lld\t%lld\t%lld\t%d\t%d\n",
					   ALG->GetNodesExpanded(),ALG->GetNodesTouched(),ALG->GetNodesReopened(),
					   size, cost);
				
				asd->GetPath(env,g,gFrom,gTo,gPath);
				printf("D\t%lld\t%lld\t%lld\t%d\t%d\n",
					   asd->GetNodesExpanded(),asd->GetNodesTouched(),asd->GetNodesReopened(),
					   size, cost);

				delete env;
			}
		}
	}
	ALG = 0;
}

// experiments with 0, 10, ..., 100 differential heuristics
void RunExperiments1(ScenarioLoader *sl)
{
	std::vector<graphState> aPath;

	Map *m = new Map(sl->GetNthExperiment(0).GetMapName());
	m->Scale(sl->GetNthExperiment(0).GetXScale(), 
			 sl->GetNthExperiment(0).GetYScale());
	Graph *g = GraphSearchConstants::GetGraph(m);
	
	GraphDistanceHeuristic diffHeuristic(g);
	diffHeuristic.UseSmartPlacement(true);
	
	GraphEnvironment gEnv(g, &diffHeuristic);
	gEnv.SetDirected(true);
	
	TemplateAStar<graphState, graphMove, GraphEnvironment> taNew;

	
	for (int z = 0; z <= 10; z++)
	{
		for (int x = 0; x < sl->GetNumExperiments(); x++)
		{
			Experiment e = sl->GetNthExperiment(x);

			graphState start, goal;
			start = m->GetNodeNum(e.GetStartX(), e.GetStartY());
			goal = m->GetNodeNum(e.GetGoalX(), e.GetGoalY());

			Timer t;
			t.StartTimer();
			taNew.GetPath(&gEnv, start, goal, aPath);
			t.EndTimer();

			printf("%d\t%d\t%lld\t%f\n", e.GetBucket(), diffHeuristic.GetNumHeuristics(), taNew.GetNodesExpanded(), t.GetElapsedTime());
		}
		for (int x = 0; x < 10; x++)
			diffHeuristic.AddHeuristic();
	}
	exit(0);
}

// experiments with 0, 10, ..., 100 differential heuristics
// 10% of them are used at each step with and without BPMX
void RunExperiments(ScenarioLoader *sl)
{
	std::vector<graphState> aPath;
	
	Map *m = new Map(sl->GetNthExperiment(0).GetMapName());
	m->Scale(sl->GetNthExperiment(0).GetXScale(), 
			 sl->GetNthExperiment(0).GetYScale());
	Graph *g = GraphSearchConstants::GetGraph(m);
	
	GraphMapInconsistentHeuristic diffHeuristic(m, g);
	diffHeuristic.UseSmartPlacement(true);
	diffHeuristic.SetMode(kRandom);
	
	GraphEnvironment gEnv(g, &diffHeuristic);
	gEnv.SetDirected(true);
	
	TemplateAStar<graphState, graphMove, GraphEnvironment> taNew;

	Timer t;
	
	for (int z = 0; z < 1; z++)
	{
		for (int x = 0; x < 10; x++)
			diffHeuristic.AddHeuristic();
		diffHeuristic.SetNumUsedHeuristics(diffHeuristic.GetNumHeuristics()/10);

		for (int x = 0; x < sl->GetNumExperiments(); x++)
		{
			Experiment e = sl->GetNthExperiment(x);
//			if (e.GetBucket() != 127)
//			{ continue; }
			
			graphState start, goal;
			start = m->GetNodeNum(e.GetStartX(), e.GetStartY());
			goal = m->GetNodeNum(e.GetGoalX(), e.GetGoalY());
			
//			taNew.SetUseBPMX(false);
//			Timer t;
//			t.StartTimer();
//			taNew.GetPath(&gEnv, start, goal, aPath);
//			t.EndTimer();
//			
//			printf("%d\t%d.%d\t%d\t%f\t\t%f\n", e.GetBucket(), diffHeuristic.GetNumHeuristics(), diffHeuristic.GetNumHeuristics()/10,
//				   taNew.GetNodesExpanded(), t.GetElapsedTime(), gEnv.GetPathLength(aPath));

			for (int y = 1; y < 6; y++)
			{
				if (y == 5)
					taNew.SetUseBPMX(1000);
				else
					taNew.SetUseBPMX(y);
				t.StartTimer();
				taNew.GetPath(&gEnv, start, goal, aPath);
				t.EndTimer();
				
				printf("%d\t%d.%d\t%lld\t%f\tBPMX\t%f\n", e.GetBucket(), diffHeuristic.GetNumHeuristics(), y,
					   taNew.GetNodesExpanded(), t.GetElapsedTime(), gEnv.GetPathLength(aPath));
			}
		}
	}
	exit(0);
}
