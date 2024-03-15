#include "Log.h"
#include "command/local/UsageCommand.h"
#include "net/LocalClient.h"
#include "packet/LoginPacket.h"
#include "util/Util.h"

namespace Game3 {
	extern Lockable<std::unordered_map<std::string, size_t>> entityUpdates;

	void UsageCommand::operator()(LocalClient &client) {
		INFO_("");
		INFO("Header bytes: \e[3{}m{:L}\e[39m", client.headerBytes.empty()? '2' : '1', client.headerBytes.size());
		INFO("Payload size: \e[33m{:L}\e[39m", client.payloadSize);
		{
			INFO_("Packets received:");
			auto lock = client.receivedPacketCounts.sharedLock();
			for (const auto &[packet_id, count]: client.receivedPacketCounts)
				INFO("    {:3}\e[2m:\e[22m {:L}", packet_id, count);
		}
		{
			INFO_("Packets sent:");
			auto lock = client.sentPacketCounts.sharedLock();
			for (const auto &[packet_id, count]: client.sentPacketCounts)
				INFO("    {:3}\e[2m:\e[22m {:L}", packet_id, count);
		}

		INFO("Bytes read: \e[36m{:L}\e[39m", client.bytesRead.load());
		INFO("Bytes written: \e[35m{:L}\e[39m", client.bytesWritten.load());

		{
			auto lock = entityUpdates.sharedLock();
			if (!entityUpdates.empty()) {
				INFO_("Entity updates:");
				size_t total = 0;
				for (const auto &[name, updates]: entityUpdates) {
					INFO("- {}: \e[32m{:L}\e[39m", name, updates);
					total += updates;
				}
				INFO("Total: \e[32m{:L}\e[39m", total);
			}
		}
	}
}
