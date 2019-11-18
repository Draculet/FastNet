#ifndef __NET_EVENTLOOP_H__
#define __NET_EVENTLOOP_H__

#include <vector>
#include <memory>
#include <functional>
#include "Poller.h"
#include "Gettid.h"
#include "Mutex.h"
#include "Channel.h"

namespace net
{
    class Channel;
    class Eventloop : base::noncopyable
    {
        public:
        //TODO typedef std::vector<Connection *> connectionList;
        typedef std::vector<Channel *> channelList;
        typedef std::vector<std::function<void()>> taskQueue;
        Eventloop();
        ~Eventloop();//TODO 待补充
        void loop();
        //void runInThread();//thread的函数//已移除
        void update(Channel *channel);//注册关注事件
        void dotasks();//消费队列任务
        void insertQueue(std::function<void()>);//将任务插入任务队列
        void runInloop(std::function<void()>);//跨线程调用核心
        bool inloop(){return tid_ == base::gettid();}
        void wakeUp();
        int getWakeupFd();
        void handleRead();
        //TODO connectionList connlist;
        channelList activelist;
        taskQueue tasks;
        private:
        std::unique_ptr<Poller> poller_;
        int tid_;//用于判断currentThread是否是ownThread
        bool quit_;//用于判断loop退出否,是否应用mutex保护?
        //TODO quit_用atomic<bool>

        int wakeFd_;
        std::unique_ptr<Channel> wakechan_;
        Mutex mutex_;
    };
}

#endif