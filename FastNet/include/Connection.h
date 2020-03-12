#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include "Socket.h"
#include "Channel.h"
#include "NetAddr.h"
#include "Buffer.h"
#include "Eventloop.h"
#include "Gettid.h"
#include <memory>
#include <functional>
#include <errno.h>

namespace net
{
class Connection : base::noncopyable, public std::enable_shared_from_this< Connection>
{
    public:
    Connection(int fd,  NetAddr &addr,  Eventloop *loop);
    ~Connection();
    void handleEstablish();
    void handleRead();
    void handleWrite();
    void handleError();
    void handleClose();
    int send(std::string str);
    size_t sendData(std::string str);
    void setConnCallback(std::function<void (std::shared_ptr< Connection>)> connCallback);
    void setDisConnCallback(std::function<void (std::shared_ptr< Connection>)> disConnCallback);
    void setReadCallback(std::function<void ( Buffer *, std::shared_ptr< Connection>)> readCallback );
    void setCloseCallback(std::function<void (std::shared_ptr< Connection>)> closeCallback);
    void setWriteFinishCallBack(std::function<void ()> writeFinishCallBack);
     NetAddr getPeerAddr();
     Channel *getChannel();
    int getFd();
     Eventloop *getLoop();
    int getInputSize();
    int getOutputSize();
    private:
    std::unique_ptr< Socket> soc_;
    std::unique_ptr< Channel> chan_;
     Buffer inputbuffer_;
     Buffer outputbuffer_;
    //function<void (Buffer *, Connection *)> writeCallback;
     Eventloop * loop_;
    int highWaterMark;
    enum {kInit, kConnected, kClosed} state_;
    std::function<void (std::shared_ptr<Connection>) > connCallback_;
    std::function<void (std::shared_ptr<Connection>) > disConnCallback_;
    std::function<void ( Buffer *, std::shared_ptr<Connection>)> readCallback_;
    std::function<void (std::shared_ptr<Connection>)> closeCallback_;
    std::function<void ( Buffer *)> highWaterCallBack_;
    std::function<void ()> writeFinishCallBack_;
};
}
#endif