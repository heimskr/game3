#include <cassert>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <unistd.h>

#include "graph/Graph.h"
#include "graph/UncolorableError.h"
#include "util/Util.h"

namespace Game3 {
	Graph::Graph() {}

	Graph::Graph(const Graph &other) {
		for (const auto &[label, node]: other)
			addNode(label);
		for (const auto &[label, node]: other) {
			for (const Node *in: node->in)
				link(in->label, node->label);
			for (const Node *out: node->out)
				link(node->label, out->label);
		}
	}

	Graph::Graph(Graph &&other) {
		nodes = std::move(other.nodes);
		labelMap = std::move(other.labelMap);
		name = std::move(other.name);
		colors = std::move(other.colors);
		for (Node *node: nodes)
			node->owner = this;
	}

	Graph & Graph::operator=(const Graph &other) {
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

	Graph & Graph::operator=(Graph &&other) {
		clear();
		nodes = std::move(other.nodes);
		labelMap = std::move(other.labelMap);
		name = std::move(other.name);
		colors = std::move(other.colors);
		for (Node *node: nodes)
			node->owner = this;
		return *this;
	}

	Graph::Graph(std::string name_): name(std::move(name_)) {}

	Graph::Graph(size_t node_count) {
		for (size_t i = 0; i < node_count; ++i)
			*this += std::to_string(i);
	}

	Graph::Graph(std::initializer_list<std::string> labels) {
		for (const std::string &label: labels)
			*this += label;
	}

	Graph::~Graph() {
		// We could just call clear(), but all we need to do is delete the pointers.
		// The compiler-generated part of the destructor will handle the rest.
		for (Node *node: nodes)
			delete node;
	}

	void Graph::clear() {
		labelMap.clear();
		for (Node *node: nodes)
			delete node;
		nodes.clear();
	}

	bool Graph::hasLabel(const std::string &label) const {
		return labelMap.find(label) != labelMap.end();
	}

	size_t Graph::size() const {
		return nodes.size();
	}

	bool Graph::empty() const {
		return nodes.empty();
	}

	const std::list<Node *> & Graph::getNodes() const {
		return nodes;
	}

	Node & Graph::operator[](size_t index) const {
		if (nodes.size() <= index)
			throw std::out_of_range("Invalid node index: " + std::to_string(index));
		Node *node = *std::next(nodes.begin(), index);
		if (!node)
			throw std::runtime_error("Node at index " + std::to_string(index) + " is null");
		return *node;
	}

	Node & Graph::operator[](const std::string &label) const {
		auto iter = labelMap.find(label);
		if (iter == labelMap.end()) {
			std::cerr << name << "\n";
			const_cast<Graph *>(this)->renderTo("graph_error.png");
			throw std::out_of_range("No node with label \"" + label + "\" found");
		}
		return *iter->second;
	}

	Node & Graph::operator[](const Node &node) const {
		return (*this)[node.getLabel()];
	}

	Graph & Graph::operator+=(const std::string &label) {
		addNode(label);
		return *this;
	}

	Graph & Graph::operator+=(Node *node) {
		addNode(node);
		return *this;
	}

	Graph & Graph::operator-=(Node &to_remove) {
		return *this -= &to_remove;
	}

	Graph & Graph::operator-=(Node *to_remove) {
		auto iter = std::find(nodes.begin(), nodes.end(), to_remove);
		if (iter == nodes.end())
			throw std::out_of_range("Can't remove: node is not in graph");
		for (Node *node: nodes) {
			node->unlink(to_remove, true);
			node->dirty();
		}

		nodes.erase(iter);
		labelMap.erase(to_remove->label);
		delete to_remove;
		return *this;
	}

	Graph & Graph::operator-=(const std::string &label) {
		for (Node *node: nodes)
			if (node->getLabel() == label)
				return *this -= node;
		throw std::out_of_range("Can't remove: no node with label \"" + label + "\" found");
	}

	Node & Graph::addNode(const std::string &label) {
		if (hasLabel(label))
			throw std::runtime_error("Can't add: a node with label \"" + label + "\" already exists");
		Node *node = new Node(this, label);
		labelMap.insert({label, node});
		nodes.push_back(node);
		return *node;
	}

	Node & Graph::addNode(Node *node) {
		labelMap.insert({node->getLabel(), node});
		nodes.push_back(node);
		return *node;
	}

	Node & Graph::rename(const std::string &old_label, const std::string &new_label) {
		return rename((*this)[old_label], new_label);
	}

	Node & Graph::rename(Node &node, const std::string &new_label) {
		return rename(&node, new_label);
	}

	Node & Graph::rename(Node *node, const std::string &new_label) {
		if (!node)
			throw std::invalid_argument("Can't rename a null node");
		if (node->getLabel() == new_label)
			return *node;
		if (hasLabel(new_label))
			throw std::runtime_error("Can't rename: a node with label \"" + new_label + "\" already exists");
		labelMap.erase(node->getLabel());
		node->label = new_label;
		labelMap.insert({new_label, node});
		return *node;
	}

	void Graph::link(const std::string &from, const std::string &to, bool bidirectional) {
		(*this)[from].link((*this)[to], bidirectional);
	}

	void Graph::unlink(const std::string &from, const std::string &to, bool bidirectional) {
		(*this)[from].unlink((*this)[to], bidirectional);
	}

	void Graph::unlink() {
		for (auto &pair: (*this))
			pair.second->unlink();
	}

	void Graph::cloneTo(Graph &out, std::unordered_map<Node *, Node *> *rename_map) {
		out.reset();

		// Maps old nodes to new nodes.
		std::unordered_map<Node *, Node *> node_map {};
		for (Node *node: nodes) {
			Node *new_node = new Node(&out, node->getLabel());
			node_map.insert({node, new_node});
			out.nodes.push_back(new_node);
			out.labelMap.insert({node->getLabel(), new_node});
		}

		for (auto &pair: node_map) {
			Node *old_node = pair.first, *new_node = pair.second;
			for (Node *old_link: old_node->out)
				new_node->link(node_map.at(old_link), false);
		}

		if (rename_map)
			*rename_map = node_map;
	}

	Graph Graph::clone(std::unordered_map<Node *, Node *> *rename_map) {
		Graph new_graph;
		cloneTo(new_graph, rename_map);
		return new_graph;
	}

	void Graph::addEdges(const std::string &pairs) {
		size_t last = 0, space;
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

	void Graph::reset() {
		while (!nodes.empty())
			*this -= nodes.front();
	}

	Node * Graph::find(std::function<bool(Node &)> predicate) {
		for (Node *node: nodes)
			if (predicate(*node))
				return node;
		return nullptr;
	}

	DFSResult Graph::DFS(Node *start) const {
		DFSResult::ParentMap parents;
		DFSResult::TimeMap discovered, finished;
		int time = 0;

		std::function<void(Node *)> visit = [&](Node *node) {
			discovered[node] = ++time;
			for (Node *out: node->out)
				if (!discovered.contains(out)) {
					parents[out] = node;
					visit(out);
				}
			finished[node] = ++time;
		};

		visit(start);
		return {*this, parents, discovered, finished};
	}

	DFSResult Graph::DFS(Node &start) const {
		return DFS(&start);
	}

	DFSResult Graph::DFS(const std::string &start_label) const {
		return DFS((*this)[start_label]);
	}

	std::vector<Node *> Graph::BFS(Node &start) const {
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

	std::vector<Node *> Graph::BFS(const std::string &start_label) const {
		return BFS((*this)[start_label]);
	}

	std::unordered_set<Node *> Graph::undirectedSearch(Node &start) const {
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

	std::unordered_set<Node *> Graph::undirectedSearch(const std::string &start_label) const {
		return undirectedSearch((*this)[start_label]);
	}

	std::vector<Node *> Graph::postOrder(Node &start) const {
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

	std::vector<Node *> Graph::reversePostOrder(Node &start) const {
		std::vector<Node *> post = postOrder(start);
		std::reverse(post.begin(), post.end());
		return post;
	}

	std::vector<std::pair<Graph::Label, Graph::Label>> Graph::bridges() const {
		std::vector<std::pair<Label, Label>> out;
		std::unordered_map<const Node *, bool> visited;
		std::unordered_map<const Node *, size_t> discovered;
		std::unordered_map<const Node *, size_t> low;
		std::unordered_map<const Node *, const Node *> parent;
		bridgeTraverse(*nodes.front(), visited, discovered, low, parent, out);
		return out;
	}

	std::list<Graph> Graph::components() const {
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

	std::unordered_map<Node *, std::unordered_set<Node *>> Graph::predecessors() const {
		std::unordered_map<Node *, std::unordered_set<Node *>> out;
		for (const Node *node: nodes)
			for (Node *successor: node->out)
				out[successor].insert(const_cast<Node *>(node));
		return out;
	}

	void Graph::color(Graph::ColoringAlgorithm algo, int color_min, int color_max) {
		const int total_colors = color_max != -1? color_max - color_min + 1 : -1;
		if (algo == Graph::ColoringAlgorithm::Bad) {
			if (color_max != -1 && total_colors < static_cast<int>(nodes.size()))
				throw UncolorableError();
			int color = color_min - 1;
			for (Node *node: nodes) {
				node->colors.clear();
				for (int i = 0; i < node->colorsNeeded; ++i)
					node->colors.insert(++color);
			}
		} else if (algo == Graph::ColoringAlgorithm::Greedy) {
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
			throw std::invalid_argument("Unknown graph coloring algorithm: " + std::to_string(static_cast<int>(algo)));
		}
	}

	std::vector<std::pair<Node *, Node *>> Graph::allEdges() const {
		std::vector<std::pair<Node *, Node *>> out;
		for (Node *node: nodes)
			for (Node *successor: *node)
				out.push_back({node, successor});
		return out;
	}

	std::vector<Node *> Graph::reverseTopoSort() {
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

	std::vector<Node *> Graph::topoSort() {
		auto out = reverseTopoSort();
		std::reverse(out.begin(), out.end());
		return out;
	}

	std::string Graph::toDot(const std::string &direction) {
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
			if (node->isolated()) {
				out << " " << node->getLabel();
				any_added = true;
			}

		if (any_added)
			out << ";";
		out << "\n";

		for (const Node *node: nodes)
			if (node->colors.size() == 1 && static_cast<size_t>(*node->colors.begin()) < colors.size())
				out << "\t" << node->getLabel() << " [fillcolor=" << colors.at(*node->colors.begin()) << "];\n";

		for (const Node *node: nodes)
			for (const Node *neighbor: node->out)
				if (neighbor != node)
					out << "\t" << node->getLabel() << " -> " << neighbor->getLabel() << ";\n";
		out << "}\n";
		return out.str();
	}

	void Graph::renderTo(std::string out_path, const std::string &direction) {
		std::ofstream out;
		std::string path = "/tmp/ll2w_graph_";
		for (const char ch: out_path)
			if (std::isdigit(ch) || std::isalpha(ch) || ch == '_')
				path += ch;
		path += ".dot";
		out.open(path);
		out << toDot(direction);
		out.close();

		if (out_path.substr(0, 2) == "./")
			out_path = (std::filesystem::current_path() / out_path.substr(2)).string();

		std::string type = "png";
		const size_t pos = out_path.find_last_of(".");
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

	decltype(Graph::labelMap)::iterator Graph::begin() {
		return labelMap.begin();
	}

	decltype(Graph::labelMap)::iterator Graph::end() {
		return labelMap.end();
	}

	decltype(Graph::labelMap)::const_iterator Graph::begin() const {
		return labelMap.cbegin();
	}

	decltype(Graph::labelMap)::const_iterator Graph::end() const {
		return labelMap.cend();
	}

	void Graph::bridgeTraverse(const Node &node, std::unordered_map<const Node *, bool> &visited,
	                           std::unordered_map<const Node *, size_t> &discovered,
	                           std::unordered_map<const Node *, size_t> &low,
	                           std::unordered_map<const Node *, const Node *> &parents,
	                           std::vector<std::pair<Label, Label>> &out) const {
		static size_t time = 0;
		visited[&node] = true;
		discovered[&node] = low[&node] = ++time;
		for (const Node *adjacent: node.allEdges())
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
