#include <iostream>
#include <stdexcept>

#include "graph/Graph.h"
#include "graph/Node.h"

namespace Game3 {
	Node::Node(Graph *owner_, std::string label_):
		owner(owner_), label(std::move(label_)) {}

	Node & Node::setColorsNeeded(int count) {
		colorsNeeded = count;
		return *this;
	}

	const std::string & Node::getLabel() const {
		return label;
	}

	Node & Node::rename(const std::string &new_label) {
		return owner->rename(this, new_label);
	}

	bool Node::reflexive() const {
		return out.contains(const_cast<Node *>(this));
	}

	bool Node::link(Node &other, bool bidirectional) {
		return link(&other, bidirectional);
	}

	bool Node::link(Node *other, bool bidirectional) {
		bool already_linked = out.contains(other);
		if (!already_linked) {
			out.insert(other);
			other->in.insert(this);
		}

		if (bidirectional && other != this)
			other->link(*this);

		return already_linked;
	}

	bool Node::unlink(Node &other, bool bidirectional) {
		return unlink(&other, bidirectional);
	}

	bool Node::unlink(Node *other, bool bidirectional) {
		bool exists = out.contains(other);
		out.erase(other);
		other->in.erase(this);
		reachability.erase(other);

		if (bidirectional && other != this)
			other->unlink(*this, false);

		return exists;
	}

	void Node::unlink() {
		for (Node *other: out)
			other->in.erase(this);
		out.clear();
	}

	bool Node::isolated() const {
		return out.empty();
	}

	void Node::dirty() {
		index = -1;
	}

	int Node::getIndex() {
		if (index != -1)
			return index;

		for (const Node *node: owner->getNodes()) {
			++index;
			if (node == this)
				return index;
		}

		throw std::runtime_error("Node not found in parent graph");
	}

	const Node::Set & Node::getOut() const {
		return out;
	}

	const Node::Set & Node::getIn() const {
		return in;
	}

	Node::Set Node::allEdges() const {
		Set set = out;
		set.insert(in.begin(), in.end());
		return set;
	}

	bool Node::canReach(Node &other) {
		if (other.owner != owner)
			return false;

		if (reachability.contains(&other))
			return reachability.at(&other);

		std::unordered_set<Node *> visited;
		std::list<Node *> queue {this};
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

	void Node::clearReachability() {
		reachability.clear();
	}

	size_t Node::degree() const {
		size_t deg = out.size();
		for (Node *neighbor: in)
			if (!out.contains(neighbor))
				++deg;
		return deg;
	}

	Node * Node::parent() const {
		if (in.size() != 1)
			throw std::runtime_error("Cannot find parent of node with " + std::to_string(in.size()) + " inward edges");
		return *in.begin();
	}

	Node & Node::operator+=(Node &neighbor) {
		return *this += &neighbor;
	}

	Node & Node::operator+=(Node *neighbor) {
		out.insert(neighbor);
		return *this;
	}

	Node & Node::operator-=(Node &neighbor) {
		return *this -= &neighbor;
	}

	Node & Node::operator-=(Node *neighbor) {
		out.erase(neighbor);
		in.erase(neighbor);
		return *this;
	}

	Node & Node::operator-=(const std::string &label) {
		for (Node *node: out)
			if (node->label == label)
				return *this -= node;
		throw std::out_of_range("Can't remove: no neighbor with label \"" + label + "\" found");
	}

	decltype(Node::out)::iterator Node::begin() {
		return out.begin();
	}

	decltype(Node::out)::iterator Node::end() {
		return out.end();
	}

	decltype(Node::in)::iterator Node::ibegin() {
		return in.begin();
	}

	decltype(Node::in)::iterator Node::iend() {
		return in.end();
	}

	std::ostream & operator<<(std::ostream &os, const Node &node) {
		return os << node.getLabel();
	}
}
