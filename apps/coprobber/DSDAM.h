#include <vector>
#include "MapAbstraction.h"
#include "DSMinimax.h"
#include "PRAStar.h"
#include "MaximumNormAbstractGraphMapHeuristic.h"

#ifndef DSDAM_H
#define DSDAM_H

/*
	dynamic abstract minimax (DAM)
	code functions with DSCREnvironment, thus possibly faster cop
*/

class DSDAM {

	public:

	// constructor
	DSDAM( MapAbstraction *gabs, bool canPause = true, unsigned int cop_speed = 1, bool useAbstraction = true );
	~DSDAM();

	void dam( node* pos_robber, node* pos_cop, std::vector<node*> &path, bool minFirst = true, double depth = 5. );

	node* MakeMove( node* pos_robber, node* pos_cop, bool minFirst = true, double depth = 5. );

	// statistic variables that get resetted everytime dam(...) is called
	unsigned int nodesExpanded, nodesTouched;

	protected:

	MapAbstraction *gabs;
	bool canPause;
	unsigned int cop_speed;
	bool useAbstraction;

	praStar *pra;

	// cache the minimax objects and graph environments for each level of abstraction
	std::vector<DSMinimax<graphState,graphMove>* > dsminimax;
	std::vector<GraphEnvironment*> graphenvironments;
	std::vector<MaximumNormAbstractGraphMapHeuristic*> graphmapheuristics;
};

#endif
