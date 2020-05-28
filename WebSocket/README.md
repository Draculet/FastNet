# 使用须知

* WebSocketCodec类是一个保存状态的类，使用它配合使用Buffer来解析WebSocket协议，

* 由于webSocketCodec类保存解析过程中的中间状态,即Buffer中可读的字节数小于一个完整的websocket请求时会等待连接下次可读,中间状态保存于类中,因此使用该类时需将其作为一个成员类来使用,切不可每次连接进入可读状态就重新构造一个新的该类,这种情况会造成解析中间状态丢失,解析失败.

* 使用frame::toFrame可将要发送的数据进行WebSoceket帧封装后发送给对方.

