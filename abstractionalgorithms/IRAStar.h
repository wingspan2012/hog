/*
 *  IRAStar.h
 *  hog2
 *
 *  Created by Nathan Sturtevant on 10/12/07.
 *  Copyright 2007 Nathan Sturtevant, University of Alberta. All rights reserved.
 *
 */

#ifndef IRAStar_H
#define IRAStar_H

#include <ext/hash_map>
#include "SearchAlgorithm.h"
#include "OpenClosedList.h"
#include "FPUtil.h"
#include "Graph.h"
#include "GraphAbstraction.h"

namespace IRAStarConstants {
	
	/** Definitions for node labels */
	enum {
		kAbstractionLevel = 0, // this is a LONG label
		kCorrespondingNode = 1, // this is a LONG label
		kGCost = 2, // this is a DOUBLE label
		kHCost = 3,	// this is a DOUBLE label
		kCachedHCost1 = 4,	// this is a DOUBLE label
		kCachedHCost2 = 5,	// this is a DOUBLE label
		//kIteration = 6, // this is a LONG label
		//		kOptimalFlag = 4, // this is a LONG label
		//		kInOpenList = 5 // this is a LONG label
	};
	
	struct GNode {
		GNode(node *nn) :n(nn) {}
		GNode() :n(0) {}
		node *n;
	};
	
	struct NodeEqual {
		bool operator()(const GNode &i1, const GNode &i2)
		{ return (i1.n->getUniqueID() == i2.n->getUniqueID()); }
	};
	
	struct NodeCompare {
		// return true if we prefer i2 over i1
		bool operator()(const GNode &i1, const GNode &i2)
		{
			// return true if node1 has higher f-cost
			return (fgreater(i1.n->GetLabelL(kGCost)+i1.n->GetLabelL(kHCost),
											 i2.n->GetLabelL(kGCost)+i2.n->GetLabelL(kHCost)));
		}
	};
	
	struct NodeHash {
		size_t operator()(const GNode &x) const
	{ return (size_t)(x.n->getUniqueID()); }
	};
	
	typedef OpenClosedList<GNode, NodeHash,
		NodeEqual, NodeCompare> PQueue;
	
	typedef __gnu_cxx::hash_map<uint32_t, GNode> NodeLookupTable;
}


// variables starting with "a" are in the abstraction
// variables starting with "g" are in the defined search graph
class IRAStar : public SearchAlgorithm {
public:
	IRAStar();
	virtual ~IRAStar();
	virtual const char *GetName();
	virtual path *GetPath(GraphAbstraction *aMap, node *from, node *to, reservationProvider *rp = 0);
	path *DoOneSearchStep();
	bool InitializeSearch(GraphAbstraction *aMap, node *from, node *to);
	void OpenGLDraw();
	int GetNodesRefined() { return nodesRefined; }
private:
		node *FindTopLevelNode(node *one, node *two, GraphAbstraction *aMap);
	void SetInitialValues(node *gNewNode, node *aRealNode, node *gParent);
	//	void UpdateNode(node *gNode);
	//	void UpdateH(node *gNode);
	//	void UpdateG(node *gNode);
	//	void UpdateOptH(node *gNode);
	//	void MakeNeighborsOpen(node *gNode);
	void RefineNode(node *gNode);
	node *GetRealNode(node *gNode);
	bool ShouldAddEdge(node *aLowerNode, node *aHigherNode);
	
	void GetAllSolutionNodes(node *goal, std::vector<node*> &nodes);
	void ExpandNeighbors(node *gNode);
	path *ExtractAndRefinePath();
	path *GetSolution(node *gNode);
	double GetHCost(node *);
	void   SetHCost(node *, double);
	double GetCachedHCost(node *);
	void   SetCachedHCost(node *, double);
	double GetGCost(node *);
	void   SetGCost(node *, double);
	double GetFCost(node *);
	
	IRAStarConstants::PQueue q;
	IRAStarConstants::NodeLookupTable closedList;
	node *aStart, *aGoal;
	node *gStart, *gGoal;
	GraphAbstraction *absGraph;
	Graph *g;
	int nodesRefined;
	int currentIteration;
	std::vector<double> iterationLimits;
};


#endif
