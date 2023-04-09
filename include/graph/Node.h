#pragma once

#include <any>
#include <map>
#include <ostream>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace Game3 {
	class Graph;

	class Node {
		friend class Graph;

		public:
			struct Node_less {
				bool operator()(Node *left, Node *right) const {
					return left->index() < right->index();
				}
			};

			using Set = std::unordered_set<Node *>;

		private:
			Graph *owner;
			std::string label;
			Set out;
			Set in;
			int index_ = -1;
			std::unordered_map<Node *, bool> reachability;

		public:
			using USet = std::unordered_set<Node *>;
			using Map = std::map<Node *, Set, Node_less>;

			std::any data;
			std::unordered_set<int> colors;
			int colorsNeeded = 1;

			Node() = delete;
			Node(Node &&) = delete;
			Node(const Node &) = delete;
			Node & operator=(Node &&) = delete;
			Node & operator=(const Node &) = delete;

			/** Creates a node with a parent graph and a given label. */
			Node(Graph *, std::string);

			template <typename T>
			const T & get() const {
				return *std::any_cast<T>(&data);
			}

			template <typename T>
			T & get() {
				return *std::any_cast<T>(&data);
			}

			Node & setColorsNeeded(int);

			/** Returns a const reference to the node's label. */
			const std::string & getLabel() const;

			/** Changes the node's label. */
			Node & rename(const std::string &);

			/** Returns whether the node is connected to itself. */
			bool reflexive() const;

			/** Adds an edge (unidirectional by default) to another node. Returns true if the edge already existed. */
			bool link(Node &, bool bidirectional = false);
			/** Adds an edge (unidirectional by default) to another node. Returns true if the edge already existed. */
			bool link(Node *, bool bidirectional = false);

			/** Removes a edge (and optionally the opposite edge) between this node and another.
			 *  Returns true if the edge existed and was removed. */
			bool unlink(Node &, bool bidirectional = false);
			/** Removes a edge (and optionally the opposite edge) between this node and another.
			 *  Returns true if the edge existed and was removed. */
			bool unlink(Node *, bool bidirectional = false);
			/** Removes all edges from this node. */
			void unlink();

			/** Returns whether the node lacks any neighbors. */
			bool isolated() const;

			/** Should be called whenever a node's index in the parent graph changes. */
			void dirty();

			/** Returns the node's index in the parent graph. */
			int index();

			/** Returns a const set of the node's outward edges. */
			const Set & getOut() const;

			/** Returns a const set of the node's inward edges. */
			const Set & getIn() const;

			/** Returns a set of the node's outward and inward edges. */
			Set allEdges() const;

			/** Returns whether there exists a path from this node to another. */
			bool canReach(Node &);
			void clearReachability();

			/** Returns the number of edges connected to this node. */
			size_t degree() const;

			/** If the node has only one inward edge, this function returns the other node. Otherwise, it throws an
			 *  exception. */
			Node * parent() const;

			/** Adds a neighbor. */
			Node & operator+=(Node &);
			/** Adds a neighbor. */
			Node & operator+=(Node *);

			/** Removes a neighbor. */
			Node & operator-=(Node &);
			/** Removes a neighbor. */
			Node & operator-=(Node *);
			/** Removes a neighbor with a given label. */
			Node & operator-=(const std::string &);

			decltype(out)::iterator begin();
			decltype(out)::iterator end();

			decltype(in)::iterator ibegin();
			decltype(in)::iterator iend();
	};

	std::ostream & operator<<(std::ostream &, const Node &);
}
