#ifndef __ACCEPETOR_H__
#define __ACCEPETOR_H__

#include "Eventloop.h"
#include "Socket.h"
#include "Channel.h"
#include "NetAddr.h"
#include <errno.h>

namespace net
{
    class Acceptor : base::noncopyable
    {
        public:
        Acceptor( Eventloop *loop, int port);
        ~Acceptor();
        int getAcceptFd();
        void setNewConnCallback(std::function<void (int,  NetAddr&)> newConnCallback);
        void listen();
        void handleRead();
        private:
         Eventloop *acceptloop_;
        std::unique_ptr< Socket> acceptsoc_;
        std::unique_ptr< Channel> acceptchan_;
        int extraFd_;//防止fd用完无法关闭新到连接
        std::function<void (int,  NetAddr&)> newConnCallback_;
    };
}
#endif