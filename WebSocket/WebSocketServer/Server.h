#include "../../FastNet/include/Server.h"
#include "../../FastNet/include/Connection.h"
#include "../../FastNet/include/Buffer.h"
#include "../WebSocketCodec/headerParse/requestParser.h"
#include "../WebSocketCodec/base64.h"
#include "../WebSocketCodec/sha1.h"
#include "../WebSocketCodec/WebSocketCodec.h"


void onRead(std::map<std::shared_ptr<net::Connection>, WebSocketCodec *> *codecMap, net::Buffer *buf, std::shared_ptr<net::Connection> conn);
void onDisConnect(std::map<std::shared_ptr<net::Connection>, WebSocketCodec *> *codecMap, std::shared_ptr<net::Connection> conn);
//FIXME 函数只考虑x86平台
uint64_t ntoh64(uint64_t &input);
void onDisConnect(std::map<std::shared_ptr<net::Connection>, WebSocketCodec *> *codecMap, std::shared_ptr<net::Connection> conn);
void onRead(std::map<std::shared_ptr<net::Connection>, WebSocketCodec *> *codecMap, net::Buffer *buf, std::shared_ptr<net::Connection> conn);