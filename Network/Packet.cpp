#include "Packet.h"
#include <cstring> // memcpy

// serialize 구현
std::vector<uint8_t> PacketUtil::serialize(uint32_t msgId, const google::protobuf::Message& msg) {
	// protobuf 메시지를 직렬화하여 body에 넣음
	std::vector<uint8_t> body(msg.ByteSizeLong());
	if (!body.empty()) {
		msg.SerializeToArray(body.data(), static_cast<int>(body.size()));
	}

	PacketHeader header;
	header.size = static_cast<uint32_t>(sizeof(PacketHeader) + body.size());
	header.id = msgId;

	// 전체 버퍼 크기는 header.size (헤더+바디)
	std::vector<uint8_t> buffer(static_cast<size_t>(header.size));

	// 메모리 복사 (헤더, 그 다음 바디)
	memcpy(buffer.data(), &header, sizeof(header));
	if (!body.empty()) {
		memcpy(buffer.data() + sizeof(header), body.data(), body.size());
	}

	return buffer;
}

// deserialize 구현
bool PacketUtil::deserialize(const std::vector<uint8_t>& buffer, PacketHeader& header, std::vector<uint8_t>& body) {
	if (buffer.size() < sizeof(PacketHeader)) {
		return false;
	}

	// 헤더 복사
	memcpy(&header, buffer.data(), sizeof(PacketHeader));

	// 길이 검증: 실제 수신 버퍼 크기와 헤더에 적힌 크기 비교
	if (buffer.size() != header.size) {
		return false;
	}

	// 바디 추출
	body.assign(buffer.begin() + sizeof(PacketHeader), buffer.end());
	return true;
}