### 主动关闭的会有time_wait状态? 正确
### 发送缓冲区还有数据时关闭CTRL+C,才会有time_wait状态? 不是

## 记录
### shutdown和close对连接状态的影响应该是相同的.
##### 当主动shutdown/close方调用close/shutdown wr时,会发送fin,状态变为fin_wait1,当收到对方的ack,状态变为fin_wait2,而对方发送ack后状态变为close_wait.在这个状态下如果对方不调用close,那么fin_wait2/close_wait状态将一直保持,似乎会有超时机制(详见tcp/ip详解2)

##### 当主动关闭方在关闭之前发送很多数据,而对端不接收,那么很快对方接受缓冲区满,我方发送缓冲区满,此时主动方调用close/shutdown wr,对方连接状态一直为establish,直到将数据读走一些后状态变为close_wait(发送ack),之后才会读到0.

##### 但是处于close_wait状态下的一端close()之后并没有使对端进入time_wait,而是直接进入closed状态(使用tcp_info选项来获取得到的state),为什么?
##### 使用netstat -an | grep 9981监听我们的端口链接状态却发现状态确实为time_wait
##### 另外,使用了reuseraddr选项却没有解决bind重用addr问题. //编写代码时的问题(在bind之后才设置选项)
##### 当对方缓冲区被堆满后,我方调用close发送fin,此时对方无暇发送ack(无法接受fin),如果在对端不接收,直接调用close也发送fin,那么就会看到closing状态
##### shutdown rd对连接状态没有任何影响,双方都保持在establish,而shutdown wr则发送fin
##### close会关闭socket的文件描述符,如果描述符变为0才会发送fin,如果描述符计数大于1,那么socket可能不会被关闭,正确的关闭方式是shutdown rdwr,之后再close.