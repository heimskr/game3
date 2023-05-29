#include "Log.h"
#include "command/local/UsageCommand.h"
#include "net/LocalClient.h"
#include "packet/LoginPacket.h"
#include "util/Util.h"

namespace Game3 {
	void UsageCommand::operator()(LocalClient &client) {
		INFO("Bytes read: " << client.bytesRead);
		INFO("Bytes written: " << client.bytesWritten);
		INFO("Header bytes: " << client.headerBytes.size());
		INFO("Payload size: " << client.payloadSize);
		{
			INFO("Packets received:");
			std::shared_lock lock(client.receivedPacketCountsMutex);
			for (const auto &[packet_id, count]: client.receivedPacketCounts)
				INFO("    Packet " << packet_id << ": " << count);
		}
		{
			INFO("Packets sent:");
			std::shared_lock lock(client.sentPacketCountsMutex);
			for (const auto &[packet_id, count]: client.sentPacketCounts)
				INFO("    Packet " << packet_id << ": " << count);
		}
	}
}
