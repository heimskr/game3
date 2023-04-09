#pragma once

#include <functional>
#include <list>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "graph/DFSResult.h"
#include "graph/Node.h"

namespace Game3 {
	class Graph {
		public:
			using Label = std::string;

		private:
			std::list<Node *> nodes;
			std::unordered_map<std::string, Node *> labelMap;

			void bridgeTraverse(const Node &node, std::unordered_map<const Node *, bool> &visited,
			                    std::unordered_map<const Node *, size_t> &discovered,
			                    std::unordered_map<const Node *, size_t> &low,
			                    std::unordered_map<const Node *, const Node *> &parent,
			                    std::vector<std::pair<Label, Label>> &out) const;

		public:
			enum class ColoringAlgorithm {Bad, Greedy};

			std::string name = "Graph";
			std::vector<std::string> colors = {"red", "orange", "yellow", "green", "blue", "purple"};

			/** Constructs a graph with no nodes. */
			Graph();

			Graph(const Graph &);

			Graph(Graph &&);

			Graph & operator=(const Graph &);

			Graph & operator=(Graph &&);

			/** Constructs a graph with a name and no nodes. */
			Graph(std::string name_);

			/** Constructs a graph with a number n of nodes with labels 0, 1, ..., n. */
			Graph(size_t);

			/** Constructs a graph with nodes with given labels. */
			Graph(std::initializer_list<Label>);

			virtual ~Graph();

			/** Clears the graph and frees up all node resources. */
			void clear();

			/** Returns whether the graph contains a node with a given label. */
			bool hasLabel(const Label &) const;

			/** Returns the number of nodes in the graph. */
			size_t size() const;

			/** Returns whether the graph is empty. */
			bool empty() const;

			/** Returns a constant reference to the list of nodes. */
			const std::list<Node *> & getNodes() const;

			/** Returns the node at a given index. Throws an exception if no node exists at the index. */
			Node & operator[](size_t) const;
			/** Returns the node with a given label. Throws an exception if no such node exists. */
			Node & operator[](const Label &) const;
			/** Returns the node that has the same label as a given node from another graph. Throws an exception if no
			 *  such node exists. */
			Node & operator[](const Node &) const;

			/** Adds a node with a given label. */
			Graph & operator+=(const Label &);

			/** Adds a premade node to the graph. Note that this doesn't check for label collisions. */
			Graph & operator+=(Node *);

			/** Removes and deletes a node. */
			Graph & operator-=(Node &);
			/** Removes and deletes a node. */
			Graph & operator-=(Node *);
			/** Removes and deletes a node with a given label. */
			Graph & operator-=(const Label &);

			/** Adds a node with a given label. */
			Node & addNode(const Label &);

			/** Adds a premade node to the graph. Note that this function doesn't check for label collisions. */
			Node & addNode(Node *);

			/** Assigns a new label to a node with a given label and returns the node. */
			Node & rename(const Label &, const Label &);
			/** Assigns a new label to a node and returns the node. */
			Node & rename(Node &, const Label &);
			/** Assigns a new label to a node and returns the node. */
			Node & rename(Node *, const Label &);

			/** Connects two nodes with given labels (unidirectionally by default). */
			void link(const Label &, const Label &, bool bidirectional = false);

			/** Removes any connection between two nodes with given labels (and optionally the inverse edge too). */
			void unlink(const Label &, const Label &, bool bidirectional = false);
			/** Removes all edges in the graph. */
			void unlink();

			/** Clones the graph into another graph. */
			void cloneTo(Graph &, std::unordered_map<Node *, Node *> *rename_map = nullptr);

			/** Returns a clone of the graph. */
			Graph clone(std::unordered_map<Node *, Node *> *rename_map = nullptr);

			/** Takes a space-separated list of colon-separated pairs of labels and links each pair of nodes. */
			void addEdges(const Label &);

			/** Removes all nodes from the graph. */
			void reset();

			/** Attempts to find the first node matching a predicate function. */
			Node * find(std::function<bool(Node &)>);

			/** Runs a depth-first search at a given start node. */
			DFSResult DFS(Node *) const;
			/** Runs a depth-first search at a given start node. */
			DFSResult DFS(Node &) const;
			/** Runs a depth-first search at a given start node. */
			DFSResult DFS(const Label &) const;

			/** Returns a vector of nodes in level (breadth-first) order. */
			std::vector<Node *> BFS(Node &) const;
			/** Returns a vector of nodes in level (breadth-first) order. */
			std::vector<Node *> BFS(const Label &) const;

			/** Returns a set of nodes connected to a node. */
			std::unordered_set<Node *> undirectedSearch(Node &) const;
			/** Returns a set of nodes connected to a node. */
			std::unordered_set<Node *> undirectedSearch(const Label &) const;

			/** Returns a postorder list of nodes. */
			std::vector<Node *> postOrder(Node &) const;
			/** Returns a reverse-postorder list of nodes. */
			std::vector<Node *> reversePostOrder(Node &) const;

			/** Finds all bridges in the graph. Assumes the graph is connected. */
			std::vector<std::pair<Label, Label>> bridges() const;

			/** Returns a list of the graph's connected components. */
			std::list<Graph> components() const;

			/** Returns a map of nodes to sets of their predecessors. */
			std::unordered_map<Node *, std::unordered_set<Node *>> predecessors() const;

			/** Colors all the nodes in the graph according to a given coloring algorithm.
			 *  Assumes all edges are bidirectional. */
			void color(ColoringAlgorithm, int color_min = -1, int color_max = -1);

			/** Returns a vectors of all edges represented as a pair of the start node and the end node. */
			std::vector<std::pair<Node *, Node *>> allEdges() const;

			std::vector<Node *> topoSort();

			/** Returns a representation of the graph in graphviz dot syntax. */
			virtual std::string toDot(const std::string &direction = "TB");

			/** Renders a representation (PNG by default; changeable by changing the file extension) of the graph to an
			 *  output file. */
			void renderTo(std::string png_path, const std::string &direction = "TB");

			decltype(labelMap)::iterator begin();
			decltype(labelMap)::iterator end();

			decltype(labelMap)::const_iterator begin() const;
			decltype(labelMap)::const_iterator end() const;
	};
}
