//#include <iostream>
//#include "auth.pb.h"
//using namespace std;
//int main() {
//	GOOGLE_PROTOBUF_VERIFY_VERSION;
//
//	// C2S_Login 생성
//	auth::C2S_Login req;
//	req.set_user_id("dkdlel12");
//	req.set_nickname("HWJ");
//	req.set_password("1234");
//
//	//직렬화
//	string blob;
//	if (!req.SerializeToString(&blob)) {
//		cerr << "Serialize Failed!\n";
//		return 1;
//	}
//	cout << "Serialized Bytes: " << blob.size() << endl;
//
//	//역직렬화(검증)
//	auth::C2S_Login parsed;
//	if (!parsed.ParseFromString(blob)) {
//		cerr << "Parse Failed!\n";
//		return 1;
//	}
//
//	cout <<"user_id: "<<parsed.user_id() << ", nickname: " << parsed.nickname() << ", password: " << parsed.password() << endl;
//
//	google::protobuf::ShutdownProtobufLibrary();
//	return 0;
//}


#include "auth.pb.h"
#include <iostream>
#include <boost/asio.hpp>
#include "server/lobby_server.h"
namespace net = boost::asio;
using namespace std;

int main() {
    try {
        GOOGLE_PROTOBUF_VERIFY_VERSION;
        net::io_context io;
        LobbyServer server(io, 12345);
        cout << "LobbyServer listening on 12345\n";
        io.run();
        google::protobuf::ShutdownProtobufLibrary();
    }
    catch (exception& e) {
        cerr << "ERROR: " << e.what() << "\n";
    }
    return 0;
}
