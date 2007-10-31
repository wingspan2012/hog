/**
* @file templateAStar.h
* @package hog2
* @brief A templated version of the original HOG's genericAstar.h
* @author Nathan Sturtevant
* @author Modified by Renee Jansen to work with templates for HOG2's 
* SearchEnvironment
* @date 3/22/06, modified 06/13/2007
*
* This file is part of HOG2.
* HOG : http://www.cs.ualberta.ca/~nathanst/hog.html
* HOG2: http://code.google.com/p/hog2/
*
* HOG2 is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
* 
* HOG2 is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with HOG2; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef TEMPLATEASTAR_H
#define TEMPLATEASTAR_H

#define __STDC_CONSTANT_MACROS
#include <stdint.h>
// this is defined in stdint.h, but it doesn't always get defined correctly
// even when __STDC_CONSTANT_MACROS is defined before including stdint.h
// because stdint might be included elsewhere first...
#ifndef UINT32_MAX
#define UINT32_MAX        4294967295U
#endif

#include "FPUtil.h"
#include <ext/hash_map>
#include "OpenClosedList.h"
//#include "SearchEnvironment.h" // for the SearchEnvironment class
#include "float.h"

#include <algorithm> // for vector reverse

#include "GenericSearchAlgorithm.h"

//int discardcount;
namespace TemplateAStarUtil
{
	/**
	* A search node class to use with hash maps
	*/
	template <class state>
	class SearchNode {
public:
		SearchNode(state &curr, state &prev, double _fCost, double _gCost, uint64_t key)
		:fCost(_fCost), gCost(_gCost), currNode(curr), prevNode(prev), hashKey(key){}

		SearchNode(state &curr,uint64_t key)
		:fCost(0), gCost(0), currNode(curr), prevNode(curr), hashKey(key) {}

		// This probably needs to be fixed. Problem is you can't set the states to 
		// 0 - so how to initialize?
		SearchNode()
		:fCost(0), gCost(0), hashKey(0) {}

		double fCost;
		double gCost;
		state currNode;
		state prevNode;
		uint64_t hashKey;
	};
		
	template <class state>
	struct SearchNodeEqual {
		bool operator()(const SearchNode<state> &i1, const SearchNode<state> &i2)
		{ return (i1.currNode == i2.currNode); } };
	
	template <class state>
	struct SearchNodeCompare {
		bool operator()(const SearchNode<state> &i1, const SearchNode<state> &i2)
		{
			if (fequal(i1.fCost, i2.fCost))
			{
				return (fless(i1.gCost, i2.gCost));
			}
			return (fgreater(i1.fCost, i2.fCost));
		} };
		
	template <class state>
	struct SearchNodeHash {
		size_t operator()(const SearchNode<state> &x) const
		{ return (size_t)(x.hashKey); }
	};
}




/**
* A templated version of A*, based on HOG genericAStar
*/
template <class state, class action, class environment>
class TemplateAStar : public GenericSearchAlgorithm<state,action,environment> {
public:
	TemplateAStar() { radius = 4.0; stopAfterGoal = true; weight=1; }
	virtual ~TemplateAStar() {}
	void GetPath(environment *env, state& from, state& to, std::vector<state> &thePath);

	void GetPath(environment *env, state& from, state& to, std::vector<action> & thePath) {};
	
		typedef OpenClosedList<TemplateAStarUtil::SearchNode<state>, TemplateAStarUtil::SearchNodeHash<state>,
		TemplateAStarUtil::SearchNodeEqual<state>, TemplateAStarUtil::SearchNodeCompare<state> > PQueue;
		typedef __gnu_cxx::hash_map<uint64_t, TemplateAStarUtil::SearchNode<state>, Hash64 > NodeLookupTable;
	
	typedef __gnu_cxx::hash_map<uint64_t, bool,Hash64 > Corridor;
	
	PQueue openQueue;
	NodeLookupTable closedList; //openList
	state goal, start;
	
	typedef typename NodeLookupTable::const_iterator closedList_iterator;

	bool InitializeSearch(environment *env, state& from, state& to, std::vector<state> &thePath);
	bool DoSingleSearchStep(std::vector<state> &thePath);
	state CheckNextNode();
	void ExtractPathToStart(state& n, std::vector<state> &thePath);

	virtual const char *GetName();
	
	void PrintStats();
	//long GetNodesExpanded() { return nodesExpanded; }
	//long GetNodesTouched() { return nodesTouched; }
	void ResetNodeCount() { nodesExpanded = nodesTouched = 0; }
	int GetMemoryUsage();

	//closedList_iterator GetClosedListIter() const;
	void GetClosedListIter(closedList_iterator);
	bool ClosedListIterNext(closedList_iterator& it, state& next) const;
	bool GetClosedListGCost(state &val, double &gCost) const;
		
  //state ClosedListIterNext(closedList_iterator&) const;
	int GetNodesExpanded() { return nodesExpanded; }
	int GetNodesTouched() { return nodesTouched; }
	
	void LogFinalStats(StatCollection *stats) {}
	
	void SetRadius(double rad) {radius = rad;}
	double GetRadius() { return radius; }

	void SetStopAfterGoal(bool val) { stopAfterGoal = val; }
	bool GetStopAfterGoal() { return stopAfterGoal; }
	
	/** Use weighted A* and set the weight
	*
	* Use f = g + weight * h during search
	*
	* @author Renee Jansen
	* @date 10/23/2007
	*/
	void SetWeight (double w){weight = w;}
private:
	long nodesTouched, nodesExpanded;
	bool GetNextNode(state &next);
	//state GetNextNode();
	void UpdateWeight(environment *env, state& currOpenNode, state& neighbor);
	void AddToOpenList(environment *env, state& currOpenNode, state& neighbor);
	
	std::vector<state> neighbors;
	environment *env;
	Corridor eligibleNodes;
	bool stopAfterGoal;

	double radius; // how far around do we consider other agents?
	double weight; 
};

using namespace TemplateAStarUtil;
static const bool verbose = false;

/**
* Return the name of the algorithm. 
* @author Nathan Sturtevant
* @date 03/22/06
*
* @return The name of the algorithm
*/

template <class state, class action, class environment>
const char *TemplateAStar<state,action,environment>::GetName()
{
	static char name[32];
	sprintf(name, "TemplateAStar[]");
	return name;
}

/**
* Perform an A* search between two states.  
* @author Nathan Sturtevant
* @date 03/22/06
*
* @param _env The search environment
* @param from The start state
* @param to The goal state
* @param thePath A vector of states which will contain an optimal path 
* between from and to when the function returns, if one exists. 
*/
template <class state, class action, class environment>
void TemplateAStar<state,action,environment>::GetPath(environment *_env, state& from, state& to, std::vector<state> &thePath)
{
	//std::cout<<"\n\ndoing search from "<<from<<" to "<<to<<std::endl;
	//discardcount=0;
  	if (!InitializeSearch(_env, from, to, thePath))
  		return;
  	while (!DoSingleSearchStep(thePath)) {}
  	//std::cout<<"numexpanded "<<nodesExpanded<<std::endl;
  //	std::cout<<"numdiscarded "<<discardcount<<std::endl;
}

/**
* Initialize the A* search 
* @author Nathan Sturtevant	
* @date 03/22/06
* 
* @param _env The search environment
* @param from The start state
* @param to The goal state
* @return TRUE if initialization was successful, FALSE otherwise
*/
template <class state, class action, class environment>
bool TemplateAStar<state,action,environment>::InitializeSearch(environment *_env, state& from, state& to, std::vector<state> &thePath)
{
	env = _env;
	closedList.clear();
	openQueue.reset();
	assert(openQueue.size() == 0);
	assert(closedList.size() == 0);
	nodesTouched = nodesExpanded = 0;
	start = from;
	goal = to;
	
	if (from == to) //assumes that from and to are valid states
	{
		//std::cout<<"from is to\n";
		thePath.resize(0);
		return false;
	}

	//SearchNode<state> first(env->heuristic(goal, start), 0, start, start);
	SearchNode<state> first(start, start, weight*env->HCost(goal, start), 0,env->GetStateHash(start));
	openQueue.Add(first);

	return true;
}

/**
* Expand a single node. 
* @author Nathan Sturtevant
* @date 03/22/06
* 
* @param thePath will contain an optimal path from start to goal if the 
* function returns TRUE
* @return TRUE if there is no path or if we have found the goal, FALSE
* otherwise
*/
template <class state, class action, class environment>
bool TemplateAStar<state,action,environment>::DoSingleSearchStep(std::vector<state> &thePath)
{
	
	state currentOpenNode; 

	if (openQueue.size() == 0)
	{
		//std::cout<<"open queue empty\n";
		thePath.resize(0); // no path found!
		closedList.clear();
		return true;
	}
			
	// get top of queue
	if(!GetNextNode(currentOpenNode))
	{
		printf("Oh no! No more open nodes!\n");
	}
	
	
	if ((stopAfterGoal) && (env->GoalTest(currentOpenNode, goal)))
	{
		ExtractPathToStart(currentOpenNode, thePath);
		// Path is backwards - reverse
		reverse(thePath.begin(), thePath.end()); 
//		closedList.clear();
//		openQueue.reset();
//		env = 0;
		return true;
	}
	//		state wantFrom, wantTo;
	//	wantFrom.x = 21; wantFrom.y = 28;
	//	wantTo.x = 19; wantTo.y = 27;
		
	//if((start == wantFrom) && (goal == wantTo))std::cout<<"\ncurrent node "<<currentOpenNode<<std::endl;
	
	neighbors.resize(0);
	env->GetSuccessors(currentOpenNode, neighbors);

	// iterate over all the children
	for (unsigned int x = 0; x < neighbors.size(); x++)
	{
		nodesTouched++;
		state neighbor = neighbors[x];
		

		
		//if((start == wantFrom) && (goal == wantTo))std::cout<<"neighbor "<<neighbor<<std::endl;
		if (closedList.find(env->GetStateHash(neighbor)) != closedList.end())
		{
			//if((start == wantFrom) && (goal == wantTo))std::cout<<"on closed list already "<<neighbor<<std::endl;
			continue;
		}
		else if (openQueue.IsIn(SearchNode<state>(neighbor, env->GetStateHash(neighbor))))
		{
			//if((start == wantFrom) && (goal == wantTo))std::cout<<"updating "<<neighbor<<std::endl;
			UpdateWeight(env,currentOpenNode, neighbor);
		}
		else if ((env->HCost(start, neighbor) < radius) &&(env->GetOccupancyInfo()->GetStateOccupied(neighbor)) && !(env->GoalTest(neighbor, goal)))
					 //&& ((closedList[env->GetStateHash(currentOpenNode)].gCost+env->GCost(currentOpenNode,neighbor)) < 4))
		{
			//discardcount++;
			//if((start == wantFrom) && (goal == wantTo))std::cout<<"not expanding "<<neighbor<<std::endl;
			//occupied - don't expand - move to the closed list 
			SearchNode<state> sn(neighbor, env->GetStateHash(neighbor));
			closedList[env->GetStateHash(neighbor)] = sn;
		}
		else {
			//if((start == wantFrom) && (goal == wantTo))std::cout<<"Adding "<<neighbor<<" to open list\n";
			AddToOpenList(env, currentOpenNode, neighbor);
		}
	}
	return false;
}

/**
* Returns the next state on the open list (but doesn't pop it off the queue). 
* @author Nathan Sturtevant
* @date 03/22/06
* 
* @return The first state in the open list. 
*/
template <class state, class action, class environment>
state TemplateAStar<state, action,environment>::CheckNextNode()
{
	return openQueue.top().currNode;
}

/**
* Removes the top node from the open list  
* @author Nathan Sturtevant
* @date 03/22/06
*
* @param state will contain the next state in the open list
* @return TRUE if next contains a valid state, FALSE if there is no  more states in the
* open queue 
*/
template <class state, class action, class environment>
bool TemplateAStar<state,action,environment>::GetNextNode(state &next)
{
	nodesExpanded++;
	if(openQueue.Empty())
		return false;
	SearchNode<state> it = openQueue.Remove();
	//if(it == openQueue.end())
	//	return false;
	next = it.currNode;
	closedList[env->GetStateHash(next)] = it;
	return true;
}

/**
* Update the weight of a node. 
* @author Nathan Sturtevant
* @date 03/22/06
* 
* @param currOpenNode The node that's currently being expanded
* @param neighbor The node whose weight will be updated
*/
template <class state, class action, class environment>
void TemplateAStar<state,action,environment>::UpdateWeight(environment *env, state &currOpenNode, state &neighbor)
{
	SearchNode<state> prev = openQueue.find(SearchNode<state>(neighbor, env->GetStateHash(neighbor)));
	SearchNode<state> alt = closedList[env->GetStateHash(currOpenNode)];
	double edgeWeight = env->GCost(currOpenNode, neighbor);
	double altCost = alt.gCost+edgeWeight+(prev.fCost-prev.gCost);
	if (fgreater(prev.fCost, altCost))
	{
		prev.fCost = altCost;
		prev.gCost = alt.gCost+edgeWeight;
		prev.prevNode = currOpenNode;
		openQueue.DecreaseKey(prev);
	}
}

/**
* Add a node to the open list
* @author Nathan Sturtevant
* @date 03/22/06
* 
* @param currOpenNode the state that's currently being expanded
* @param neighbor the state to be added to the open list
*/
template <class state, class action,class environment>
void TemplateAStar<state, action,environment>::AddToOpenList(environment *env, state &currOpenNode, state &neighbor)
{
	double edgeWeight = env->GCost(currOpenNode, neighbor);
	SearchNode<state> n(neighbor, currOpenNode, closedList[env->GetStateHash(currOpenNode)].gCost+edgeWeight+weight*env->HCost(neighbor, goal),
							 closedList[env->GetStateHash(currOpenNode)].gCost+edgeWeight, env->GetStateHash(neighbor));
	
	openQueue.Add(n);
	
}

/**
* Get the path from a goal state to the start state 
* @author Nathan Sturtevant
* @date 03/22/06
* 
* @param goalNode the goal state
* @param thePath will contain the path from goalNode to the start state
*/
template <class state, class action,class environment>
void TemplateAStar<state, action,environment>::ExtractPathToStart(state &goalNode,
																			std::vector<state> &thePath)
{
	SearchNode<state> n;
	if (closedList.find(env->GetStateHash(goalNode)) != closedList.end())
	{
		n = closedList[env->GetStateHash(goalNode)];
	}
	else n = openQueue.find(SearchNode<state>(goalNode, env->GetStateHash(goalNode)));

	do {
		thePath.push_back(n.currNode);
		n = closedList[env->GetStateHash(n.prevNode)];
	} while (!(n.currNode == n.prevNode));
	thePath.push_back(n.currNode);
}

/**
* A function that prints the number of states in the closed list and open
* queue. 
* @author Nathan Sturtevant
* @date 03/22/06
*/
template <class state, class action, class environment>
void TemplateAStar<state, action,environment>::PrintStats()
{
	printf("%u items in closed list\n", (unsigned int)closedList.size());
	printf("%u items in open queue\n", (unsigned int)openQueue.size());
}

/**
* Return the amount of memory used by TemplateAstar
* @author Nathan Sturtevant
* @date 03/22/06
* 
* @return The combined number of elements in the closed list and open queue
*/
template <class state, class action, class environment>
int TemplateAStar<state, action,environment>::GetMemoryUsage()
{
	return closedList.size()+openQueue.size();
}

/**
* Get an iterator for the closed list
* @author Nathan Sturtevant
* @date 06/13/07
* 
* @return An iterator pointing to the first node in the closed list
*/
template <class state, class action,class environment>
//__gnu_cxx::hash_map<state, TemplateAStarUtil::SearchNode<state> >::const_iterator
void TemplateAStar<state, action,environment>::GetClosedListIter(closedList_iterator) //const
{
	return closedList.begin();
}

/**
* Get the next state in the closed list
* @author Nathan Sturtevant
* @date 06/13/07
* 
* @param it A closedList_iterator pointing at the current state in the closed 
* list
* @return The next state in the closed list. Returns UINT_MAX if there's no 
* more states
*/
template <class state, class action, class environment>
bool TemplateAStar<state, action,environment>::ClosedListIterNext(closedList_iterator& it, state& next) const
{
	if (it == closedList.end())
		return false;
	next = (*it).first;
	it++;
	return true;
}


/**
* Get state from the closed list
 * @author Nathan Sturtevant
 * @date 10/09/07
 * 
 * @param val The state to lookup in the closed list
 * @gCost The g-cost of the node in the closed list
 * @return success Whether we found the value or not
 * more states
 */
template <class state, class action, class environment>
bool TemplateAStar<state, action,environment>::GetClosedListGCost(state &val, double &gCost) const
{
	if (closedList.find(val) != closedList.end())
	{
		gCost = closedList[val].gCost;
		return true;
	}
	return false;
}

#endif
