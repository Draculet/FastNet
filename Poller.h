#ifndef __NET_POLLER_H__
#define __NET_POLLER_H__

#include <sys/epoll.h>
#include <unistd.h>
#include <stdio.h>
#include "Channel.h"

namespace net
{
const int kinitlist = 16;

class Channel;
class Poller
{
    typedef std::vector<Channel *> channelList;
    typedef std::vector<epoll_event> eventList;
    public:
    Poller();
    int poll(channelList *);
    void update(Channel *);
    void removeChannel(int);
    void fillActiveChannel(channelList *, int);
    

    private:
    int epfd_;
    eventList events_;//用于epoll返回活跃事件,并且是可伸缩的
    //map<> TODO 增加一个fd到channel的map
};


}
#endif