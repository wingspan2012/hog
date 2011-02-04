/*
 *  RoboticArmTest.cpp
 *  hog2
 *
 *  Created by Nathan Sturtevant on 11/15/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
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
#include "UnitSimulation.h"
#include "EpisodicSimulation.h"
#include <deque>
#include "IDAStar.h"
#include "TemplateAStar.h"
#include "RoboticArmTest.h"
#include "FrontierBFS.h"

#include "RoboticArm.h"


//#define RUN_RANDOM_TESTS
//#define HEURISTIC_TABLES
//#define MAX_DIST_HEUR_TABLES
#define FIXED_RANDOM_NUMBER_SEED 7


const int numArms = 3;
RoboticArm *r = 0;
armAngles config;
armAngles goal;
bool validSearch = false;
unsigned int pathLoc = 0;
std::vector<armAngles> ourPath;
TemplateAStar<armAngles, armRotations, RoboticArm> astar;
float totalTime;
ArmToArmHeuristic *aa = 0;
void TestArms();
void TestArms2();
void BuildTipTables();

bool mouseTracking;
int px1, py1, px2, py2;
int absType = 0;

//std::vector<PuzzleSimulation *> unitSims;

int main(int argc, char* argv[])
{
	InstallHandlers();
	setlinebuf( stdout );
	RunHOGGUI(argc, argv);
}


/**
 * This function is used to allocate the unit simulated that you want to run.
 * Any parameters or other experimental setup can be done at this time.
 */
void CreateSimulation(int)
{
	if (r != 0)
		delete r;
	r = new RoboticArm(numArms, 1.0/(double)numArms);
//	r->AddObstacle(line2d(recVec(-1,  1, 0), recVec( 1,  1, 0)));
//	r->AddObstacle(line2d(recVec(-1, -1, 0), recVec( 1, -1, 0)));
//	r->AddObstacle(line2d(recVec( 1, -1, 0), recVec( 1,  1, 0)));
//	r->AddObstacle(line2d(recVec(-1, -1, 0), recVec(-1,  1, 0)));

	r->AddObstacle(line2d(recVec(-0.32, -0.30, 0), recVec(0.32, -0.30, 0)));
	////r->AddObstacle(line2d(recVec(-0.30, -0.28, 0), recVec(-0.30, -0.32, 0)));
	r->AddObstacle(line2d(recVec(0.30, 0.32, 0), recVec(0.30, -0.32, 0)));
	r->AddObstacle(line2d(recVec(0.28, 0.30, 0), recVec(0.32, 0.30, 0)));
	r->AddObstacle(line2d(recVec(-0.30, -0.32, 0), recVec(-0.30, 0.32, 0)));
	r->AddObstacle(line2d(recVec(-0.28, 0.30, 0), recVec(-0.32, 0.30, 0)));

//	r->AddObstacle(line2d(recVec(0, 0.50, 0), recVec(0.02, 0.52, 0)));
//	r->AddObstacle(line2d(recVec(0.02, 0.52, 0), recVec(0, 0.54, 0)));
//	r->AddObstacle(line2d(recVec(0, 0.54, 0), recVec(-0.02, 0.52, 0)));
//	r->AddObstacle(line2d(recVec(-0.02, 0.52, 0), recVec(0, 0.50, 0)));

#if 0
	r->AddObstacle(line2d(recVec(0.50, 0, 0), recVec(0.52, 0.02, 0)));
	r->AddObstacle(line2d(recVec(0.52, 0.02, 0), recVec(0.54, 0, 0)));
	r->AddObstacle(line2d(recVec(0.54, 0, 0), recVec(0.52, -0.02, 0)));
	r->AddObstacle(line2d(recVec(0.52, -0.02, 0), recVec(0.50, 0, 0)));
#endif

	config.SetNumArms( numArms );
//	for (int x = 0; x < numArms; x++)
//		config.SetAngle( x, 512 );
	//330, 2, 870
	//694, 1022, 154
	config.SetAngle(0, 694);
	config.SetAngle(1, 1022);
	config.SetAngle(2, 154);
	//r->GenerateTipPositionTables( config );

//	config.SetAngle( 0, 790 );
//	config.SetAngle( 1, 590 );
//	config.SetAngle( 2, 596 );
//	r->GenerateLegalStateTable( config );

//	do {
//	  for (int x = 0; x < numArms; x++)
//	    config.SetAngle(x, 512+random()%512);
//	} while( !r->LegalState( config ) );
//	unitSims.resize(id+1);
//	unitSims[id] = new PuzzleSimulation(new MNPuzzle(4, 4));
//	unitSims[id]->SetStepType(kMinTime);

//#ifdef FIXED_RANDOM_NUMBER_SEED
//	srandom( FIXED_RANDOM_NUMBER_SEED );
//#endif
//
//
//#ifdef HEURISTIC_TABLES
//#ifndef MAX_DIST_HEUR_TABLES
//	// some randomly generated goals
//	goal.SetGoal( 0.256288, -0.2005733 );
//	if( !r->GenerateHeuristic( config, goal ) ) {
//	  printf( "WARNING: INVALID GOAL FOR HEURISTIC\n" );
//	}
//	goal.SetGoal( -0.344304, 0.453280 );
//	if( !r->GenerateHeuristic( config, goal ) ) {
//	  printf( "WARNING: INVALID GOAL FOR HEURISTIC\n" );
//	}
//	goal.SetGoal( -0.119685, -0.437794 );
//	if( !r->GenerateHeuristic( config, goal ) ) {
//	  printf( "WARNING: INVALID GOAL FOR HEURISTIC\n" );
//	}
//	goal.SetGoal( -0.651559, 0.052971 );
//	if( !r->GenerateHeuristic( config, goal ) ) {
//	  printf( "WARNING: INVALID GOAL FOR HEURISTIC\n" );
//	}
//	goal.SetGoal( 0.608928, 0.593450 );
//	if( !r->GenerateHeuristic( config, goal ) ) {
//	  printf( "WARNING: INVALID GOAL FOR HEURISTIC\n" );
//	}
//#else
//#if 0
//	if( r->GenerateMaxDistHeuristics( config, 5 ) < 5 ) {
//	  printf( "WARNING: INVALID GOAL FOR HEURISTIC\n" );
//	}
//#else
//	// these goals were generated by GenerateMaxDistHeuristics
//	goal.SetGoal( .299558, -0.153535 );
//	if( !r->GenerateHeuristic( config, goal ) ) {
//	  printf( "WARNING: INVALID GOAL FOR HEURISTIC\n" );
//	}
// 	goal.SetGoal( -0.146228, -0.299603 );
// 	if( !r->GenerateHeuristic( config, goal ) ) {
// 	  printf( "WARNING: INVALID GOAL FOR HEURISTIC\n" );
// 	}
// 	goal.SetGoal( -0.091633, 0.901277 );
// 	if( !r->GenerateHeuristic( config, goal ) ) {
// 	  printf( "WARNING: INVALID GOAL FOR HEURISTIC\n" );
// 	}
// 	goal.SetGoal( 0.207202, 0.259286 );
// 	if( !r->GenerateHeuristic( config, goal ) ) {
// 	  printf( "WARNING: INVALID GOAL FOR HEURISTIC\n" );
// 	}
// 	goal.SetGoal( -0.209546, 0.252604 );
// 	if( !r->GenerateHeuristic( config, goal ) ) {
// 	  printf( "WARNING: INVALID GOAL FOR HEURISTIC\n" );
// 	}
//#endif
//#endif
//#endif
//
//// one test position, goal (0.53543, 0.280313)
//config.SetAngle( 0, 696 );
//config.SetAngle( 1, 784 );
//config.SetAngle( 2, 622 );
//
//// last arm added from max dist heuristics to generate goals
////config.SetAngle( 0, 330 );
////config.SetAngle( 1, 2 );
////config.SetAngle( 2, 872 );
////config.SetAngle( 0, 948 );
////config.SetAngle( 1, 1022 );
////config.SetAngle( 2, 2 );
////config.SetAngle( 0, 430 );
////config.SetAngle( 1, 682 );
////config.SetAngle( 2, 464 );
////config.SetAngle( 0, 404 );
////config.SetAngle( 1, 452 );
////config.SetAngle( 2, 2 );
////config.SetAngle( 0, 628 );
////config.SetAngle( 1, 914 );
////config.SetAngle( 2, 1020 );
//
//#ifdef RUN_RANDOM_TESTS
//	for( int i = 0; i < 1000; ++i ) {
//	  double x, y;
//
//	  do {
//	    for( int s = 0; s < numArms; s++ ) {
//	      config.SetAngle( s, random() & 0x3FE );
//	    }
//	  } while( !r->LegalState( config ) );
//
//	  
//	  do {
//	    x = (double)random() / (double)RAND_MAX * 2.0 - 1.0;
//	    y = (double)random() / (double)RAND_MAX * 2.0 - 1.0;
//	  } while( !r->ValidGoalPosition( x, y ) );
//
//	  goal.SetGoal( x, y );
//
//	  std::cout << "Starting search between: " << config << " and " << goal << std::endl;
//
// 	  Timer t;
// 	  t.StartTimer();
// 	  astar.GetPath( r, config, goal, ourPath );
// 	  if( ourPath.size() > 0 ) {
// 	    printf( "Done!" );
// 	  } else {
// 	    printf( "No solution!" );
// 	  }
// 	  printf( " %d nodes expanded; %1.1f nodes/sec\n",
// 		  astar.GetNodesExpanded(), 
//		  (double)astar.GetNodesExpanded() / t.EndTimer() );
//	}
//#endif
}

/**
 * Allows you to install any keyboard handlers needed for program interaction.
 */
void InstallHandlers()
{
	//InstallKeyboardHandler(MyDisplayHandler, "Toggle Abstraction", "Toggle display of the ith level of the abstraction", kAnyModifier, '0', '9');
	InstallKeyboardHandler(MyDisplayHandler, "Cycle Abs. Display", "Cycle which group abstraction is drawn", kAnyModifier, '\t');
	InstallKeyboardHandler(MyDisplayHandler, "Pause Simulation", "Pause simulation execution.", kNoModifier, 'p');
	InstallKeyboardHandler(MyDisplayHandler, "Step Simulation", "If the simulation is paused, step forward .1 sec.", kNoModifier, 'o');
	InstallKeyboardHandler(MyDisplayHandler, "Step History", "If the simulation is paused, step forward .1 sec in history", kAnyModifier, '}');
	InstallKeyboardHandler(MyDisplayHandler, "Step History", "If the simulation is paused, step back .1 sec in history", kAnyModifier, '{');
	InstallKeyboardHandler(MyDisplayHandler, "Step Abs Type", "Increase abstraction type", kAnyModifier, ']');
	InstallKeyboardHandler(MyDisplayHandler, "Step Abs Type", "Decrease abstraction type", kAnyModifier, '[');
	
	InstallKeyboardHandler(MyKeyHandler, "0-9", "select segment", kNoModifier, '0', '9');
	InstallKeyboardHandler(MyKeyHandler, "Rotate segment", "rotate segment CW", kNoModifier, 'a');
	InstallKeyboardHandler(MyKeyHandler, "Rotate segment", "rotate segment CCW", kNoModifier, 's');
	InstallKeyboardHandler(MyKeyHandler, "Build Heuristic", "Build differential heuristic", kNoModifier, 'b');
	InstallKeyboardHandler(MyKeyHandler, "Test Heuristic", "Build & test differential heuristic", kNoModifier, 't');
	
	InstallCommandLineHandler(MyCLHandler, "-map", "-map filename", "Selects the default map to be loaded.");
	
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
		SetNumPorts(windowID, 1);
	}
}

void MyFrameHandler(unsigned long , unsigned int , void *)
{
	static int currFrame = 0;
	currFrame++;

	r->OpenGLDraw();
	if (!validSearch)
	{
		if (ourPath.size() == 0)
			r->OpenGLDraw(config);
		else
			r->OpenGLDraw(ourPath[(pathLoc++)%ourPath.size()]);
	}
	else {
		Timer t;
		t.StartTimer();
		for (int x = 0; x < 1000; x++)
		{
			if (astar.DoSingleSearchStep(ourPath))
			{
				validSearch = false;
				if (ourPath.size() > 0)
				{
					totalTime += t.EndTimer();
					printf("Done! %lld nodes expanded; %1.1f nodes/sec\n",
						   astar.GetNodesExpanded(), (double)astar.GetNodesExpanded()/totalTime);
					config = ourPath.back();
					pathLoc = 0;
					break;
				}
			}
		}
		totalTime += t.EndTimer();
		if ((currFrame%500) == 499)
			printf("Currently generating %1.1f nodes/sec\n", (double)astar.GetNodesExpanded()/totalTime);
		armAngles next = astar.CheckNextNode();
		r->OpenGLDraw(next);
		r->OpenGLDraw(goal);
		//astar.GetPath(r, config, goal, ourPath);
	}
}

int MyCLHandler(char *argument[], int maxNumArgs)
{
	if (maxNumArgs <= 1)
		return 0;
	strncpy(gDefaultMap, argument[1], 1024);
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
		case 'p': //unitSims[windowID]->SetPaused(!unitSims[windowID]->GetPaused()); break;
		case 'o':
//			if (unitSims[windowID]->GetPaused())
//			{
//				unitSims[windowID]->SetPaused(false);
//				unitSims[windowID]->StepTime(1.0/30.0);
//				unitSims[windowID]->SetPaused(true);
//			}
			break;
		case ']': absType = (absType+1)%3; break;
		case '[': absType = (absType+4)%3; break;
			//		case '{': unitSim->setPaused(true); unitSim->offsetDisplayTime(-0.5); break;
			//		case '}': unitSim->offsetDisplayTime(0.5); break;
		default:
			break;
	}
}


void MyKeyHandler(unsigned long, tKeyboardModifier, char key)
{
	static int which = 0;
	if ((key >= '0') && (key <= '9'))
	{
		which = key-'0';
		return;
	}
	
	if (key == 'a')
	{
		std::vector<armRotations> actions;

		armRotations rot;
		rot.SetRotation(which, kRotateCW);
		r->GetActions(config, actions);
		for (unsigned int x = 0; x < actions.size(); x++)
			if (rot == actions[x])
			{
				r->ApplyAction(config, rot);
				ourPath.resize(0);
			}
	}
	if (key == 's')
	{
		std::vector<armRotations> actions;
		
		armRotations rot;
		rot.SetRotation(which, kRotateCCW);
		r->GetActions(config, actions);
		for (unsigned int x = 0; x < actions.size(); x++)
			if (rot == actions[x])
			{
				r->ApplyAction(config, rot);
				ourPath.resize(0);
			}
	}
	
	if (key == 't')
	{
		//BuildTipTables();
		//TestArms();
		TestArms2();
	}
	
	if (key == 'b')
	{
		if (aa == 0)
		{
			aa = new ArmToArmHeuristic(r, config);
			r->AddHeuristic(aa);
		}
		else
			aa->AddDiffTable();
	}
}

bool MyClickHandler(unsigned long , int x, int y, point3d loc, tButtonType whichButton, tMouseEventType mouseEvent)
{
	printf("Hit %d/%d (%f, %f)\n", x, y, loc.x, loc.y);
	if ((mouseEvent != kMouseDown) || (whichButton != kRightButton))
		return false;
	goal.SetGoal(loc.x, loc.y);
	if (aa)
	{
		const std::vector<armAngles> &pos = aa->GetTipPositions(loc.x, loc.y);
		if (pos.size() > 0)
		{
			goal = pos[0];
			validSearch = astar.InitializeSearch(r, goal, config, ourPath);
			for (unsigned int t = 1; t < pos.size(); t++)
			{
				goal = pos[t];
				astar.AddAdditionalStartState(goal);
			}
		}
	}
	else
		validSearch = astar.InitializeSearch(r, config, goal, ourPath);

	if (validSearch)
	{
		std::cout << "Starting search between: " << config << " and " << goal << std::endl;
		totalTime = 0;
	}
//	static point3d oldloc(0, 0, 0);
//
//	if (oldloc.x != 0)
//	{
//		r->AddObstacle(line2d(recVec(oldloc.x, oldloc.y, 0), recVec(loc.x, loc.y, 0)));
//		oldloc = loc;
//		return true;
//	}
//	oldloc = loc;
//	return false;

	return true;
}

#if 0
void AddToMinHeap( std::vector<armAngles> &heap, armAngles &arm,
		   float *distances )
{
	unsigned i;

	// need to increase size, although we don't actually care
	// about the new content yet
	heap.push_back( arm );
	for( i = heap.size(); i > 1; i >>= 1 ) {
		if( ArmDistance( arm, distances )
		    >= ArmDistance( heap[ ( i >> 1 ) - 1 ], distances ) ) {
			break;
		}
		heap[ i - 1 ] = heap[ ( i >> 1 ) - 1 ];
	}
	heap[ i - 1 ] = arm;
}

armAngles GetFromMinHeap( std::vector<armAngles> &heap, float *distances )
{
	unsigned c; // child index
	armAngles ret;

	ret = heap[ 1 - 1 ];

	// item heap.size() starts as the root (1) but it
	// must be pushed down if it is too large
	c = 1 << 1;
	while( c <= heap.size() - 1 ) {
		if( c + 1 <= heap.size() - 1
		    && ArmDistance( heap[ c - 1 ], distances )
		    > ArmDistance( heap[ c + 1 - 1 ], distances ) ) {
			// child c is bigger than child c+1, so use c+1
			++c;
		}
		if( ArmDistance( heap[ heap.size() - 1 ], distances )
		    <= ArmDistance( heap[ c - 1 ], distances ) ) {
			// new root <= than child, so it stops moving down
			break;
		}

		// root is pushed down past child, so child
		// item is now the parent
		heap[ ( c >> 1 ) - 1 ] = heap[ c - 1 ];

		// root has a new potential location, get new child
		c <<= 1;
	}

	// we found the child of our final location, put item
	// in its place
	heap[ ( c >> 1 ) - 1 ] = heap[ heap.size() - 1 ];
	heap.pop_back();

	return ret;
}
#endif

void TestArms()
{
	assert(aa == 0);
	aa = new ArmToArmHeuristic(r, config, true);
	r->AddHeuristic(aa);
	
	std::vector<armAngles> starts;
	std::vector<armAngles> goals;
	
	// first, get lots of good problems
	printf("Generating problems\n");
	while (starts.size() < 500)
	{
		if ((starts.size()%500) == 0)
			printf("Generating problem %d\n", (int)starts.size());
		double x, y;
		x = random()%10000;
		x = 2*x/10000-1;
		y = random()%10000;
		y = 2*y/10000-1;
		//printf("Trying config from (%f, %f) for start\n", x, y);
		const std::vector<armAngles> &pos = aa->GetTipPositions(x, y);
		if (pos.size() > 0)
		{
			starts.push_back(pos[random()%pos.size()]);
		}		
		else
			continue;

		while (1)
		{
			x = random()%10000;
			x = 2*x/10000-1;
			y = random()%10000;
			y = 2*y/10000-1;
			//printf("Trying (%f, %f) for goal\n", x, y);
			const std::vector<armAngles> &pos2 = aa->GetTipPositions(x, y);
			if (pos2.size() > 0)
			{
				armAngles theGoal;
				theGoal.SetGoal(x, y);
				goals.push_back(theGoal);
				break;
			}
		}
	}
	printf("Done generating problems\n");
	
//	if (aa == 0)
//	{
//		aa = new ArmToArmHeuristic(r, config);
//		r->AddHeuristic(aa);
//	}
//	else
//		aa->AddDiffTable();

	double fTotalTime;
	double totalNodes;
	double totalHvalue;
	
//	aa->AddDiffTable();
//	aa->AddDiffTable();
//	aa->AddDiffTable();
	for (int total = 0; total <= -1; total++)
	{
		printf("Solving with %d heuristics\n", total);
		fTotalTime = 0;
		totalNodes = 0;
		totalHvalue = 0;
		for (unsigned int x = 0; x < starts.size(); x++)
		{
			armAngles theGoal;
			double x1, y1;
			goals[x].GetGoal(x1, y1);
			const std::vector<armAngles> &pos = aa->GetTipPositions(x1, y1);
			theGoal = pos[0];
			double localHvalue = r->HCost(starts[x], theGoal);
			validSearch = astar.InitializeSearch(r, theGoal, starts[x], ourPath);
			for (unsigned int t = 1; t < pos.size(); t++)
			{
				theGoal = pos[t];
				astar.AddAdditionalStartState(theGoal);
				localHvalue = min(localHvalue, r->HCost(starts[x], theGoal));
			}
			
			Timer t;
			t.StartTimer();
//			while (!astar.DoSingleSearchStep(ourPath))
//			{}
			totalHvalue += localHvalue;
			fTotalTime += t.EndTimer();
			totalNodes += astar.GetNodesExpanded();
			printf("%d\t%lld\t%f\t%f\n", x, astar.GetNodesExpanded(), t.GetElapsedTime(), localHvalue);
		}
		fTotalTime /= starts.size();
		totalNodes /= starts.size();
		totalHvalue /= starts.size();
		printf("time\t%f\tnodes\t%f\thcost\t%f\n", fTotalTime, totalNodes, totalHvalue);
		aa->AddDiffTable();
	}

	printf("Solving with no heuristics the normal way\n");
	fTotalTime = 0;
	totalNodes = 0;
	for (unsigned int x = 0; x < starts.size(); x++)
	{
		validSearch = astar.InitializeSearch(r, starts[x], goals[x], ourPath);
		
		Timer t;
		t.StartTimer();
		while (!astar.DoSingleSearchStep(ourPath))
		{}
		fTotalTime += t.EndTimer();
		totalHvalue += r->HCost(starts[x], goals[x]);
		totalNodes += astar.GetNodesExpanded();
		printf("%d\t%lld\t%f\t%f\n", x, astar.GetNodesExpanded(), t.GetElapsedTime(), r->HCost(starts[x], goals[x]));
	}
	fTotalTime /= starts.size();
	totalNodes /= starts.size();
	totalHvalue /= starts.size();
	printf("time\t%f\tnodes\t%f\n", fTotalTime, totalNodes);
	exit(0);
}

void TestArms2()
{
	std::vector<armAngles> starts;
	std::vector<armAngles> goals;
	std::vector<armAngles> succ;

	for (int x = 0; x < 100; x++)
	{
		armAngles a;
		a.SetNumArms(numArms);
		for (int y = 0; y < numArms; y++)
			a.SetAngle( y, 512 );
		for (int y = 0; y < 50000; y++)
		{
			r->GetSuccessors(a, succ);
			a = succ[random()%succ.size()];
		}
		starts.push_back(a);
	}
	for (int x = 0; x < 100; x++)
	{
		armAngles a;
		a.SetNumArms(numArms);
		for (int y = 0; y < numArms; y++)
			a.SetAngle( y, 512 );
		for (int y = 0; y < 50000; y++)
		{
			r->GetSuccessors(a, succ);
			a = succ[random()%succ.size()];
		}
		goals.push_back(a);
	}
	ArmToArmCompressedHeuristic *a1 = new ArmToArmCompressedHeuristic(r, "3-arm_far1.diff");
	ArmToArmCompressedHeuristic *a2 = new ArmToArmCompressedHeuristic(r, "3-arm_far2.diff");
	r->AddHeuristic(a1);
	r->AddHeuristic(a2);
	astar.SetUseBPMX(1);
	printf("%d starts; %d goals\n", (int)starts.size(), (int)goals.size());
	for (unsigned int x = 0; x < starts.size(); x+=1)
	{
		config = starts[x];
		goal = goals[x];
		std::cout << "Searching " << starts[x] << " to " << goals[x] << std::endl;
		astar.InitializeSearch(r, starts[x], goals[x], ourPath);
		
		Timer t;
		t.StartTimer();
		while (!astar.DoSingleSearchStep(ourPath))
		{}
		t.EndTimer();
//		totalHvalue += r->HCost(starts[x], goals[x]);
//		totalNodes += astar.GetNodesExpanded();
		printf("%d\t%lld\t%lld\t%f\t%f\t%1.0f\n", x, astar.GetNodesExpanded(), astar.GetUniqueNodesExpanded(),
			   t.GetElapsedTime(), r->HCost(starts[x], goals[x]), r->GetPathLength(ourPath));
	}
	
}

void WriteCache(int index, std::vector<armAngles> &values);

void BuildTipTables()
{
	std::vector<int> reduction, offset1, offset2;
	reduction.push_back(3);reduction.push_back(2);reduction.push_back(2);
	offset1.push_back(0);offset1.push_back(0);offset1.push_back(0);
	offset2.push_back(0);offset2.push_back(0);offset2.push_back(1);
	ArmToArmCompressedHeuristic *aah = new ArmToArmCompressedHeuristic(r, reduction, offset1);
	ArmToArmCompressedHeuristic *aah1 = new ArmToArmCompressedHeuristic(r, reduction, offset2);
	Timer t;
	t.StartTimer();
	
	FrontierBFS<armAngles, armRotations> fbfs;
	printf("Performing frontier BFS!\n");
	std::cout << "Starting from " << config << std::endl;
//	std::vector<std::vector<armAngles> > cache;
	
	armAngles tmp = config;
	std::cout << "Adding heuristic from: " << tmp << std::endl;
	aah->BuildHeuristic(tmp);
	std::cout << "Adding heuristic from: " << tmp << std::endl;
	aah1->BuildHeuristic(tmp);
	std::cout << "Farthest state: " << tmp << std::endl;
	aah->BuildHeuristic(tmp);
	std::cout << "Farthest state: " << tmp << std::endl;
	aah->Save("3-arm_far2.diff");
	aah1->Save("3-arm_far1.diff");
	
	printf("%1.2f seconds elapsed\n", t.EndTimer());
	goal = config;
	for (int x = 0; x < numArms; x++)
		goal.SetAngle( x, 512 );
	config.SetAngle(0, 360);
	config.SetAngle(1, 70);
	config.SetAngle(2, 276);
//	config.SetAngle(0, 510);
//  goal.SetAngle(0, 510);
//	for (int x = 0; x < 1024; x+=2)
//	{
//		for (int y = 0; y < 1024; y+=2)
//		{
//			goal.SetAngle(1, x);
//			goal.SetAngle(2, y);
//			goal.SetAngle(0, 512);
//			std::cout << "1] " << goal << " to " << config << ": " << aah.HCost(goal, config) << std::endl;
//			goal.SetAngle(0, 510);
//			std::cout << "2] " << goal << " to " << config << ": " << aah1.HCost(goal, config) << std::endl;
//		}
//	}
	r->AddHeuristic(aah);
	r->AddHeuristic(aah1);
	validSearch = astar.InitializeSearch(r, config, goal, ourPath);
	astar.SetUseBPMX(1);
}

void WriteCache(int index, std::vector<armAngles> &values)
{
	return;
	char filename[255];
	sprintf(filename, "%d.tipIndex", index);
	FILE *f = fopen(filename, "a+");
	if (!f) assert(!"Couldn't open file!");
	for (unsigned int x = 0; x < values.size(); x++)
	{
		for (unsigned int y = 0; y < values[x].GetNumArms(); y++)
			fprintf(f, "%d ", values[x].GetAngle(y));
		fprintf(f, "\n");
	}
	fclose(f);
	values.resize(0);
}
