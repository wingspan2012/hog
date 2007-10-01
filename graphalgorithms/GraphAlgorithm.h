/*
 *  GraphAlgorithm.h
 *  hog2
 *
 *  Created by Nathan Sturtevant on 9/30/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef GRAPHALGORITHM_H
#define GRAPHALGORITHM_H

#include "GraphEnvironment.h"
#include "Graph.h"
#include <vector>

class GraphAlgorithm {
public:
	virtual ~GraphAlgorithm() {}
	virtual void GetPath(GraphEnvironment *env, Graph* _g, graphState from, graphState to, std::vector<graphState> &thePath) = 0;

	virtual long GetNodesExpanded() = 0;
	virtual long GetNodesTouched() = 0;
	virtual long GetNodesReopened() = 0;

	virtual bool InitializeSearch(GraphEnvironment *env, Graph* g, graphState from, graphState to, std::vector<graphState> &thePath) = 0;
	virtual bool DoSingleSearchStep(std::vector<graphState> &thePath) = 0;
	virtual void OpenGLDraw() = 0;
	virtual void OpenGLDraw(int window) = 0;
};

#endif
