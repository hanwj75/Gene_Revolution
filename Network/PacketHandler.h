#pragma once
#include <unordered_map>
#include <functional>
#include <memory>
#include <iostream>
#include "center.pb.h"

class PacketHandler {
public:
	using HandlerFunc = std::function<void(const std::vector<uint8_t>&)>;
	static PacketHandler& instance() {
		static PacketHandler inst;
		return inst;
	}

	void registerHandler(uint32_t msgId, HandlerFunc func) {
		handlers[msgId] = func;
	}

	void handlePacket(uint32_t msgId, const std::vector<uint8_t>& body) {
		auto it = handlers.find(msgId);
		if (it != handlers.end()) {
			it->second(body);
		}
		else {
			std::cout << "[PacketHandler] Unknown msgId = " << msgId << std::endl;
		}
	}

private:
	std::unordered_map<uint32_t, HandlerFunc> handlers;
};