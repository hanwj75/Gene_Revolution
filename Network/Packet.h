#pragma once
#include <google/protobuf/message.h>
#include <vector>
#include <cstdint>

//[패킷 헤더]
//4바이트: 전체 길이 (헤더 + 바디)
//4바이트: 메시지 ID

struct PacketHeader {
	uint32_t size; //전체 패킷 크기
	uint32_t id; //메시지 ID
};

class PacketUtil {
public:
	//직렬화 선언 (protobuf -> buffer) msgId포함
	static std::vector<uint8_t> serialize(uint32_t msgId, const google::protobuf::Message& msg);

	// 역직렬화 선언 (buffer -> header + body)
	// buffer: 전체 수신 버퍼 (헤더 + 바디)
	// header: 출력용 헤더 구조체
	// body: 바디(헤더 제외) 출력

	static bool deserialize(const std::vector<uint8_t>& buffer, PacketHeader& header, std::vector<uint8_t>& body);
};