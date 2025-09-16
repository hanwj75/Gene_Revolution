#include "Packet.h"
#include <cstring> // memcpy

// serialize ����
std::vector<uint8_t> PacketUtil::serialize(uint32_t msgId, const google::protobuf::Message& msg) {
	// protobuf �޽����� ����ȭ�Ͽ� body�� ����
	std::vector<uint8_t> body(msg.ByteSizeLong());
	if (!body.empty()) {
		msg.SerializeToArray(body.data(), static_cast<int>(body.size()));
	}

	PacketHeader header;
	header.size = static_cast<uint32_t>(sizeof(PacketHeader) + body.size());
	header.id = msgId;

	// ��ü ���� ũ��� header.size (���+�ٵ�)
	std::vector<uint8_t> buffer(static_cast<size_t>(header.size));

	// �޸� ���� (���, �� ���� �ٵ�)
	memcpy(buffer.data(), &header, sizeof(header));
	if (!body.empty()) {
		memcpy(buffer.data() + sizeof(header), body.data(), body.size());
	}

	return buffer;
}

// deserialize ����
bool PacketUtil::deserialize(const std::vector<uint8_t>& buffer, PacketHeader& header, std::vector<uint8_t>& body) {
	if (buffer.size() < sizeof(PacketHeader)) {
		return false;
	}

	// ��� ����
	memcpy(&header, buffer.data(), sizeof(PacketHeader));

	// ���� ����: ���� ���� ���� ũ��� ����� ���� ũ�� ��
	if (buffer.size() != header.size) {
		return false;
	}

	// �ٵ� ����
	body.assign(buffer.begin() + sizeof(PacketHeader), buffer.end());
	return true;
}