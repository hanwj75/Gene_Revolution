#pragma once
#include <google/protobuf/message.h>
#include <vector>
#include <cstdint>

//[��Ŷ ���]
//4����Ʈ: ��ü ���� (��� + �ٵ�)
//4����Ʈ: �޽��� ID

struct PacketHeader {
	uint32_t size; //��ü ��Ŷ ũ��
	uint32_t id; //�޽��� ID
};

class PacketUtil {
public:
	//����ȭ ���� (protobuf -> buffer) msgId����
	static std::vector<uint8_t> serialize(uint32_t msgId, const google::protobuf::Message& msg);

	// ������ȭ ���� (buffer -> header + body)
	// buffer: ��ü ���� ���� (��� + �ٵ�)
	// header: ��¿� ��� ����ü
	// body: �ٵ�(��� ����) ���

	static bool deserialize(const std::vector<uint8_t>& buffer, PacketHeader& header, std::vector<uint8_t>& body);
};