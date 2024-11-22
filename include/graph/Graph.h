#pragma once

#include "graph/UncolorableError.h"

#include <cassert>
#include <fstream>
#include <functional>
#include <list>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace Game3 {
	class Node;

	template <typename T, typename Color = int>
	class Graph {
		public:
			using Index = ssize_t;
			using Label = std::string;

			class Node {
			friend class Graph;

				public:
					struct NodeLess {
						bool operator()(Node *left, Node *right) const {
							assert(left != nullptr);
							assert(right != nullptr);
							return left->getIndex() < right->getIndex();
						}
					};

					using Set = std::unordered_set<Node *>;

				private:
					Graph *owner;
					std::string label;
					Set out;
					Set in;
					Index index = -1;
					std::unordered_map<Node *, bool> reachability;

				public:
					using USet = std::unordered_set<Node *>;
					using Map  = std::map<Node *, Set, NodeLess>;

					T data{};
					std::unordered_set<Color> colors;
					Color colorsNeeded = 1;

					Node() = delete;
					Node(Node &&) = delete;
					Node(const Node &) = delete;
					Node & operator=(Node &&) = delete;
					Node & operator=(const Node &) = delete;

					/** Creates a node with a parent graph and a given label. */
					Node(Graph &owner_, Label label_):
						owner(&owner_), label(std::move(label_)) {}

					/** Returns a const reference to the node's label. */
					const auto & getLabel() const {
						return label;
					}

					/** Changes the node's label. */
					void rename(const auto &new_label) {
						owner->rename(*this, new_label);
					}

					/** Returns whether the node is connected to itself. */
					bool reflexive() const {
						return out.contains(const_cast<Node *>(this));
					}

					/** Adds an edge (unidirectional by default) to another node. Returns true if the edge already existed. */
					bool link(Node &other, bool bidirectional = false) {
						const bool already_linked = out.contains(&other);
						if (!already_linked) {
							out.insert(&other);
							other.in.insert(this);
						}

						if (bidirectional && &other != this)
							other.link(*this);

						return already_linked;
					}

					/** Removes a edge (and optionally the opposite edge) between this node and another.
					 *  Returns true if the edge existed and was removed. */
					bool unlink(Node &other, bool bidirectional = false) {
						const auto iter = out.find(&other);
						const bool exists = iter != out.end();
						if (exists)
							out.erase(iter);

						other.in.erase(this);
						reachability.erase(&other);

						if (bidirectional && &other != this)
							other.unlink(*this, false);

						return exists;
					}

					/** Removes all edges from this node. */
					void unlink() {
						for (Node *other: out)
							other->in.erase(this);
						out.clear();
					}

					/** Returns whether the node lacks any neighbors. */
					bool isIsolated() const {
						return out.empty();
					}

					/** Should be called whenever a node's index in the parent graph changes. */
					void markDirty() {
						index = -1;
					}

					/** Returns the node's index in the parent graph. */
					int getIndex() {
						if (index != -1)
							return index;

						for (const Node *node: owner->getNodes()) {
							++index;
							if (node == this)
								return index;
						}

						throw std::runtime_error("Node not found in parent graph");
					}

					/** Returns a const set of the node's outward edges. */
					const auto & getOut() const {
						return out;
					}

					/** Returns a const set of the node's inward edges. */
					const auto & getIn() const {
						return in;
					}

					/** Returns a set of the node's outward and inward edges. */
					Set allEdges() const {
						Set set = out;
						set.insert(in.begin(), in.end());
						return set;
					}

					/** Returns whether there exists a path from this node to another. */
					bool canReach(Node &other) {
						if (other.owner != owner)
							return false;

						if (auto iter = reachability.find(&other); iter != reachability.end())
							return *iter;

						std::unordered_set<Node *> visited;
						std::list<Node *> queue{this};
						while (!queue.empty()) {
							Node *node = queue.front();
							queue.pop_front();
							for (Node *outnode: node->out) {
								if (outnode == &other) {
									reachability.emplace(&other, true);
									return true;
								}

								if (!visited.contains(outnode)) {
									visited.insert(outnode);
									queue.push_back(outnode);
								}
							}
						}

						reachability.emplace(&other, false);
						return false;
					}

					void clearReachability() {
						reachability.clear();
					}

					/** Returns the number of edges connected to this node. */
					size_t degree() const {
						size_t deg = out.size();
						for (Node *neighbor: in)
							if (!out.contains(neighbor))
								++deg;
						return deg;
					}

					/** If the node has only one inward edge, this function returns the other node. Otherwise, it throws an exception. */
					Node * parent() const {
						if (in.size() != 1)
							throw std::runtime_error("Cannot find parent of node with " + std::to_string(in.size()) + " inward edges");
						return *in.begin();
					}

					/** Adds a neighbor. */
					Node & operator+=(Node &neighbor) {
						out.insert(&neighbor);
						return *this;
					}

					/** Removes a neighbor. */
					Node & operator-=(Node &neighbor) {
						out.erase(&neighbor);
						in.erase(&neighbor);
						return *this;
					}

					/** Removes a neighbor with a given label. */
					Node & operator-=(const std::string &label) {
						for (Node *node: out)
							if (node->label == label)
								return *this -= *node;
						throw std::out_of_range("Can't remove: no neighbor with label \"" + label + "\" found");
					}

					typename decltype(out)::iterator begin() {
						return out.begin();
					}

					typename decltype(out)::iterator end() {
						return out.end();
					}

					typename decltype(in)::iterator ibegin() {
						return in.begin();
					}

					typename decltype(in)::iterator iend() {
						return in.end();
					}
			};

			struct DFSResult {
				using ParentMap = std::unordered_map<Node *, Node *>;
				using TimeMap   = std::unordered_map<Node *, int>;

				const Graph *graph;
				ParentMap parents;
				TimeMap discovered, finished;

				DFSResult(const Graph &graph_, ParentMap parents_, TimeMap discovered_, TimeMap finished_):
					graph(&graph_), parents(std::move(parents_)), discovered(std::move(discovered_)), finished(std::move(finished_)) {}

				DFSResult(const Graph &graph_, const std::vector<Node *> &parents_, const std::vector<int> &discovered_, const std::vector<int> &finished_):
				graph(&graph_) {
					for (int i = 0, len = parents_.size(); i < len; ++i)
						parents[&(*graph)[i]] = parents_[i];
					for (int i = 0, len = discovered_.size(); i < len; ++i)
						discovered[&(*graph)[i]] = discovered_[i];
					for (int i = 0, len = finished_.size(); i < len; ++i)
						finished[&(*graph)[i]] = finished_[i];
				}
			};

		private:
			std::list<Node *> nodes;
			std::unordered_map<std::string, Node *> labelMap;

			void bridgeTraverse(const Node &node, std::unordered_map<const Node *, bool> &visited,
			                    std::unordered_map<const Node *, size_t> &discovered,
			                    std::unordered_map<const Node *, size_t> &low,
			                    std::unordered_map<const Node *, const Node *> &parents,
			                    std::vector<std::pair<Label, Label>> &out) const {
				static size_t time = 0;
				visited[&node] = true;
				discovered[&node] = low[&node] = ++time;
				for (const Node *adjacent: node.allEdges()) {
					if (!visited[adjacent]) {
						parents[adjacent] = &node;
						bridgeTraverse(*adjacent, visited, discovered, low, parents, out);
						if (low[adjacent] < low[&node])
							low[&node] = low[adjacent];
						if (discovered[&node] < low[adjacent])
							out.emplace_back(node.getLabel(), adjacent->getLabel());
					} else if (adjacent != parents[&node] && discovered[adjacent] < low[&node])
						low[&node] = discovered[adjacent];
				}
			}

		public:
			enum class ColoringAlgorithm {Bad, Greedy};

			std::string name = "Graph";
			std::vector<std::string> colors = {"red", "orange", "yellow", "green", "blue", "purple"};

			/** Constructs a graph with no nodes. */
			Graph() = default;

			/** Constructs a graph with a name and no nodes. */
			Graph(std::string name_):
				name(std::move(name_)) {}

			/** Constructs a graph with a number n of nodes with labels 0, 1, ..., n. */
			Graph(size_t node_count) {
				for (size_t i = 0; i < node_count; ++i)
					*this += std::to_string(i);
			}

			/** Constructs a graph with nodes with given labels. */
			Graph(std::initializer_list<Label> labels) {
				for (const Label &label: labels)
					*this += label;
			}

			Graph(const Graph &other) {
				for (const auto &[label, node]: other)
					addNode(label);
				for (const auto &[label, node]: other) {
					for (Node *in: node->in)
						link(in->label, node->label);
					for (Node *out: node->out)
						link(node->label, out->label);
				}
			}

			Graph(Graph &&other) noexcept {
				nodes = std::move(other.nodes);
				labelMap = std::move(other.labelMap);
				name = std::move(other.name);
				colors = std::move(other.colors);
				for (Node *node: nodes)
					node->owner = this;
			}

			Graph & operator=(const Graph &other) {
				clear();
				for (const auto &[label, node]: other)
					addNode(label);
				for (const auto &[label, node]: other) {
					for (const Node *in: node->in)
						link(in->label, node->label);
					for (const Node *out: node->out)
						link(node->label, out->label);
				}
				return *this;
			}

			Graph & operator=(Graph &&other) noexcept {
				clear();
				nodes = std::move(other.nodes);
				labelMap = std::move(other.labelMap);
				name = std::move(other.name);
				colors = std::move(other.colors);
				for (Node *node: nodes)
					node->owner = this;
				return *this;
			}

			virtual ~Graph() {
				for (Node *node: nodes)
					delete node;
			}

			/** Clears the graph and frees up all node resources. */
			void clear() {
				labelMap.clear();
				for (Node *node: nodes)
					delete node;
				nodes.clear();
			}

			/** Returns whether the graph contains a node with a given label. */
			inline bool hasLabel(const Label &label) const {
				return labelMap.contains(label);
			}

			/** Returns the number of nodes in the graph. */
			inline size_t size() const {
				return nodes.size();
			}

			/** Returns whether the graph is empty. */
			inline bool empty() const {
				return nodes.empty();
			}

			/** Returns a constant reference to the list of nodes. */
			const auto & getNodes() const {
				return nodes;
			}

			/** Returns the node at a given index. Throws an exception if no node exists at the index. */
			Node & operator[](size_t index) const {
				if (nodes.size() <= index)
					throw std::out_of_range("Invalid node index: " + std::to_string(index));
				Node *node = *std::next(nodes.begin(), index);
				if (!node)
					throw std::runtime_error("Node at index " + std::to_string(index) + " is null");
				return *node;
			}

			/** Returns the node with a given label. Creates a node if no such node exists. */
			Node & operator[](const Label &label) {
				if (auto iter = labelMap.find(label); iter != labelMap.end())
					return *iter->second;
				return addNode(label);
			}

			Node & get(const Label &label, bool &created) {
				if (auto iter = labelMap.find(label); iter != labelMap.end()) {
					created = false;
					return *iter->second;
				}

				created = true;
				return addNode(label);
			}

			Node * maybe(const Label &label) {
				if (auto iter = labelMap.find(label); iter != labelMap.end())
					return iter->second;
				return nullptr;
			}

			/** Returns the node that has the same label as a given node from another graph. Throws an exception if no such node exists. */
			Node & operator[](const Node &node) const {
				return (*this)[node.getLabel()];
			}

			/** Adds a node with a given label. */
			Graph & operator+=(const Label &label) {
				addNode(label);
				return *this;
			}

			/** Adds a premade node to the graph. Note that this doesn't check for label collisions. */
			Graph & operator+=(Node &node) {
				addNode(node);
				return *this;
			}

			/** Removes and deletes a node. */
			Graph & operator-=(Node &to_remove) {
				auto iter = std::find(nodes.begin(), nodes.end(), &to_remove);

				if (iter == nodes.end())
					throw std::out_of_range("Can't remove: node is not in graph");

				for (Node *node: nodes) {
					node->unlink(to_remove, true);
					node->markDirty();
				}

				nodes.erase(iter);
				labelMap.erase(to_remove.label);
				delete &to_remove;
				return *this;
			}

			/** Removes and deletes a node with a given label. */
			Graph & operator-=(const Label &label) {
				for (Node *node: nodes)
					if (node->getLabel() == label)
						return *this -= *node;
				throw std::out_of_range("Can't remove: no node with label \"" + label + "\" found");
			}

			/** Adds a node with a given label. */
			Node & addNode(const Label &label) {
				if (hasLabel(label))
					throw std::runtime_error("Can't add: a node with label \"" + label + "\" already exists");
				Node *node = new Node(*this, label);
				labelMap.insert({label, node});
				nodes.push_back(node);
				return *node;
			}

			/** Adds a premade node to the graph. Note that this function doesn't check for label collisions. */
			void addNode(Node &node) {
				labelMap.emplace(node.getLabel(), &node);
				nodes.push_back(&node);
			}

			/** Assigns a new label to a node with a given label and returns the node. */
			void rename(const Label &old_label, const Label &new_label) {
				rename((*this)[old_label], new_label);
			}

			/** Assigns a new label to a node and returns the node. */
			void rename(Node &node, const Label &new_label) {
				if (node.getLabel() == new_label)
					return;
				if (hasLabel(new_label))
					throw std::runtime_error("Can't rename: a node with label \"" + new_label + "\" already exists");
				labelMap.erase(node.getLabel());
				node.label = new_label;
				labelMap.emplace(new_label, &node);
			}

			/** Connects two nodes with given labels (unidirectionally by default). */
			void link(const Label &from, const Label &to, bool bidirectional = false) {
				(*this)[from].link((*this)[to], bidirectional);
			}

			/** Removes any connection between two nodes with given labels (and optionally the inverse edge too). */
			void unlink(const Label &from, const Label &to, bool bidirectional = false) {
				(*this)[from].unlink((*this)[to], bidirectional);
			}

			/** Removes all edges in the graph. */
			void unlink() {
				for (auto &[label, node]: *this)
					node->unlink();
			}

			/** Clones the graph into another graph. */
			void cloneTo(Graph &out, std::unordered_map<Node *, Node *> *rename_map = nullptr) {
				out.reset();

				// Maps old nodes to new nodes.
				std::unordered_map<Node *, Node *> node_map;

				for (Node *node: nodes) {
					Node *new_node = new Node(&out, node->getLabel());
					node_map.insert({node, new_node});
					out.nodes.push_back(new_node);
					out.labelMap.insert({node->getLabel(), new_node});
				}

				for (const auto &[old_node, new_node]: node_map)
					for (Node *old_link: old_node->out)
						new_node->link(node_map.at(old_link), false);

				if (rename_map != nullptr)
					*rename_map = std::move(node_map);
			}

			/** Returns a clone of the graph. */
			Graph clone(std::unordered_map<Node *, Node *> *rename_map = nullptr) {
				Graph new_graph;
				cloneTo(new_graph, rename_map);
				return new_graph;
			}

			/** Takes a space-separated list of colon-separated pairs of labels and links each pair of nodes. */
			void addEdges(const std::string &pairs) {
				size_t last = 0;
				size_t space{};

				while (last != std::string::npos) {
					space = pairs.find(' ', last + 1);
					const std::string sub = pairs.substr(last? last + 1 : 0, space - (last + (last? 1 : 0)));
					const size_t colon = sub.find(':');
					const std::string from = sub.substr(0, colon);
					const std::string to = sub.substr(colon + 1);
					link(from, to, false);
					last = space;
				}
			}

			/** Removes all nodes from the graph. */
			void reset() {
				while (!nodes.empty())
					*this -= *nodes.front();
			}

			/** Attempts to find the first node matching a predicate function. */
			Node * find(const std::function<bool(Node &)> &predicate) {
				for (Node *node: nodes)
					if (predicate(*node))
						return node;
				return nullptr;
			}

			/** Runs a depth-first search at a given start node. */
			DFSResult DFS(Node &start) const {
				typename DFSResult::ParentMap parents;
				typename DFSResult::TimeMap discovered, finished;
				int time = 0;

				std::function<void(Node *)> visit = [&](Node *node) {
					discovered[node] = ++time;
					for (Node *out: node->out) {
						if (!discovered.contains(out)) {
							parents[out] = node;
							visit(out);
						}
					}
					finished[node] = ++time;
				};

				visit(&start);
				return {*this, parents, discovered, finished};
			}

			/** Runs a depth-first search at a given start node. */
			DFSResult DFS(const Label &start_label) const {
				return DFS((*this)[start_label]);
			}

			/** Returns a vector of nodes in level (breadth-first) order. */
			std::vector<Node *> BFS(Node &start) const {
				std::list<Node *> queue = {&start};
				std::unordered_set<Node *> visited;
				std::vector<Node *> order;
				order.reserve(size());

				while (!queue.empty()) {
					Node *next = queue.front();
					queue.pop_front();
					for (Node *out: next->out)
						if (!visited.contains(out)) {
							visited.insert(out);
							order.push_back(out);
							queue.push_back(out);
						}
				}

				return order;
			}

			/** Returns a vector of nodes in level (breadth-first) order. */
			std::vector<Node *> BFS(const Label &start_label) const {
				return BFS((*this)[start_label]);
			}

			/** Returns a set of nodes connected to a node. */
			std::unordered_set<Node *> undirectedSearch(Node &start) const {
				std::list<Node *> queue = {&start};
				std::unordered_set<Node *> visited;
				std::unordered_set<Node *> out;
				out.reserve(size());

				while (!queue.empty()) {
					Node *next = queue.front();
					queue.pop_front();
					for (const auto *set: {&next->in, &next->out})
						for (Node *adjacent: *set)
							if (!visited.contains(adjacent)) {
								visited.insert(adjacent);
								out.insert(adjacent);
								queue.push_back(adjacent);
							}
				}

				return out;
			}

			/** Returns a set of nodes connected to a node. */
			std::unordered_set<Node *> undirectedSearch(const Label &start_label) const {
				return undirectedSearch((*this)[start_label]);
			}

			/** Returns a postorder list of nodes. */
			std::vector<Node *> postOrder(Node &start) const {
				std::vector<Node *> out;
				out.reserve(size());
				std::unordered_set<Node *> visited;

				std::function<void(Node *)> visit = [&](Node *node) {
					visited.insert(node);
					out.push_back(node);
					for (Node *successor: node->out)
						if (!visited.contains(successor))
							visit(successor);
				};

				visit(&start);
				return out;
			}

			/** Returns a reverse-postorder list of nodes. */
			std::vector<Node *> reversePostOrder(Node &start) const {
				std::vector<Node *> post = postOrder(start);
				std::reverse(post.begin(), post.end());
				return post;
			}

			/** Finds all bridges in the graph. Assumes the graph is connected. */
			std::vector<std::pair<Label, Label>> bridges() const {
				std::vector<std::pair<Label, Label>> out;
				std::unordered_map<const Node *, bool> visited;
				std::unordered_map<const Node *, size_t> discovered;
				std::unordered_map<const Node *, size_t> low;
				std::unordered_map<const Node *, const Node *> parent;
				bridgeTraverse(*nodes.front(), visited, discovered, low, parent, out);
				return out;
			}

			/** Returns a list of the graph's connected components. */
			std::list<Graph> components() const {
				std::list<Graph> out_list;
				std::unordered_set<Node *> remaining(nodes.begin(), nodes.end());

				while (!remaining.empty()) {
					Graph component_graph;
					Node *front = *remaining.begin();
					remaining.erase(front);
					std::unordered_set<Node *> component_nodes = undirectedSearch(*front);
					for (Node *node: component_nodes) {
						component_graph.addNode(node->label);
						remaining.erase(node);
					}
					if (!component_graph.hasLabel(front->label))
						component_graph.addNode(front->label);
					for (Node *node: component_nodes) {
						for (const Node *in: node->in)
							component_graph.link(in->label, node->label);
						for (const Node *out: node->out)
							component_graph.link(node->label, out->label);
					}
					out_list.push_back(std::move(component_graph));
				}

				return out_list;
			}

			/** Returns a map of nodes to sets of their predecessors. */
			std::unordered_map<Node *, std::unordered_set<Node *>> predecessors() const {
				std::unordered_map<Node *, std::unordered_set<Node *>> out;
				for (Node *node: nodes)
					for (Node *successor: node->out)
						out[successor].insert(node);
				return out;
			}

			/** Colors all the nodes in the graph according to a given coloring algorithm.
			 *  Assumes all edges are bidirectional. */
			void color(ColoringAlgorithm algorithm, int color_min = -1, int color_max = -1) {
				const int total_colors = color_max != -1? color_max - color_min + 1 : -1;
				if (algorithm == Graph::ColoringAlgorithm::Bad) {
					if (color_max != -1 && total_colors < static_cast<int>(nodes.size()))
						throw UncolorableError();
					int color = color_min - 1;
					for (Node *node: nodes) {
						node->colors.clear();
						for (int i = 0; i < node->colorsNeeded; ++i)
							node->colors.insert(++color);
					}
				} else if (algorithm == Graph::ColoringAlgorithm::Greedy) {
					std::set<int> all_colors;
					const int max = color_max == -1? static_cast<int>(color_min + size() - 1) : color_max;
					for (int i = color_min; i <= max; ++i)
						all_colors.insert(i);

					for (Node *node: nodes) {
						std::set<int> available = all_colors;
						for (Node *neighbor: node->out)
							for (const int color: neighbor->colors)
								available.erase(color);
						for (Node *neighbor: node->in)
							for (const int color: neighbor->colors)
								available.erase(color);
						if (available.size() < static_cast<size_t>(node->colorsNeeded)) {
							// error() << available.size() << " < " << static_cast<size_t>(node->colorsNeeded) << "\n";
							throw UncolorableError();
						}
						auto iter = available.begin();
						for (int i = 0; i < node->colorsNeeded; ++i)
							node->colors.insert(*iter++);
					}
				} else {
					throw std::invalid_argument("Unknown graph coloring algorithm: " + std::to_string(static_cast<int>(algorithm)));
				}
			}

			/** Returns a vectors of all edges represented as a pair of the start node and the end node. */
			std::vector<std::pair<Node *, Node *>> allEdges() const {
				std::vector<std::pair<Node *, Node *>> out;
				for (Node *node: nodes)
					for (Node *successor: *node)
						out.push_back({node, successor});
				return out;
			}

			std::vector<Node *> reverseTopoSort() {
				if (empty())
					return {};

				std::vector<Node *> out;
				out.reserve(size());

				std::unordered_set<Node *> permanent;
				std::unordered_set<Node *> temporary;
				std::unordered_set<Node *> nonpermanent(nodes.begin(), nodes.end());

				std::function<void(Node *)> visit = [&](Node *node) {
					if (permanent.contains(node))
						return;
					if (temporary.contains(node))
						throw std::runtime_error("Can't topologically sort a cyclic graph");
					temporary.insert(node);
					for (Node *out_node: node->out)
						visit(out_node);
					temporary.erase(node);
					permanent.insert(node);
					nonpermanent.erase(node);
					out.push_back(node);
				};

				while (!nonpermanent.empty())
					visit(*nonpermanent.begin());

				return out;
			}

			std::vector<Node *> topoSort() {
				auto out = reverseTopoSort();
				std::reverse(out.begin(), out.end());
				return out;
			}

			/** Returns a representation of the graph in graphviz dot syntax. */
			virtual std::string toDot(const std::string &direction = "TB") {
				std::list<Node *> reflexives;
				for (Node *node: nodes) {
					// node->rename("\"" + node->getLabel() + "_i" + std::to_string(node->in.size()) + "_o" +
					// 	std::to_string(node->out.size()) + "\"");
					if (node->reflexive())
						reflexives.push_back(node);
				}

				std::ostringstream out;
				out << "digraph rendered_graph {\n";
				out << "graph [fontname = \"helvetica\"];\n";
				out << "node [fontname = \"helvetica\", style = \"filled\"];\n";
				out << "edge [fontname = \"helvetica\", arrowhead=open, arrowsize=0.666];\n";
				out << "\trankdir=" << direction << ";\n";
				if (!reflexives.empty()) {
					out << "\tnode [shape = doublecircle];";
					for (Node *node: reflexives)
						out << " " << node->getLabel();
					out << ";\n";
				}

				out << "\tnode [shape = circle];";
				bool any_added = false;
				for (Node *node: nodes)
					if (node->isIsolated()) {
						out << " " << node->getLabel();
						any_added = true;
					}

				if (any_added)
					out << ";";
				out << "\n";

				for (const Node *node: nodes)
					if (node->colors.size() == 1 && static_cast<size_t>(*node->colors.begin()) < colors.size())
						out << "\t\"" << node->getLabel() << "\" [fillcolor=" << colors.at(*node->colors.begin()) << "];\n";

				for (const Node *node: nodes)
					for (const Node *neighbor: node->out)
						if (neighbor != node)
							out << "\t\"" << node->getLabel() << "\" -> \"" << neighbor->getLabel() << "\";\n";
				out << "}\n";
				return out.str();
			}

			/** Renders a representation (PNG by default; changeable by changing the file extension) of the graph to an output file. */
			void renderTo(std::string out_path, const std::string &direction = "TB") {
				std::ofstream out;
				std::string path = "/tmp/ll2w_graph_";
				for (const char ch: out_path)
					if (std::isdigit(ch) || std::isalpha(ch) || ch == '_')
						path += ch;
				path += ".dot";
				out.open(path);
				out << toDot(direction);
				out.close();

				if (std::string_view(out_path).substr(0, 2) == "./")
					out_path = (std::filesystem::current_path() / out_path.substr(2)).string();

				std::string type = "png";
				const size_t pos = out_path.find_last_of('.');
				if (pos != std::string::npos && pos != out_path.size() - 1) {
					type = out_path.substr(pos + 1);
					for (const char ch: type)
						if (!std::isalpha(ch)) {
							type = "png";
							break;
						}
				}

				type.insert(0, "-T");
				const char *typearg = type.c_str();

				if (fork() == 0) {
					if (4096 <= allEdges().size())
						execlp("dot", "dot", typearg, path.c_str(), "-o", out_path.c_str(), nullptr);
					else
						execlp("sfdp", "sfdp", "-x", "-Goverlap=scale", typearg, path.c_str(), "-o", out_path.c_str(), nullptr);
				}
			}

			typename decltype(labelMap)::iterator begin() {
				return labelMap.begin();
			}

			typename decltype(labelMap)::iterator end() {
				return labelMap.end();
			}

			typename decltype(labelMap)::const_iterator begin() const {
				return labelMap.cbegin();
			}

			typename decltype(labelMap)::const_iterator end() const {
				return labelMap.cend();
			}
	};
}
