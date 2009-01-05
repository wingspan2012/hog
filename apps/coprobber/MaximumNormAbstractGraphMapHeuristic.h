#include "GraphAbstraction.h"
#include "GraphEnvironment.h"
#include <stdlib.h>

#ifndef MAXIMUMNORMABSTRACTGRAPHMAPHEURISTIC_H
#define MAXIMUMNORMABSTRACTGRAPHMAPHEURISTIC_H

class MaximumNormAbstractGraphMapHeuristic : public GraphHeuristic {
public:
  MaximumNormAbstractGraphMapHeuristic(Graph *graph, Map *map)
  :g(graph), m(map) {}
  double HCost(graphState &state1, graphState &state2)
  {
		double x1 = g->GetNode(state1)->GetLabelF(GraphAbstractionConstants::kXCoordinate);
		double y1 = g->GetNode(state1)->GetLabelF(GraphAbstractionConstants::kYCoordinate);
		double x2 = g->GetNode(state2)->GetLabelF(GraphAbstractionConstants::kXCoordinate);
		double y2 = g->GetNode(state2)->GetLabelF(GraphAbstractionConstants::kYCoordinate);

		return( max( fabs(x1-x2), fabs(y1-y2) ) * m->getCoordinateScale() );
  }
private:
  Graph *g;
	Map *m;
};

#endif
