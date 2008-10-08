#include <vector>
#include <queue>
#include <ext/hash_map>
#include "GraphEnvironment.h"
#include "CopRobberGame.h"
#include "Minimax.h"

#ifndef MINIMAXASTAR_H
#define MINIMAXASTAR_H

template<>
uint64_t CRHash<graphState>( const std::vector<graphState> &s );

/*
	Implementation for one robber and multiple cops
*/
class MinimaxAStar {

	public:

	typedef CopRobberGame::CRState CRState;
	typedef CopRobberGame::CRMove CRMove;
	typedef CopRobberGame::CRAction CRAction;

	// priority queue
	class QueueEntry {
		public:
		QueueEntry( CRState _pos, bool mf, double fv, double gv ):
			pos(_pos), minFirst(mf), fvalue(fv), gvalue(gv) {};
		QueueEntry() {};

		CRState pos;
		bool minFirst;
		double fvalue;
		double gvalue;
	};

	// be aware of q1.fvalue > q2.fvalue, this changes the ordering of our queue s.t.
	// we can top() the one with the lowest value
	struct QueueEntryCompare {
		bool operator() ( const QueueEntry q1, const QueueEntry q2 ) const {
			return( q1.fvalue > q2.fvalue );
		}
	};

	struct CRStateHash {
		size_t operator() ( const CRState s ) const {
			return CRHash<graphState>( s );
		}
	};

	typedef std::priority_queue<QueueEntry, std::vector<QueueEntry>, QueueEntryCompare> MyPriorityQueue;
	typedef __gnu_cxx::hash_map<CRState, double, CRStateHash> MyClosedList;

	// constructor
	MinimaxAStar( GraphEnvironment *_env, unsigned int _number_of_cops, bool _canPass );
	~MinimaxAStar();

	double astar( CRState pos, bool minFirst, double weight = 1. );

	protected:

	void push_end_states_on_queue( CRState &goal_pos, bool &goal_minFirst, double &weight );
	double compute_target_value( CRState &s );

	// we use these functions independent from the environment implementations
	// to be more variable in our own calculations
	double MinGCost( CRState &pos1, CRState &pos2 );
	// the definition of the heuristic relies on the graph heuristic used
	// in the submitted graph environment (=> use MyGraphMapHeuristic)
	double HCost( CRState &pos1, bool &minFirst1, CRState &pos2, bool &minFirst2 );

	GraphEnvironment *env;
	unsigned int number_of_cops;
	bool canPass;
	CopRobberGame *crg;

	// Priority Queues
	MyPriorityQueue queue;
	// state => value
//	std::vector<double> min_cost, max_cost;
	MyClosedList min_cost, max_cost;

};



#endif
