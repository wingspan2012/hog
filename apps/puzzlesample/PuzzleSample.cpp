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
#include "PuzzleSample.h"
#include "UnitSimulation.h"
#include "EpisodicSimulation.h"
#include "Plot2D.h"
#include "RandomUnit.h"
#include "MNPuzzle.h"
#include "FlipSide.h"
#include "IDAStar.h"

static FlipSideState fss(5);
static FlipSide fs(5);

bool mouseTracking;
int px1, py1, px2, py2;
int absType = 0;

std::vector<PuzzleSimulation *> unitSims;
//unit *cameraTarget = 0;

Plotting::Plot2D *plot = 0;
Plotting::Line *distLine = 0;

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
		map = new Map(60, 60);
	else
		map = new Map(gDefaultMap);

	unitSims.resize(id+1);
	unitSims[id] = new PuzzleSimulation(new MNPuzzle(5, 5));
	unitSims[id]->SetStepType(kMinTime);
}

/**
 * Allows you to install any keyboard handlers needed for program interaction.
 */
void InstallHandlers()
{
	InstallKeyboardHandler(MyDisplayHandler, "Toggle Abstraction", "Toggle display of the ith level of the abstraction", kAnyModifier, '0', '9');
	InstallKeyboardHandler(MyDisplayHandler, "Cycle Abs. Display", "Cycle which group abstraction is drawn", kAnyModifier, '\t');
	InstallKeyboardHandler(MyDisplayHandler, "Pause Simulation", "Pause simulation execution.", kNoModifier, 'p');
	InstallKeyboardHandler(MyDisplayHandler, "Step Simulation", "If the simulation is paused, step forward .1 sec.", kAnyModifier, 'o');
	InstallKeyboardHandler(MyDisplayHandler, "Step History", "If the simulation is paused, step forward .1 sec in history", kAnyModifier, '}');
	InstallKeyboardHandler(MyDisplayHandler, "Step History", "If the simulation is paused, step back .1 sec in history", kAnyModifier, '{');
	InstallKeyboardHandler(MyDisplayHandler, "Step Abs Type", "Increase abstraction type", kAnyModifier, ']');
	InstallKeyboardHandler(MyDisplayHandler, "Step Abs Type", "Decrease abstraction type", kAnyModifier, '[');

	InstallKeyboardHandler(MyPathfindingKeyHandler, "Mapbuilding Unit", "Deploy unit that paths to a target, building a map as it travels", kNoModifier, 'd');
	InstallKeyboardHandler(MyRandomUnitKeyHandler, "Add A* Unit", "Deploys a simple a* unit", kNoModifier, 'a');
	InstallKeyboardHandler(MyRandomUnitKeyHandler, "Add simple Unit", "Deploys a randomly moving unit", kShiftDown, 'a');
	InstallKeyboardHandler(MyRandomUnitKeyHandler, "Add simple Unit", "Deploys a right-hand-rule unit", kControlDown, 1);

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
	}
}

void MyFrameHandler(unsigned long windowID, unsigned int viewport, void *data)
{
		if ((windowID < unitSims.size()) && (unitSims[windowID] == 0))
			return;

	if (viewport == 0)
	{
		unitSims[windowID]->StepTime(1.0/30.0);
	}
	if (viewport == 3)
	{
		fs.OpenGLDraw(windowID, fss);
		return;
	}
	if (unitSims[windowID]->GetUnit(viewport))
	{
		printf("Drawing unit %d\n", viewport);
		MNPuzzleState s;
		unitSims[windowID]->GetUnit(viewport)->GetLocation(s);
		MNPuzzleState g(s.width, s.height);
		printf("Distance: %f\n", unitSims[windowID]->GetEnvironment()->HCost(s, g));
		unitSims[windowID]->GetUnit(viewport)->OpenGLDraw(windowID,
																											unitSims[windowID]->GetEnvironment(),
																											unitSims[windowID]);
	}
	//unitSims[windowID]->OpenGLDraw(windowID);
	
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
		case 'p': unitSims[windowID]->SetPaused(!unitSims[windowID]->GetPaused()); break;
		case 'o':
		{
			if (mod == kShiftDown)
			{
				IDAStar<FlipSideState, flipMove> ida;
				FlipSideState fg(5);
//				fg.puzzle[0] = 5;
//				fg.puzzle[1] = 6;
//				fg.puzzle[2] = 3;
//				fg.puzzle[3] = 2;
//				fg.puzzle[4] = 4;
//				fg.puzzle[5] = 0;
//				fg.puzzle[6] = 1;
//				fg.puzzle[7] = 7;
				std::vector<FlipSideState> path;
				ida.GetPath(&fs, fss, fg, path);
				for (int x = 0; x < path.size(); x++)
					std::cout << path[x] << std::endl;
			}
			else {
				std::vector<flipMove> acts;
				fs.GetActions(fss, acts);
				fs.ApplyAction(fss, acts[random()%acts.size()]);
				FlipSideState fg(5);
				printf("Distance: %f\n", fs.HCost(fss, fg));
			}
			
			if (unitSims[windowID]->GetPaused())
			{
				unitSims[windowID]->SetPaused(false);
				unitSims[windowID]->StepTime(1.0/30.0);
				unitSims[windowID]->SetPaused(true);
			}
		}
			break;
		case ']': absType = (absType+1)%3; break;
		case '[': absType = (absType+4)%3; break;
//		case '{': unitSim->setPaused(true); unitSim->offsetDisplayTime(-0.5); break;
//		case '}': unitSim->offsetDisplayTime(0.5); break;
		default:
			//if (unitSim)
			//	unitSim->GetEnvironment()->GetMapAbstraction()->ToggleDrawAbstraction(((mod == kControlDown)?10:0)+(key-'0'));
			break;
	}
}

void MyRandomUnitKeyHandler(unsigned long windowID, tKeyboardModifier mod, char)
{
	unitSims[windowID]->GetEnvironment();
	
//	SearchUnit *su1 = new SearchUnit(random()%m->getMapWidth(), random()%m->getMapHeight(), 0, 0);
//	SearchUnit *su2 = new SearchUnit(random()%m->getMapWidth(), random()%m->getMapHeight(), su1, new praStar());
//	//unitSim->AddUnit(su1);
	unitSims[windowID]->AddUnit(new RandomUnit<MNPuzzleState, slideDir, MNPuzzle>(MNPuzzleState(5, 5)));
}

void MyPathfindingKeyHandler(unsigned long windowID, tKeyboardModifier mod, char)
{
}

bool MyClickHandler(unsigned long windowID, int, int, point3d loc, tButtonType button, tMouseEventType mType)
{
	return false;
}

