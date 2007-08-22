/*
 * $Id: Graph.h,v 1.9 2006/09/18 06:20:15 nathanst Exp $
 *
 * This file is part of HOG.
 *
 * HOG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * HOG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with HOG; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/ 

// HOG File

#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <list>
#include <iostream>

#define MAXINT (1<<30)
//#define MAXLABELS 15

class Graph;
class node;
class edge;

typedef std::vector<edge *>::const_iterator edge_iterator;
typedef std::vector<node *>::const_iterator node_iterator;
typedef unsigned int neighbor_iterator;

typedef union { double fval; long lval; } labelValue;

/**
 * Parent class for nodes and edges allowing them to be stored in a Heap or
 * manipulated with other data structures.
 */
class graph_object {
public:
  graph_object():key(0) {}
  virtual ~graph_object() {}
  virtual double GetKey() { return 0; }
  virtual void Print(std::ostream&) const;
  virtual graph_object *clone() const = 0;
  unsigned int key; // for use by a data structure to maintain a reverse-lookup
  // to go from an object to a table key in constant time.
private:
  //	double val;
};

std::ostream& operator <<(std::ostream & out, const graph_object &_Obj);

/**
 * A generic Graph class.
 */

class Graph : public graph_object {
public:
  Graph();
  ~Graph();
  graph_object *clone() const; // clones just the nodes
  Graph *cloneAll() const;     // clones everything

  int AddNode(node *);
  node *GetNode(unsigned int num);
  void AddEdge(edge *);
  edge *findDirectedEdge(unsigned int from, unsigned int to);
  edge *FindEdge(unsigned int from, unsigned int to);
	
  bool relax(edge *e, int weightIndex);
  bool relaxReverseEdge(edge *e, int weightIndex);
	
  node *getRandomNode();
  edge *getRandomEdge();

  node_iterator getNodeIter() const;
  node *nodeIterNext(node_iterator&) const;
  edge_iterator getEdgeIter() const;
  edge *edgeIterNext(edge_iterator&) const;

  void RemoveEdge(edge *);
  // returns the node that had it's node number changed along with its previous node number, if any
  node *RemoveNode(node *, unsigned int&);
  void RemoveNode(node *n) { unsigned int x; RemoveNode(n, x); } // if you don't care about node #'s
  void RemoveNode(unsigned int nodeNum) { RemoveNode(GetNode(nodeNum)); }
	
  int getNumEdges();
  int getNumNodes();
  
  std::vector<node*>* getReachableNodes(node* start);
  
  bool verifyGraph() const;
  void Print(std::ostream&) const;
	void printStats();
private:
  std::vector<node *> _nodes;
  //unsigned int node_index;
  std::vector<edge *> _edges;
  //unsigned int edge_index;
};

// Moved to GraphAbstraction.h
// enum {
// 	kEdgeWeight = 0,
// 	kEdgeWidth = 1
// };

/**
 * Edge class for connections between \ref node in a \ref Graph.
 */
class edge : public graph_object {
 public:
	edge(unsigned int, unsigned int, double);
	graph_object *clone() const { return new edge(from, to, GetLabelF(kEdgeWeight)); }

	// set/get various labels for each node
	void SetLabelF(unsigned int index, double val);
	void SetLabelL(unsigned int index, long val);
	inline double GetLabelF(unsigned int index) const { if (index < label.size()) return label[index].fval; return MAXINT; }
	inline long GetLabelL(unsigned int index) const { if (index < label.size()) return label[index].lval; return MAXINT; }

	double getWeight() { return GetLabelF(kEdgeWeight); }
	void setWeight(double val) { SetLabelF(kEdgeWeight, val); }
	
//	double getWidth() { return GetLabelF(kEdgeWidth); }
//	void setWidth(double val) { SetLabelF(kEdgeWidth, val); }
	
	unsigned int getFrom() { return from; }
	unsigned int getTo() { return to; }
	
	void setMarked(bool marked) { mark = marked; }
	bool getMarked() { return mark; }

	int getEdgeNum() const { return edgeNum; } 

	void Print(std::ostream&) const;
 private:
	friend class Graph;
	bool mark;
	unsigned int from, to;
//	double weight;
//	double width;
	unsigned int edgeNum;//, label[MAXLABELS];

	std::vector<labelValue> label;
};

/**
 * Nodes to be stored within a \ref Graph.
 */

class node : public graph_object {
public:
  node(const char *);
  graph_object *clone() const;

  const char *GetName() const { return name; }
  unsigned int GetNum() const { return nodeNum; }
  int getUniqueID() const { return uniqueID; }
  void AddEdge(edge *);
  void RemoveEdge(edge *);

  // iterator over incoming edges
  edge *edgeIterNextIncoming(edge_iterator&) const;
  edge_iterator getIncomingEdgeIter() const;
  // iterator over outcoming edges
  edge *edgeIterNextOutgoing(edge_iterator&) const;
  edge_iterator getOutgoingEdgeIter() const;
  // iterate over all edges
  edge *edgeIterNext(edge_iterator&) const;
  edge_iterator getEdgeIter() const;

	edge *getEdge(unsigned int which);
	
  // iterate over all neighbors
  // don't return node* because we don't have access to them internally
  neighbor_iterator getNeighborIter() const;
  int nodeNeighborNext(neighbor_iterator&) const;

  edge *getRandomIncomingEdge();
  edge *getRandomOutgoingEdge();
	edge *getRandomEdge();
	
  int getNumOutgoingEdges();
  int getNumIncomingEdges();
  int getNumEdges() { return getNumOutgoingEdges()+getNumIncomingEdges(); }
	
  // chooses which label will be used as the key for
  // priority queue
  void setKeyLabel(int which) { keyLabel = which; }
  double GetKey() { return label[keyLabel].fval; }

  // set/get various labels for each node
  void SetLabelF(unsigned int index, double val);
  void SetLabelL(unsigned int index, long val);
  inline double GetLabelF(unsigned int index) {
    if (index < label.size()) return label[index].fval; return MAXINT;
  }
  inline long GetLabelL(unsigned int index) {
    if (index < label.size()) return label[index].lval; return MAXINT;
  }
	
	
  // set/get marked edge for each node (limit 1)
  void markEdge(edge *e) { markedEdge = e; }
  edge *getMarkedEdge() { return markedEdge; }
	
  double getWidth() { return width; }
  void setWidth(double val) { width = val; }

  void Print(std::ostream&) const;
private:
  friend class Graph;
  unsigned int nodeNum;//, label[MAXLABELS];
  std::vector<labelValue> label;
  edge *markedEdge;
  std::vector<edge *> _edgesOutgoing;
  std::vector<edge *> _edgesIncoming;
  std::vector<edge *> _allEdges;
  char name[30];
  int keyLabel;
  double width;
  int uniqueID;
  static unsigned int uniqueIDCounter;
};

std::ostream& operator <<(std::ostream & out, const Graph &_Graph);
std::ostream& operator <<(std::ostream & out, const node &_Node);
std::ostream& operator <<(std::ostream & out, const edge &_Edge);

#endif
