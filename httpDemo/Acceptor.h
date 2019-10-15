#ifndef __NET_ACCEPTOR_H__
#define __NET_ACCEPTOR_H__

#include "../Channel.h"
#include "../Eventloop.h"
#include "../Noncopyable.h"
#include "NetAddr.h"

namespace net
{
    const int kinitListen = 30;
    class Eventloop;
    class Acceptor : base::noncopyable
    {
        public:
        Acceptor(Eventloop *, int);
        void listen();
        Channel *getChannel(){return &acceptChannel_;}
        private:
        NetAddr addr_;
        Eventloop *loop_;
        Channel acceptChannel_;
    };
}

#endif