#include "graph/DFSResult.h"
#include "graph/Graph.h"

namespace Game3 {
	DFSResult::DFSResult(const Graph &graph_,
	                     DFSResult::ParentMap parents_,
	                     DFSResult::TimeMap discovered_,
	                     DFSResult::TimeMap finished_):
		graph(&graph_), parents(std::move(parents_)), discovered(std::move(discovered_)), finished(std::move(finished_)) {}

	DFSResult::DFSResult(const Graph &graph_,
	                     const std::vector<Node *> &parents_,
	                     const std::vector<int> &discovered_,
	                     const std::vector<int> &finished_): graph(&graph_) {
		for (int i = 0, len = parents_.size(); i < len; ++i)
			parents[&(*graph)[i]] = parents_[i];
		for (int i = 0, len = discovered_.size(); i < len; ++i)
			discovered[&(*graph)[i]] = discovered_[i];
		for (int i = 0, len = finished_.size(); i < len; ++i)
			finished[&(*graph)[i]] = finished_[i];
	}

	std::ostream & operator<<(std::ostream &os, const DFSResult &result) {
		os << "Parents    [";
		for (const auto &pair: result.parents)
			os << " " << pair.first->getLabel() << "<-" << pair.second->getLabel();
		os << " ]\nDiscovered [";
		for (const auto &pair: result.discovered)
			os << " " << pair.first->getLabel() << ":" << pair.second;
		os << " ]\nFinished   [";
		for (const auto &pair: result.finished)
			os << " " << pair.first->getLabel() << ":" << pair.second;
		return os << "]\n";
	}
}
