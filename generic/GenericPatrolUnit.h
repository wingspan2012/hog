#ifndef GENERICPATROLUNIT_H
#define GENERICPATROLUNIT_H

#include <cstdlib> // for random number
#include "Unit.h"
#include "GenericSearchUnit.h"
#include "ReservationProvider.h"
//#include "SearchEnvironment.h"

template <class state, class action, class environment>
class GenericPatrolUnit : public GenericSearchUnit<state,action, environment> {
public:
	GenericPatrolUnit(state &s, GenericSearchAlgorithm<state,action,environment>* alg);
	GenericPatrolUnit(state &s, GenericSearchAlgorithm<state,action,environment>* alg, GLfloat _r, GLfloat _g, GLfloat _b);
	virtual const char *GetName() { return name; } 
	void SetName(char* myname) { strncpy(name, myname,128); }
	virtual void GetLocation(xyLoc& s) { s=loc;}
	virtual bool MakeMove(environment *env, OccupancyInterface<state,action> *, SimulationInfo *si, action &dir);
	
	virtual void OpenGLDraw(int window, environment *, SimulationInfo *);
	void AddPatrolLocation(state &s); 
	state& GetGoal(); // get CURRENT goal? 
	virtual bool Done(); 
	void UpdateLocation(environment *, state &l, bool success, SimulationInfo *si);
	void LogStats(StatCollection *stats);
	void LogFinalStats(StatCollection *stats);
	void SetNumPatrols(int num) {numPatrols = num;}
	
	/** Set wether we want to trim the planned path
	 * 
	 * @author Renee Jansen
	 * @date 10/2007
	 */
	void SetTrimPath(bool b) {trimPath = b;}
	
	/** Set the window at which we want to trim the planned path
	* 
	* If trimPath is set to true, this will determine where the path is
	* trimmed
	* 
	* @author Renee Jansen
	* @date 10/2007
	*/
	void SetTrimWindow(double d) {trimWindow = d;}
	
	/** Set whether we want to draw the unit. Default "true".
	*	
	* @author Renee Jansen
	* @date 10/30/2007
	*/
	
	void SetDrawUnit(bool b) {drawUnit = b;}
// 	void SetColor(GLfloat _r, GLfloat _g, GLfloat _b) {Unit::Set		GLFloat _r,_g,_b;
// 		GetColor(_r,_g,_b);
// 		glColor3f(_r,_g,_b);this->r=_r; this->g=_g; this->b=_b;}
	
private:
	//GLfloat r, g, b;
	xyLoc loc;
	int numPatrols;
	int counter;
	tDirection oldDir, oldDirColl;
	double totalDistance;
	double lastFailedMove;
	
	// this used to return path cost, but that wasn't used... 
	/*double*/ void GoToLoc(environment *env, int which);
 	void AddPathToCache(environment *env, std::vector<state> &path);
	std::vector<action> moves;
	std::vector<state> locs;

	GenericSearchAlgorithm<state,action,environment> *algorithm;
	
	int currTarget;
	int nodesExpanded;
	int nodesTouched;
	int numFailedMoves;
	int numDirectionChanges;
	int numDirectionChangesCollisions;
	
	char name[128];
	
	bool trimPath;
	double trimWindow;
	
	bool drawUnit; 
};

template <class state, class action, class environment>
GenericPatrolUnit<state,action,environment>::GenericPatrolUnit(state &s,GenericSearchAlgorithm<state,action,environment>* alg) 
:GenericSearchUnit<state,action,environment>(s,s,alg)
{
	lastFailedMove = -10;
	counter = 0; 
	numPatrols = -1; // infinitely patrol
	
	locs.push_back(s);
	loc = s;
	//setObjectType(kWorldObject);
	currTarget = -1;
	nodesExpanded = nodesTouched = numFailedMoves = numDirectionChanges = 0;
	numDirectionChangesCollisions = 0;
	algorithm = alg;
	
	GLfloat _r,_g,_b;
	_r = (double)rand() / RAND_MAX;
	_g = (double)rand() / RAND_MAX;
	_b = (double)rand() / RAND_MAX;
	this->SetColor(_r,_g,_b);
	strncpy(name, "GenericPatrolUnit",128);
	
	totalDistance = 0; 
	oldDir = oldDirColl = kStay;
	
	trimPath = false;
	trimWindow = 5; 
	drawUnit = true;
}

template <class state, class action, class environment>
GenericPatrolUnit<state,action,environment>::GenericPatrolUnit(state &s, GenericSearchAlgorithm<state,action,environment>* alg, GLfloat _r, GLfloat _g, GLfloat _b):GenericSearchUnit<state,action,environment>(s,s,alg)
{
	lastFailedMove = -10;
	counter = 0; 
	numPatrols = -1; // infinitely patrol
	
	locs.push_back(s);
	loc = s;

	currTarget = -1;
	nodesExpanded = nodesTouched = numFailedMoves = numDirectionChanges = 0;
	numDirectionChangesCollisions = 0;
	algorithm = alg;
	
	this->r = _r;
	this->g = _g;
	this->b = _b;
	
	strncpy(name, "GenericPatrolUnit",128);
	
	totalDistance = 0;
	oldDir = oldDirColl = kStay;
	
	trimPath = false;
	trimWindow = 5;
	drawUnit = true;
}

template <class state, class action, class environment>
bool GenericPatrolUnit<state,action,environment>::MakeMove(environment *env, OccupancyInterface<state,action> *oi, SimulationInfo *si, action& dir)
{
	if (moves.size() > 0)
	{
		//dir = moves.back();
		//moves.pop_back();
		dir = moves.front();
		moves.erase(moves.begin());
		return true;
	}
		
	if (currTarget != -1)
	{
		// if we're not yet at our current target, then we don't need to update 
		// current target
		if(loc == locs[currTarget])
		{
			currTarget = (currTarget+1)%locs.size();
			if((numPatrols != -1)&&(currTarget == 1))
			{	
				counter++;
			}
		}
 		
 		// If we're right beside our goal and it's blocked, make a random move
		else if(env->GetAction(locs[currTarget], loc) != kTeleport)
		{
			if(env->GetOccupancyInfo()->GetStateOccupied(locs[currTarget]))
			{
				srand(time(0));
				std::vector<tDirection> actions;
				env->GetActions(loc, actions);
				dir = actions[(rand())%(actions.size())];
				return true;
			}
		}
 		
 		if((numPatrols == -1 ) || (counter < numPatrols))
	 	{
	 		GoToLoc(env, currTarget);
	 	}
		else
		{
 			return false; 
		}
		
		if (moves.size() > 0)
		{
			dir = moves.front();
			moves.erase(moves.begin());
			return true;
		}
	}
	//Stay where you are
	return false;
}

template <class state, class action, class environment>
void GenericPatrolUnit<state,action,environment>::GoToLoc(environment *env, int which)
{
	std::vector<state> path; 

	algorithm->GetPath(env, loc, locs[which],path);
	
	nodesExpanded += algorithm->GetNodesExpanded();
	nodesTouched += algorithm->GetNodesTouched();
	if (path.size() > 0)
	{
		AddPathToCache(env, path);
	}
}

template <class state, class action, class environment>
void GenericPatrolUnit<state,action,environment>::AddPatrolLocation(state &s)
{
 currTarget = 1;
 locs.push_back(s);
}
 
template <class state, class action, class environment>
void GenericPatrolUnit<state,action,environment>::AddPathToCache(environment* env, std::vector<state> &p)
{
	for(unsigned int i=0; i<p.size()-1; i++)
	{
		if(trimPath && (env->HCost(p[i+1], loc) > trimWindow))
		break;
		moves.push_back(env->GetAction(p[i], p[i+1]));
	}
}

template <class state, class action, class environment>
void GenericPatrolUnit<state,action, environment>::OpenGLDraw(int window, environment *env, SimulationInfo *si)
{
	if(drawUnit)
	{
		GLfloat _r,_g,_b;
		this->GetColor(_r,_g,_b);
		if (si->GetSimulationTime() < lastFailedMove+2)
		{
			double interval = (si->GetSimulationTime()-lastFailedMove)/2;
			_r *= interval;
			_g *= interval;
			_b *= interval;
		}
		if(!Done())
			env->OpenGLDraw(window, loc,_r,_g,_b);	
		else
			env->OpenGLDraw(window, loc, 0,0,0);
	} 
		
	// Draw the patrol locations	
	for(unsigned int i=0; i<locs.size(); i++)
	{
		state l = locs[i];
		GLdouble xx, yy, zz, rad;
		env->GetMapAbstraction()->GetMap()->getOpenGLCoord(l.x, l.y, xx, yy, zz, rad);
		GLfloat _r,_g,_b;
		this->GetColor(_r,_g,_b);
		glColor3f(_r,_g,_b);
		DrawPyramid(xx, yy, zz, 1.1*rad, 0.75*rad);
		//		env->OpenGLDraw(window, locs[i], r, g, b);
	}	
	
	// Draw the planned path
	if(drawUnit)
	{
		xyLoc current = loc; 
		xyLoc next;
	  	for(unsigned int i=0; i<moves.size(); i++)
 		{
 			env->OpenGLDraw(window,current, moves[i],1.0,0,0); // draw in red
 			env->GetNextState(current, moves[i],next);
 			current = next;
 		}	
	}
}

template <class state, class action, class environment>
void GenericPatrolUnit<state,action,environment>::UpdateLocation(environment *env, state &l, bool success, SimulationInfo *si)
{ 
	if (!success)
		lastFailedMove = si->GetSimulationTime();
	// Occupancy interface stuff should be done in UnitSimulation
	//Update occupancy interface
	//env->GetOccupancyInterface()->SetStateOccupied(loc,false);
	//env->GetOccupancyInterface()->SetStateOccupied(l, true);
	
	//if(!success)
	
	if(!(l==loc))
	{
		tDirection dir = env->GetAction(loc,l);
		
		if(dir != oldDir)
		{
			numDirectionChanges++;
			oldDir = dir;
		}
		if(dir != oldDirColl)
		{
			numDirectionChangesCollisions++;
			oldDirColl = dir;
		}

		totalDistance += env->HCost(loc,l);
	}
	
	if((!success))//||(l == loc))
	{ 
		numFailedMoves++;
		moves.resize(0); 
		oldDirColl = kStay;
		//if(currTarget != -1) 
		//	currTarget = 0; 
	} 
	loc = l; 
	
}

template <class state, class action, class environment>
bool GenericPatrolUnit<state,action,environment>::Done()
{
	if(numPatrols == -1)
		return true;
	else if(counter >= numPatrols)
		return true;
	else
		return false;
}

template <class state, class action, class environment>
void GenericPatrolUnit<state,action, environment>::LogStats(StatCollection *sc)
{

}

template <class state, class action, class environment>
void GenericPatrolUnit<state,action,environment>::LogFinalStats(StatCollection *sc)
{
	//Want: 
	// * nodesExpanded
	// * distance travelled
	// * failedMoves
	// * change in direction
	
	sc->AddStat("nodesExpanded", GetName(), (long)(nodesExpanded));
	sc->AddStat("distanceTravelled", GetName(), totalDistance);
	sc->AddStat("directionChanges",GetName(), (long)(numDirectionChanges));
	sc->AddStat("directionChangesCollision",GetName(), (long)(numDirectionChangesCollisions));
	sc->AddStat("failedMoves",GetName(), (long)(numFailedMoves));
	
}

#endif
