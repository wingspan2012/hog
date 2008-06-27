#include <math.h>
#include <vector>
#include "CopRobberGame.h"
#include "MyHash.h"

CopRobberGame::CopRobberGame( GraphEnvironment *_genv, unsigned int _num_cops, bool simultaneous, bool playerscanpass ):
	CopRobberEnvironment<graphState,graphMove>( (SearchEnvironment<graphState,graphMove>*) _genv, playerscanpass ),
	MarkovGame<graphState,graphMove>( simultaneous ),
	num_cops(_num_cops),
	genv(_genv)
{
}

CopRobberGame::~CopRobberGame() {};

unsigned int CopRobberGame::GetNumPlayers() const {
	return num_cops+1;
}

void CopRobberGame::GetPossiblePlayerActions( unsigned int player, CRState s, std::vector<CRAction> &actions ) {

	std::vector<graphMove> orig_actions;

	// cleanup
	actions.clear();

	if( playerscanpass )
		actions.push_back( CRAction() );

	// get all possible actions of the player on the graph without respecting
	// the semantics of the game (CopRobberOccupancy)
	env->GetActions( s[player], orig_actions );

	for( unsigned int i = 0; i < orig_actions.size(); i++ ) {
		actions.push_back( CRAction( orig_actions[i] ) );
	}
	return;
}

void CopRobberGame::GetPossibleOpponentActions( unsigned int excluded_player, CRState s, std::vector<CRMove> &actions ) {

	// number of opponents is the number of all agents excluding exluded_player
	unsigned int num_opps = s.size() - 1;

	unsigned long i, j;
  unsigned int k;
  std::vector<graphMove> p_actions[num_opps+1];
  CRState p_state;
  graphState temp_s;
  // as=agent actions, n__=number of, m_=my
  unsigned int naa[num_opps], aa[num_opps], nmaa = 0, maa;
  // t___=total
  unsigned long tnaa = 0;
	bool found_action;

  actions.clear();

  // get the actions of each individual agent
	j = 0;
  for( i = 0; i < num_opps+1; i++ ) {
    env->GetActions( s[i], p_actions[i] );
		if( i == excluded_player ) {
			nmaa = p_actions[i].size() + (playerscanpass?1:0);
		} else {
	    // if an agent can pass his turn then we augment all the numbers
 	   // by one and 0 becomes the pass action
 	   naa[j] = p_actions[i].size() + (playerscanpass?1:0);
 	   if( tnaa ) tnaa *= naa[j];
 	   else tnaa = naa[j];
		 j++;
		}
  }

	j = 0;
	for( i = 0; i < tnaa; i++ ) {
		dehash_permutation( i, num_opps, naa, aa );
		actions.push_back( CRMove() );
		p_state.clear();

		for( k = 0; k < excluded_player; k++ ) {
			if( playerscanpass && aa[k] == 0 ) {
				actions[j].push_back( CRAction() );
				p_state.push_back( s[k] );
			} else {
				actions[j].push_back( CRAction(p_actions[k][aa[k]-(playerscanpass?1:0)]) );
				// apply the action
				temp_s = s[k];
				env->ApplyAction( temp_s, actions[j][k].a );
				p_state.push_back( temp_s );
			}
		}
		// push_back a temporary actions for the excluded_player
		actions[j].push_back( CRAction() );
		p_state.push_back( s[excluded_player] );
		// push_back the rest of the player actions
		for( k = excluded_player+1; k < num_opps+1; k++ ) {
			if( playerscanpass && aa[k-1] == 0 ) {
				actions[j].push_back( CRAction() );
				p_state.push_back( s[k] );
			} else {
				actions[j].push_back( CRAction(p_actions[k][aa[k-1]-(playerscanpass?1:0)]) );
				temp_s = s[k];
				env->ApplyAction( temp_s, actions[j][k].a );
				p_state.push_back( temp_s );
			}
		}

		found_action = false;

		for( maa = 0; maa < nmaa; maa++ ) {
			if( !playerscanpass || maa != 0 ) {
				actions[j][excluded_player] = p_actions[excluded_player][maa-(playerscanpass?1:0)];
				temp_s = s[excluded_player];
				env->ApplyAction( temp_s, actions[j][excluded_player].a );
				p_state[excluded_player] = temp_s;
			}

			if( oi->CanMove( s, p_state ) ) {
				found_action = true;
				break;
			}
		}

		if( found_action )
			j++;
		else
			actions.pop_back();
	}

	// cleanup
	for( i = 0; i < num_opps+1; i++ )
		p_actions[i].clear();

	return;
}

double CopRobberGame::GetReward( unsigned int player, CRState s, std::vector<CRAction> act ) {
	// if the game has ended yet
	if( GoalTest( s, s ) ) return 0.;

/* TEMPORARY TODO
	double temp, result = 0.;
	// determine the maximum time the robber cannot be caught
	for( unsigned int i = 0; i < s.size(); i++ ) {
		if( !act[i].noaction ) {
			temp = env->GCost( s[i], act[i].a );
			if( result < temp ) result = temp;
		}
	}

	if( player ) return( -1. * result );
	else return result;
*/


	if( player ) return -1.; // cop
	else return 1.; // robber
}

unsigned int CopRobberGame::GetNumStates() const {
	return( (unsigned int) pow( genv->GetGraph()->GetNumNodes(), num_cops+1 ) );
}

// This function is exploiting knowledge about how the GraphEnvironment works
// Unfortunately there is no easy way to avoid this
CopRobberGame::CRState CopRobberGame::GetStateByNumber( unsigned int num ) {
	CRState p_state;
	unsigned int i, nps[num_cops+1], ps[num_cops+1], t;
	t = genv->GetGraph()->GetNumNodes();

	for( i = 0; i < num_cops+1; i++ )
		nps[i] = t;
	dehash_permutation( num, num_cops+1, nps, ps );
	for( i = 0; i < num_cops+1; i++ ) {
		p_state.push_back( ps[i] );
	}
	return p_state;
}

// as GetStateByNumber this exploits knowledge about the GraphEnvironment
unsigned int CopRobberGame::GetNumberByState( CRState s ) {
	unsigned int nps[num_cops+1], ps[num_cops+1], t;

	// security statement
	assert( s.size() == num_cops+1 );

	t = genv->GetGraph()->GetNumNodes();
	for( unsigned int i = 0; i < s.size(); i++ ) {
		nps[i] = t;
		ps[i]  = (unsigned int) s[i];
	}
	return (unsigned int) hash_permutation( num_cops+1, nps, ps );
}


void CopRobberGame::WriteExpectedRewardToDisc( const char* filename, unsigned int player, double *V ) {
	FILE *fhandler;
	unsigned int tns = GetNumStates(), tnp = GetNumPlayers(), i, j;
	CRState s;

	fhandler = fopen( filename, "w" );
	fprintf( fhandler, "number of players: %u\n", tnp );
	fprintf( fhandler, "states in space: %u\n", tns );
	fprintf( fhandler, "expected rewards for player: %u\n", player );
	fprintf( fhandler, "\n\n" );
	fprintf( fhandler, "positions:\n" );
	for( i = 0; i < tnp; i++ ) {
		fprintf( fhandler, "player%u ", i );
	}
	fprintf( fhandler, "expected reward\n" );
	for( i = 0; i < tns; i++ ) {
		s = GetStateByNumber( i );
		for( j = 0; j < tnp; j++ ) {
			fprintf( fhandler, "%lu ", s[j] );
		}
		fprintf( fhandler, "%g\n", V[i] );
	}
	fclose( fhandler );
	return;
}

void CopRobberGame::ReadExpectedRewardFromDisc( const char* filename, unsigned int &player, double *&V ) {
	FILE *fhandler;
	unsigned int tnp, tns, i, j;
	graphState ss;
	CRState s;

	fhandler = fopen( filename, "r" );
	if( fhandler == 0 ) {
		fprintf( stderr, "ERROR: File %s not found\n", filename );
		return;
	}

	fscanf( fhandler, "number of players: %u\n", &tnp );
	if( tnp != GetNumPlayers() ) {
		fprintf( stderr, "ERROR: Wrong number of players in %s (should be %u)\n", filename, GetNumPlayers() );
		return;
	}
	fscanf( fhandler, "states in space: %u\n", &tns );
	if( tns != GetNumStates() ) {
		fprintf( stderr, "ERROR: Wrong number of states in space in %s (should be %u)\n", filename, GetNumStates() );
		return;
	}
	fscanf( fhandler, "expected rewards for player: %u\n", &player );
	fscanf( fhandler, "\n\n" );

	fscanf( fhandler, "positions:\n" );
	for( i = 0; i < tnp; i++ ) fscanf( fhandler, "player%*u " );
	fscanf( fhandler, "expected reward\n" );

	// allocate V
	V = new double[tns];

	// first assignment
	for( i = 0; i < tns; i++ )
		V[i] = 0.;

	for( i = 0; i < tns; i++ ) {
		s.clear();
		for( j = 0; j < tnp; j++ ) {
			fscanf( fhandler, "%lu", &ss );
			s.push_back( ss );
		}
		fscanf( fhandler, "%lg\n", &V[GetNumberByState(s)] );
	}
	fclose( fhandler );
}
