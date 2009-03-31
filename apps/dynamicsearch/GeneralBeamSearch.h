#ifndef GENBEAMSEARCH_H
#define GENBEAMSEARCH_H
#include "StatCollection.h"
#include "GenericStepAlgorithm.h"
#include <algorithm>
#include <ext/hash_map>
#include <iostream>
#include "BeamNode.h"
#include <assert.h>
#define NO_CHILDREN -100

/**
A class for performing regular beam search.
**/
template <class state, class action, class environment>
class GeneralBeamSearch : public GenericStepAlgorithm<state, action, environment> {
public:
	GeneralBeamSearch() {
		beam_size = 1;
		memory_limit = 50;
		g_weight = 1.0; // weights don't matter if actions are constant cost
		h_weight = 1.0;
		step_by_step_active = false;

		best_path_cost = -1.0;

		prune_dups = false;
		debug = false;
	}

	/** See GenericStepAlgorithm **/
	int GetPath(environment *env, state from, state to, std::vector<state> &path);
	int GetPath(environment *env, state from,
	            state to, std::vector<action> &path){return 0;}

	const char *GetName() {return "General Beam Search";}

	virtual uint64_t GetNodesExpanded() { return nodes_expanded; }
	virtual uint64_t GetNodesTouched() { return nodes_touched; }
	virtual uint64_t GetNodesChecked() {return nodes_checked; }

	void LogFinalStats(StatCollection *stats){}

	bool Initialize(environment *env, state start, state goal);

	int StepAlgorithm(std::vector<action> &path){return 0;}
	int StepAlgorithm(std::vector<state> &path);

	/** Changes the size of the beam. Default setting is 1. Returns true if succeeds, false otherwise. **/
	bool Change_Beam_Size(unsigned new_beam_size) {
		if(new_beam_size > 0) { beam_size = new_beam_size; return true; }
		return false;
	}
	/** Changes memory limit for beam search. That is, the number of nodes that can be in the beam
	 at one time. Default is set at 50.**/
	void Change_Memory_Limit(unsigned new_limit) {memory_limit = new_limit;}

	/** Sets a limit on the number of nodes to expand - input 0 for no limit **/
	virtual bool SetExpandedLimit(uint64_t limit);

	/** Sets a limit on the number of nodes to generate - input 0 for no limit **/
	virtual bool SetTouchedLimit(uint64_t limit);

	/** Sets a limit on the number of nodes to check - input 0 for no limit **/
	virtual bool SetCheckedLimit(uint64_t limit);

	/**
	Checks if the algorithm is ready to be incremented one step
	**/
	bool Is_Step_Active() {return step_by_step_active;}

	/**
	Prints information on all items in the beams.
	**/
	void print_beams();

	/** returns the path cost of the last path found **/
	double GetPathCost() {return best_path_cost; }

	/** Sets whether to remove duplicated nodes or not. If it is set to true, any node
	 that is in the beams cannot be added a second time to the latest beam. **/
	void Select_Duplicate_Prune(bool to_prune) {prune_dups = to_prune;}

	/**
	Returns the value of the applicable weight
	**/
	double Get_H_Weight() { return h_weight;}
	double Get_G_Weight() { return g_weight;}

	/**
	Changes the weights. Note, if the problem is constant cost, changing the weights
	does nothing.
	**/
	int Change_Weights(double h_g,double h_w) {
		if(!fless (h_g, 0.0) && !fless(h_w, 0.0)) {
			h_weight = h_w;
			g_weight = h_g;
			return 1;
		}
		return 0;
	}

protected:
	bool step_by_step_active;

	unsigned beam_size;
	unsigned memory_limit;
	uint64_t nodes_checked, nodes_expanded, nodes_touched;
	uint64_t expanded_limit, checked_limit, touched_limit;

	bool bound_expanded;
	bool bound_touched;
	bool bound_checked;

	bool prune_dups;

	/** Prepares the vars for search**/
	void prepare_vars_for_search();

	std::vector<std::vector <BeamNode<state> > > beams; // beam construct

	__gnu_cxx::hash_map<uint64_t, unsigned, Hash64> current_layer; // nodes in the current layer constructing
	__gnu_cxx::hash_map<uint64_t, unsigned, Hash64> children_nodes; // children of the current node being expanded
	__gnu_cxx::hash_map<uint64_t, unsigned, Hash64> nodes_in_beam; // nodes in beams not in current layer

	environment *my_env;
	state goal;

	/**
	Returns the cost for this state based on the g and h values.
	**/
	virtual double get_cost(double g, double h);

	void extract_goal(std::vector<state> &state_path);

	double g_weight;
	double h_weight;

	unsigned beam_index;
	unsigned nodes_stored;

	double best_path_cost;

	int generate_all_successors(environment *env, std::vector<BeamNode<state> > &successors, state &goal);
	int expand_beam(environment *env, state &goal);

	bool debug;
};

template <class state, class action, class environment>
void GeneralBeamSearch<state, action, environment>::prepare_vars_for_search() {
	nodes_checked = 0;
	nodes_expanded = 0;
	nodes_touched = 0;

	current_layer.clear();
	beams.clear();
	nodes_in_beam.clear();
}

/** Calculates f-cost in standard way **/
template <class state, class action, class environment>
double GeneralBeamSearch<state, action, environment>::get_cost(double g, double h) {
	return g_weight*g + h_weight*h;
}

template <class state, class action, class environment>
bool GeneralBeamSearch<state, action, environment>::SetExpandedLimit(uint64_t limit) {
	if(limit > 0) {
		bound_expanded = true; expanded_limit = limit;
	}
	else {
		bound_expanded = false;
	}
	return true;
}

template <class state, class action, class environment>
int GeneralBeamSearch<state, action, environment>::GetPath(environment *env, state from, state to, std::vector<state> &path) {
	prepare_vars_for_search();

	double h_value;

	if(env->IsGoalStored()) {
		h_value = env->HCost(from);
	}
	else {
		h_value = env->HCost(from, to);
	}
	double initial_cost = get_cost(0.0, h_value);
	uint64_t initial_key = env->GetStateHash(from);

	// constructs initial beamnode
	BeamNode<state> start_node(from, 0.0, h_value, initial_cost, 0,initial_key);

	std::vector<BeamNode<state> > first_layer;
	first_layer.push_back(start_node);
	beams.push_back(first_layer); // puts first beam on the stack

	if(prune_dups)
		nodes_in_beam[initial_key] = 0;

	int status = 0;

	nodes_stored = 1;
	nodes_touched = 1;

	step_by_step_active = false;
	status = expand_beam(env, to);

	if(status == 1) {
		extract_goal(path);
	}
	return status;

}

template <class state, class action, class environment>
int GeneralBeamSearch<state, action, environment>::expand_beam(environment *env, state &goal) {

	std::vector<BeamNode<state> > successors;

	int status = generate_all_successors(env, successors, goal);

	if(status == 1) {
		// put goal on end of the beam stack
		assert(successors.size() == 1);
		beams.push_back(successors);
	}
	else if(status == NO_CHILDREN) // next beam is empty
		return 0;

	if(status != 0) {
		return status;
	}

	// correct size of successors
	while(successors.size() > beam_size) {
		successors.pop_back();
	}

	nodes_stored += successors.size();

	// ran out of memory
	if(nodes_stored >= memory_limit) {
		return 0;
	}

	unsigned succ_index = 0;
	while(prune_dups && succ_index < successors.size()) { // stores node if need be
		nodes_in_beam[successors[succ_index].my_key] = 0;
		succ_index++;
	}

	// put new successors on beam stack
	beams.push_back(successors);

	status = expand_beam(env, goal);

	return status;
}

template <class state, class action, class environment>
bool GeneralBeamSearch<state, action, environment>::SetTouchedLimit(uint64_t limit) {
	if(limit > 0) {
		bound_touched = true; touched_limit = limit;
	}
	else {
		bound_touched = false;
	}
	return true;
}

template <class state, class action, class environment>
bool GeneralBeamSearch<state, action, environment>::SetCheckedLimit(uint64_t limit) {
	if(limit > 0) {
		bound_checked = true; checked_limit = limit;
	}
	else {
		bound_checked = false;
	}
	return true;
}

template <class state, class action, class environment>
bool GeneralBeamSearch<state, action, environment>::Initialize(environment *env, state from, state to) {
	prepare_vars_for_search();

	my_env = env;
	goal = to;

	double h_value;

	if(env->IsGoalStored()) {
		h_value = env->HCost(from);
	}
	else {
		h_value = env->HCost(from, goal);
	}
	double initial_cost = get_cost(0.0, h_value);
	uint64_t initial_key = my_env->GetStateHash(from);

	// constructs initial beamnode
	BeamNode<state> start_node(from, 0.0, h_value, initial_cost, 0,initial_key);

	std::vector<BeamNode<state> > first_layer;
	first_layer.push_back(start_node);
	beams.push_back(first_layer); // initiates beams

	current_layer[initial_key] = 0;
	beam_index = 0;
	step_by_step_active = true;

	nodes_stored = 1;
	nodes_in_beam.clear();
	if(prune_dups) { // stores node if need be
		nodes_in_beam[initial_key] = 0;
	}
	return true;
}

template <class state, class action, class environment>
int GeneralBeamSearch<state, action, environment>::StepAlgorithm(std::vector<state> &path) {

	if(!step_by_step_active) // if can't use step by step at this time
		return -1;

	unsigned layer_index;

	// figure out correct beam to select from, and set max_cost
	if(beam_index == 0) { // will need to create a new beam
		layer_index = beams.size() - 1;
		current_layer.clear();
	}
	else {
		layer_index = beams.size() - 2;
	}

	BeamNode<state> parent = beams[layer_index][beam_index];
	std::vector<state> children;
	my_env->GetSuccessors(parent.my_state, children); // get successors of state

	nodes_expanded++;
	nodes_touched += children.size();

	if(bound_expanded && nodes_expanded >= expanded_limit) {
		step_by_step_active = false;
		return EXPAND_MET;
	}

	if(bound_touched && nodes_touched >= touched_limit) {
		step_by_step_active = false;
		return TOUCHED_MET;
	}

	std::vector<BeamNode<state> > children_beam; // new beam for children
	children_nodes.clear(); // clear holder vector

	/* For Debugging
	std::cout << layer_index << " " << parent.my_state << '\n'; */
	for(unsigned i = 0; i < children.size(); i++) {

		// prevents inversion of moves
		if(layer_index > 0 &&
		   children[i] == beams[layer_index - 1][parent.parent_index].my_state) {
			continue;
		}
		// if have found the goal, stop and return the answer
		if(my_env->GoalTest(children[i], goal)) {
			double child_g = parent.g_value + my_env->GCost(parent.my_state, children[i]);
			uint64_t child_key = my_env->GetStateHash(children[i]);

			double child_h;

			if(my_env->IsGoalStored()) {
				child_h = my_env->HCost(children[i]);
			}
			else {
				child_h = my_env->HCost(children[i], goal);
			}
			double child_cost = get_cost(child_g, child_h);

			BeamNode<state> goal(children[i], child_g, child_h, child_cost, beam_index, child_key);
			children_beam.clear();
			children_beam.push_back(goal);

			if(layer_index != beams.size() - 1)
				beams.pop_back();

			beams.push_back(children_beam);
			extract_goal(path);


			best_path_cost = child_g;
			step_by_step_active = false;
			return 1;
		}

		nodes_checked++;

		if(bound_checked && nodes_checked >= checked_limit) {
			step_by_step_active = false;
			return CHECKED_MET;
		}

		double child_g = parent.g_value + my_env->GCost(parent.my_state, children[i]);
		uint64_t child_key = my_env->GetStateHash(children[i]);

		// if node is not already on beam, or of the new children generated
		if(current_layer.find(child_key) != current_layer.end() ||
		   children_nodes.find(child_key) != children_nodes.end() ||
		   (prune_dups && nodes_in_beam.find(child_key) != nodes_in_beam.end())) {
			   continue;
		}
		double child_h;

		if(my_env->IsGoalStored()) {
			child_h = my_env->HCost(children[i]);
		}
		else {
			child_h = my_env->HCost(children[i], goal);
		}
		double child_cost = get_cost(child_g, child_h);

		// construct new BeamNode
		BeamNode<state> new_node(children[i], child_g, child_h, child_cost, beam_index, child_key);

		children_beam.push_back(new_node);
		children_nodes[child_key] = 0; // store child as here

		/* For debugging
		printf("Candidate: ");
		std::cout << new_node.my_state;
		printf(" %.0f %.0f %.0f\n", new_node.g_value, new_node.h_value, new_node.cost);*/
	}

	// sorts children
	sort(children_beam.begin(), children_beam.end());

	if(beam_index == 0) { // add new layer to beams if is first of new beam
		while(children_beam.size() > beam_size) { // get rid of back of beam
			children_beam.pop_back();
		}
		beams.push_back(children_beam);

		nodes_stored += children_beam.size();

		if(nodes_stored >= memory_limit) {
			step_by_step_active = false;
			return NO_SOLUTION;
		}
	}
	else { // beam already full or partially full, need to merge lists
		std::vector<BeamNode<state> > new_layer;

		unsigned new_children_index = 0;
		unsigned old_beam_index = 0;

		nodes_stored -= beams[beams.size() -1].size(); // are to discard these nodes

		// performs a merge sort
		while(new_layer.size() < beam_size && (new_children_index < children_beam.size() || old_beam_index < beams[beams.size() -1].size())) {

			// if new children layer is out of states
			if(new_children_index == children_beam.size()) {
				new_layer.push_back(beams[beams.size() -1][old_beam_index]);
				old_beam_index++;
			}
			// if old beam is out of states
			else if(old_beam_index == beams[beams.size() -1].size()) {
				new_layer.push_back(children_beam[new_children_index]);
				new_children_index++;
				current_layer[children_beam[new_children_index].my_key] = 0;
			}
			// if the next child in new children is best candidate
			else if(children_beam[new_children_index] < beams[beams.size() -1][old_beam_index]) {
				new_layer.push_back(children_beam[new_children_index]);
				new_children_index++;
				current_layer[children_beam[new_children_index].my_key] = 0;
			}
			else {
				new_layer.push_back(beams[beams.size() -1][old_beam_index]);
				old_beam_index++;
			}
		}

		// remove the hash keys for discarded states
		for(; old_beam_index < beams[beams.size() -1].size(); old_beam_index++) {
			current_layer.erase(beams[beams.size() -1][old_beam_index].my_key);
		}

		nodes_stored += new_layer.size();

		if(nodes_stored >= memory_limit) {
			step_by_step_active = false;
			return -5;
		}

		beams.pop_back(); // remove the old beam from the stack
		beams.push_back(new_layer); // add the fixed layer to the stack
	}

	// fix value of beam_index
	beam_index++;
	if(beam_index == beams[layer_index].size()) {
		beam_index = 0;

		if(prune_dups) {
			for(unsigned i = 0; i < beams[beams.size()-1].size(); i++) {
				nodes_in_beam[ beams[beams.size()-1][i].my_key] = beams.size() - 1;
			}
		}
	}
	return 0;
}

template <class state, class action, class environment>
void GeneralBeamSearch<state, action, environment>::print_beams() {
	printf("\n\n");
	for(unsigned i = 0; i < beams.size(); i++) {
		for(unsigned j = 0; j < beams[i].size(); j++) {

			for(unsigned k = 0; k < i; k++)
				printf("\t");

			if((j == 0 && beam_index == 0 && i == beams.size() - 1) ||
			   (beam_index != 0 && beam_index == j && i == beams.size() - 2)) {
				printf("Expanding Next: ");
			}
			std::cout << beams[i][j].my_state;
			printf(" G:%.0f H:%.0f Cost:%.0f\n", beams[i][j].g_value, beams[i][j].h_value, beams[i][j].cost);
		}
	}
}

template <class state, class action, class environment>
void GeneralBeamSearch<state, action, environment>::extract_goal(std::vector<state> &state_path) {
	state_path.resize(beams.size()); // properly resizes beam, last spot will have final node added elsewhere

	unsigned beam_layer = beams.size() - 1;
	unsigned current_index = beams[beam_layer].size() - 1;

	while(beam_layer > 0) { // extract states from beam
		state_path[beam_layer] = beams[beam_layer][current_index].my_state;
		current_index = beams[beam_layer][current_index].parent_index;
		beam_layer--;
	}

	state_path[beam_layer] = beams[beam_layer][current_index].my_state;
}

template <class state, class action, class environment>
int GeneralBeamSearch<state, action, environment>::generate_all_successors(environment *env, std::vector<BeamNode<state> > &successors, state &goal) {

	current_layer.clear(); // nodes in current successors list
	successors.clear();

	std::vector<state> children;
	assert(beams.size() > 0);

	// iterate through states in last layer of beam
	for(unsigned i = 0 ; i < beams[beams.size() - 1].size(); i++) {
		nodes_expanded++;
		// if hit expansion limit
		if(bound_expanded && nodes_expanded >= expanded_limit) {
			return EXPAND_MET;
		}

		children.clear();
		env->GetSuccessors(beams[beams.size() - 1][i].my_state, children);
		nodes_touched += children.size();

		// if hit generated limit
		if(bound_touched && nodes_touched >= touched_limit) {
			return TOUCHED_MET;
		}

		// iterate through generated child
		for(unsigned j = 0; j < children.size(); j++) {
			uint64_t child_key = env->GetStateHash(children[j]);

			// if already have the node stored in beam or in current layer
			if((prune_dups && nodes_in_beam.find(child_key) != nodes_in_beam.end()) ||
			   current_layer.find(child_key) != current_layer.end()) {
				   continue;
			   }

			double child_g = beams[beams.size() - 1][i].g_value +
				env->GCost(beams[beams.size() - 1][i].my_state, children[j]);
			double child_h;

			if(env->IsGoalStored()) {
				child_h = env->HCost(children[j]);
			}
			else {
				child_h = env->HCost(children[j], goal);
			}

			double child_cost = get_cost(child_g, child_h);

			// construct new BeamNode
			BeamNode<state> new_node(children[j], child_g, child_h, child_cost, i, child_key);
			successors.push_back(new_node); // add to list of successors

			nodes_checked++;

			// if hit nodes checked limit
			if(bound_checked && nodes_checked >= checked_limit) {
				return CHECKED_MET;
			}

			// if have found a goal
			if(env->GoalTest(children[j], goal)) {
				successors.clear();
				successors.push_back(new_node);
				best_path_cost = child_g;
				return 1;
			}

			// label node as stored in current layer
			current_layer[child_key] = 0;
		}
	}

	// sorts children
	sort(successors.begin(), successors.end());
	if(debug) {
		printf("Successors: \n");
		for(unsigned i = 0; i < successors.size(); i++) {
			std::cout << successors[i].my_state;
			printf(" G:%.0f H:%.0f Cost:%.0f\n", successors[i].g_value, successors[i].h_value, successors[i].cost);
		}
	}
	return 0; // no goal found and no limits hit
}
#endif
