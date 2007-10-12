/*
 *  IRAStar.cpp
 *  hog2
 *
 *  Created by Nathan Sturtevant on 10/12/07.
 *  Copyright 2007 Nathan Sturtevant, University of Alberta. All rights reserved.
 *
 */

#include "IRAStar.h"
#include <vector>

using namespace IRAStarConstants;

IRAStar::IRAStar()
:SearchAlgorithm()
{
	g = 0;
}

IRAStar::~IRAStar()
{
}

const char *IRAStar::GetName()
{
	return "IRAStar";
}

path *IRAStar::GetPath(GraphAbstraction *aMap, node *from, node *to, reservationProvider *)
{
	if (!InitializeSearch(aMap, from, to))
		return 0;
	path *p = 0;
	while (p == 0)
	{
		p = DoOneSearchStep();
		//g->Print(std::cout);
	}
	return p;
}

path *IRAStar::DoOneSearchStep()
{
	if (q.size() == 0)
		return 0;
	node *gNext = q.top().n;
	q.pop();
	//printf("---\nAnalyzing %d next g:%f h:%f\n", gNext->GetNum(), GetGCost(gNext), GetHCost(gNext));
	//std::cout << *gNext << std::endl;

	
	if (gNext == gGoal) // we found the goal
	{
		if ((q.size() > 0) &&
				(fequal(GetFCost(q.top().n), GetFCost(gGoal))))
		{
			node *temp = gNext;
			gNext = q.top().n;
			q.pop();
			q.Add(GNode(temp));
			//printf("+++\nAnalyzing %d next g:%f h:%f\n", gNext->GetNum(), GetGCost(gNext), GetHCost(gNext));
		}
		else {
			SetCachedHCost(gNext, GetGCost(gNext));
			closedList[gNext->GetNum()] = GNode(gNext);
			path *p = ExtractAndRefinePath();
			if (p)
			{
				q.reset();
				closedList.clear();
			}
			return p;
		}
	}

	nodesExpanded++;
	ExpandNeighbors(gNext);
	
	SetCachedHCost(gNext, GetGCost(gNext));
	closedList[gNext->GetNum()] = GNode(gNext);
	return 0;
}

bool IRAStar::InitializeSearch(GraphAbstraction *aMap, node *from, node *to)
{
	closedList.clear();
	q.reset();

	currentIteration = 0;
	gStart = 0;
	absGraph = aMap;
	aStart = from;
	aGoal = to;
	delete g;
	g = new Graph();
	nodesExpanded = nodesTouched = nodesRefined = 0;
	
	// find most abstract node in Graph
	node *top = FindTopLevelNode(from, to, aMap);
	if (top == 0)
		return false;
	node *newTop = new node("top");
	gStart = gGoal = newTop;
	g->AddNode(newTop);
	SetInitialValues(newTop, top, 0);
	q.Add(GNode(newTop));
	return true;
}

node *IRAStar::FindTopLevelNode(node *one, node *two, GraphAbstraction *aMap)
{
	if ((one == 0) || (two == 0))
		return 0;
	if (aMap->GetAbstractionLevel(one) >= aMap->getNumAbstractGraphs())
		return 0;
	if (aMap->GetAbstractionLevel(one) == aMap->getNumAbstractGraphs() - 1)
	{
		if (one == two)
			return one;
		return 0;
	}
	node *tmp = FindTopLevelNode(aMap->GetParent(one), aMap->GetParent(two), aMap);
	if ((tmp == 0) && (one == two))
		return one;
	return tmp;
}

/*
 * aRealNode is the node inside the absGraph
 * gNewNode is the node about to be added to Graph
 * gParent is the parent inside the Graph
 */
void IRAStar::SetInitialValues(node *gNewNode, node *aRealNode, node *gParent)
{
	gNewNode->SetLabelL(kAbstractionLevel, absGraph->GetAbstractionLevel(aRealNode));
	gNewNode->SetLabelL(kCorrespondingNode, aRealNode->GetNum());
	if (gParent)
	{
		gNewNode->SetLabelL(kCachedHCost1, gParent->GetLabelL(kCachedHCost1));
		gNewNode->SetLabelL(kCachedHCost2, gParent->GetLabelL(kCachedHCost2));
	}
	else {
		//gNewNode->SetLabelL(kIteration, -1);
		SetGCost(gNewNode, 0);
		SetHCost(gNewNode, 0);
		gNewNode->SetLabelL(kCachedHCost1, 0);
		gNewNode->SetLabelL(kCachedHCost2, 0);
	}
//	gNewNode->SetLabelL(kOptimalFlag, 0);
//	gNewNode->SetLabelL(kInOpenList, 1);

	if (absGraph->IsParentOf(aRealNode, aStart) || (aRealNode == aStart))
	{
		//std::cout << "Assigning start to " << *gNewNode << std::endl;
		gStart = gNewNode;
	}
	if (absGraph->IsParentOf(aRealNode, aGoal) || (aRealNode == aGoal))
	{
		//std::cout << "Assigning goal to " << *gNewNode << std::endl;
		gGoal = gNewNode;
	}
}

void IRAStar::ExpandNeighbors(node *gNode)
{
	neighbor_iterator ni = gNode->getNeighborIter();
	for (int next = gNode->nodeNeighborNext(ni); next != -1; next = gNode->nodeNeighborNext(ni))
	{
		nodesTouched++;
		node *gNeighbor = g->GetNode(next);
		if (q.IsIn(GNode(gNeighbor))) // check for lower cost
		{
			if (fless(GetGCost(gNode)+1.0,
								GetGCost(gNeighbor)))
			{
				//printf("Updating neighbor %d to gCost %f\n", gNeighbor->GetNum(), GetGCost(gNode)+1);
				SetGCost(gNeighbor, GetGCost(gNode)+1.0);
				//gNeighbor->SetLabelL(kIteration, -1);
				q.DecreaseKey(GNode(gNeighbor));
			}
		}
		else if (closedList.find(gNeighbor->GetNum()) != closedList.end())
		{
		}
		else { // add to open list
			SetHCost(gNeighbor, GetCachedHCost(gNeighbor));
			SetGCost(gNeighbor, GetGCost(gNode)+1.0);
			q.Add(GNode(gNeighbor));
//			printf("Adding neighbor %d with gCost %f, hCost %f\n",
//						 gNeighbor->GetNum(), GetGCost(gNode)+1, GetHCost(gNeighbor));
		}
	}
}

path *IRAStar::ExtractAndRefinePath()
{
	bool done = true;

	path *p = GetSolution(gGoal);
	iterationLimits.push_back(GetGCost(gGoal));
	for (path *i = p; i; i = i->next)
	{
		if (i->n->GetLabelL(kAbstractionLevel) != 0)
		{
			done = false;
		}
	}
	if (done)
		return p;

	std::vector<node*> nodes;
	GetAllSolutionNodes(gGoal, nodes);
	for (unsigned int x = 0; x < nodes.size(); x++)
	{
		//printf("## Refining %d\n", nodes[x]->GetNum());
		RefineNode(nodes[x]);
	}
	printf("%d refined nodes %d expanded nodes\n", nodesRefined, nodesExpanded);
	closedList.clear();
	q.reset();

	currentIteration++;
	if ((currentIteration%2) == 1)
	{
		node *tmp = gStart;
		gStart = gGoal;
		gGoal = tmp;
	}
	SetGCost(gStart, 0);
	SetHCost(gStart, GetCachedHCost(gStart));
	q.Add(GNode(gStart));
		
	if (done)
		return p;
	delete p;
	return 0;
}

void IRAStar::GetAllSolutionNodes(node *goal, std::vector<node*> &nodes)
{
	nodes.push_back(goal);
	closedList.erase(goal->GetNum());
	for (unsigned int x = 0; x < nodes.size(); x++)
	{
		node *gNode = nodes[x];

		neighbor_iterator ni = gNode->getNeighborIter();
		for (int next = gNode->nodeNeighborNext(ni); next != -1; next = gNode->nodeNeighborNext(ni))
		{
			node *gNeighbor = g->GetNode(next);
			if (closedList.find(gNeighbor->GetNum()) != closedList.end())
			{
//				printf("Neighbor (%d) has g-cost %1.2f, solution path through (%d) has cost %1.2f\n",
//							 gNeighbor->GetNum(), gNeighbor->GetLabelF(kGCost),
//							 gNode->GetNum(), gNode->GetLabelF(kGCost));
				if (!fgreater(GetGCost(gNeighbor)+1, GetGCost(gNode)))
				{
//					printf("Adding to list!\n");
					closedList.erase(gNeighbor->GetNum());
					nodes.push_back(gNeighbor);
				}
			}
		}
	}
}

path *IRAStar::GetSolution(node *gNode)
{
	neighbor_iterator ni = gNode->getNeighborIter();
	for (int next = gNode->nodeNeighborNext(ni); next != -1; next = gNode->nodeNeighborNext(ni))
	{
		node *gNeighbor = g->GetNode(next);
		if (closedList.find(gNeighbor->GetNum()) != closedList.end())
		{
			node* n = closedList[gNeighbor->GetNum()].n; // silly!
			if (fequal(GetGCost(n) + 1.0, GetGCost(gNode)))
			{
				return new path(gNode, GetSolution(n));
			}
		}
	}	
	return new path(gNode, 0);
}

void IRAStar::RefineNode(node *gNode)
{
	if (absGraph->GetAbstractionLevel(gNode) == 0)
	{
		if (GetRealNode(gNode) == aStart)
			gStart = gNode;
		if (GetRealNode(gNode) == aGoal)
			gStart = gNode;
		return;
	}
	nodesRefined++;
	std::vector<node *> aChildren;
	std::vector<node *> gChildren;
	node *aNode = GetRealNode(gNode);
	for (int x = 0; x < absGraph->GetNumChildren(aNode); x++)
	{
		aChildren.push_back(absGraph->GetNthChild(aNode, x));
	}

	// first, add children to graph g
	for (unsigned int x = 0; x < aChildren.size(); x++)
	{
		gChildren.push_back(new node("child"));
		g->AddNode(gChildren[x]);
		SetInitialValues(gChildren[x], aChildren[x], gNode);
		//q.Add(GNode(gChildren[x]));
//		std::cout << "Adding child:" << std::endl;
//		std::cout << *gChildren[x] << std::endl;
	}

	// first, connect children to each other
	Graph *aGraph = absGraph->GetAbstractGraph(absGraph->GetAbstractionLevel(aChildren[0]));
	for (unsigned int x = 0; x < gChildren.size(); x++)
	{
		for (unsigned int y = 0; y < gChildren.size(); y++)
		{
			if (x != y)
			{
				edge *e;
				if ((e = aGraph->findDirectedEdge(aChildren[x]->GetNum(), aChildren[y]->GetNum())) != 0)
				{
					g->AddEdge(new edge(gChildren[x]->GetNum(), gChildren[y]->GetNum(), //1.0));
															(absGraph->GetAbstractionLevel(aChildren[0])==0)?e->GetWeight():1.5));
				}
			}
		}
	}

	// check neighbors of original node
	neighbor_iterator ni = gNode->getNeighborIter();
	for (int next = gNode->nodeNeighborNext(ni); next != -1; next = gNode->nodeNeighborNext(ni))
	{
		node *gNeighbor = g->GetNode(next);
		if (gNeighbor->GetLabelL(kAbstractionLevel) == gNode->GetLabelL(kAbstractionLevel)-1)
		{
			// same level
			for (unsigned int x = 0; x < gChildren.size(); x++)
			{
				edge *e;
				if ((e = aGraph->FindEdge(aChildren[x]->GetNum(), GetRealNode(gNeighbor)->GetNum())) != 0)
				{
					g->AddEdge(new edge(gChildren[x]->GetNum(), gNeighbor->GetNum(), //1.0));
															(absGraph->GetAbstractionLevel(aChildren[0])==0)?e->GetWeight():1.5));
				}
			}
		}
		else if (gNeighbor->GetLabelL(kAbstractionLevel) < gNode->GetLabelL(kAbstractionLevel)-1)
		{ // neighbor is lower
			for (unsigned int x = 0; x < aChildren.size(); x++)
			{
				if (ShouldAddEdge(GetRealNode(gNeighbor), aChildren[x]))
					g->AddEdge(new edge(gChildren[x]->GetNum(), gNeighbor->GetNum(), 1.0));
			}
		}
		else { // neighbor is higher
			for (unsigned int x = 0; x < aChildren.size(); x++)
			{
				if (ShouldAddEdge(aChildren[x], GetRealNode(gNeighbor)))
					g->AddEdge(new edge(gChildren[x]->GetNum(), gNeighbor->GetNum(), 1.0));
			}
		}
	}
	
	// last thing we do!
	g->RemoveNode(gNode);
}

node *IRAStar::GetRealNode(node *gNode)
{
	return absGraph->GetAbstractGraph(gNode->GetLabelL(kAbstractionLevel))->GetNode(gNode->GetLabelL(kCorrespondingNode));
}

bool IRAStar::ShouldAddEdge(node *aLowerNode, node *aHigherNode)
{
	neighbor_iterator ni = aLowerNode->getNeighborIter();
	for (int next = aLowerNode->nodeNeighborNext(ni); next != -1; next = aLowerNode->nodeNeighborNext(ni))
	{
		node *aNeighbor = absGraph->GetAbstractGraph(absGraph->GetAbstractionLevel(aLowerNode))->GetNode(next);
		if (absGraph->IsParentOf(aHigherNode, aNeighbor))
			return true;
	}
	return false;
}

void IRAStar::OpenGLDraw()
{
	if ((g == 0) || (g->GetNumNodes() == 0))
	{
		return;
	}
	
	glLineWidth(3.0);
	glBegin(GL_LINES);
	glNormal3f(0, 1, 0);
	edge_iterator ei = g->getEdgeIter();
	for (edge *e = g->edgeIterNext(ei); e; e = g->edgeIterNext(ei))
	{
		node *n;
		n = g->GetNode(e->getFrom());
		if (q.top().n == n)
			glColor3f(1.0, 0.0, 1.0);
		else if (n == gStart)
			glColor3f(0, 0, 1);
		else if (n == gGoal)
			glColor3f(0, 1, 0);
		else if (closedList.find(n->GetNum()) != closedList.end()) // on closed list
			glColor3f(1.0, 1.0, 0);
		else if (q.IsIn(GNode(n)))
			glColor3f(0.5, 0.5, 0.5);
		else
			glColor3f(1, 1, 1);
		
		recVec rv = absGraph->GetNodeLoc(GetRealNode(n));
		glVertex3f(rv.x, rv.y, rv.z);
		
		n = g->GetNode(e->getTo());
		if (q.top().n == n)
			glColor3f(1.0, 0.0, 1.0);
		else if (n == gStart)
			glColor3f(0, 0, 1);
		else if (n == gGoal)
			glColor3f(0, 1, 0);
		else if (closedList.find(n->GetNum()) != closedList.end()) // on closed list
			glColor3f(1.0, 1.0, 0);
		else if (q.IsIn(GNode(n)))
			glColor3f(0.5, 0.5, 0.5);
		else
			glColor3f(1, 1, 1);

		rv = absGraph->GetNodeLoc(GetRealNode(n));
		
		glVertex3f(rv.x, rv.y, rv.z);
	}

	glEnd();
	glLineWidth(1.0);
	
}

double IRAStar::GetHCost(node *n)
{
//	if (n->GetLabelL(kIteration) < currentIteration-1)
//		return std::max(iterationLimits.back(), val);
	return n->GetLabelL(kHCost);
}

void IRAStar::SetHCost(node *n, double val)
{
	n->SetLabelL(kHCost, val);
}

double IRAStar::GetCachedHCost(node *n)
{
	double val = 0;
	if ((currentIteration%2) == 0)
		val = n->GetLabelL(kCachedHCost2);
	else
		val = n->GetLabelL(kCachedHCost1);
	
//	if (n->GetLabelL(kIteration) < currentIteration-1)
//		return std::max(val-iterationLimits.back(), val);
}

void IRAStar::SetCachedHCost(node *n, double val)
{
	if ((currentIteration%2) == 0)
		n->SetLabelL(kCachedHCost1, val);
	else
		n->SetLabelL(kCachedHCost2, val);
}

double IRAStar::GetGCost(node *n)
{
	return n->GetLabelL(kGCost);
}

void IRAStar::SetGCost(node *n, double val)
{
	n->SetLabelL(kGCost, val);
}

double IRAStar::GetFCost(node *n)
{
	return n->GetLabelL(kGCost)+n->GetLabelL(kHCost);
}
