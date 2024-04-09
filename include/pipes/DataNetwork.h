#pragma once

#include "pipes/PipeNetwork.h"
#include "realm/Realm.h"
#include "tileentity/TileEntity.h"
#include "util/Concepts.h"

namespace Game3 {
	class DataNetwork: public PipeNetwork {
		public:
			DataNetwork(size_t id_, const std::shared_ptr<Realm> &);

			Substance getType() const final { return Substance::Data; }

			bool canWorkWith(const std::shared_ptr<TileEntity> &) const final;

			/** Iterates all unique adjacent data networks. */
			template <typename Fn>
			requires Returns<Fn, void, std::shared_ptr<DataNetwork>>
			static void visitNetworks(const Place &place, Fn &&visitor) {
				std::unordered_set<std::shared_ptr<DataNetwork>> visited_networks;

				for (const Direction direction: ALL_DIRECTIONS) {
					auto network = std::static_pointer_cast<DataNetwork>(PipeNetwork::findAt(place + direction, Substance::Data));
					if (!network || visited_networks.contains(network))
						continue;

					visited_networks.insert(network);
					visitor(network);
				}
			}

			/** Iterates all unique adjacent data networks until the given function returns true. */
			template <typename Fn>
			requires Returns<Fn, bool, std::shared_ptr<DataNetwork>>
			static void visitNetworks(const Place &place, Fn &&visitor) {
				std::unordered_set<std::shared_ptr<DataNetwork>> visited_networks;

				for (const Direction direction: ALL_DIRECTIONS) {
					auto network = std::static_pointer_cast<DataNetwork>(PipeNetwork::findAt(place + direction, Substance::Data));
					if (!network || visited_networks.contains(network))
						continue;

					visited_networks.insert(network);
					if (visitor(network))
						return;
				}
			}

			template <typename Fn>
			requires Returns<Fn, void, TileEntityPtr>
			static void visitNetwork(const std::shared_ptr<DataNetwork> &network, Fn &&visitor) {
				std::shared_ptr<Realm> realm = network->getRealm();
				assert(realm);

				std::unordered_set<TileEntityPtr> visited;

				auto visit = [&](const auto &set) {
					auto lock = set.sharedLock();
					for (const auto &[position, direction]: set) {
						TileEntityPtr member = realm->tileEntityAt(position);
						if (!member || visited.contains(member))
							continue;
						visited.insert(member);
						visitor(member);
					}
				};

				visit(network->getInsertions());
				visit(network->getExtractions());
			}

			template <typename Fn>
			requires Returns<Fn, bool, TileEntityPtr>
			static bool visitNetwork(const std::shared_ptr<DataNetwork> &network, Fn &&visitor) {
				std::shared_ptr<Realm> realm = network->getRealm();
				assert(realm);

				std::unordered_set<TileEntityPtr> visited;

				auto visit = [&](const auto &set) {
					auto lock = set.sharedLock();
					for (const auto &[position, direction]: set) {
						TileEntityPtr member = realm->tileEntityAt(position);
						if (!member || visited.contains(member))
							continue;
						visited.insert(member);
						if (visitor(member))
							return true;
					}

					return false;
				};

				if (visit(network->getInsertions()))
					return true;

				return visit(network->getExtractions());
			}

			/** Broadcasts a message to all tile entities accessible from the source through a data network.
			 *  The data payload is copied for each message sent. */
			static void broadcast(const AgentPtr &source, const std::string &name, std::any &data) {
				std::unordered_set<GlobalID> gids;

				visitNetworks(source->getPlace(), [&](const std::shared_ptr<DataNetwork> &network) {
					visitNetwork(network, [&](const TileEntityPtr &member) {
						const GlobalID gid = member->getGID();
						if (gids.contains(gid))
							return;
						gids.insert(gid);
						std::any copy(data);
						source->sendMessage(member, name, copy);
					});
				});
			}

			/** Broadcasts a message to all tile entities accessible from the source through a data network.
			 *  The data payload is newly constructed for each message sent. */
			template <typename... Args>
			static void broadcast(const AgentPtr &source, const std::string &name, Args &&...args) {
				std::unordered_set<GlobalID> gids;

				visitNetworks(source->getPlace(), [&](const std::shared_ptr<DataNetwork> &network) {
					visitNetwork(network, [&](const TileEntityPtr &member) {
						const GlobalID gid = member->getGID();
						if (gids.contains(gid))
							return;
						gids.insert(gid);
						std::any data(std::in_place_type<Buffer>, std::forward<Args>(args)...);
						source->sendMessage(member, name, data);
					});
				});
			}
	};

	using DataNetworkPtr = std::shared_ptr<DataNetwork>;
}
