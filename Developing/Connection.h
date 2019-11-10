#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include "Socket.h"
#include "Channel.h"
#include "NetAddr.h"
#include "Buffer.h"
#include <memory>

using namespace std;
using namespace base;
using namespace net;

class Connection;
//class net::Eventloop;
typedef shared_ptr<Connection> Connptr;


class Connection : noncopyable : base::noncopyable
{
    public:
    Connection(int fd, NetAddr &addr, Eventloop *loop)
        :soc_(new Socket(fd, addr)),
         chan_(new net::Channel(fd, loop, this)),
         inputbuffer_(),
         outputbuffer_(),
         loop_(loop),
         highWaterMark(1024 * 10 * 10),
         state_(kInit)
    {

    }

    void handleEstablish()
    {
        state_ = kConnected;
        //不能传shared_from_this(),不能将指针保存在Channel中,会造成析构失败
        //需要在Channel调用ReadCallback函数时对Connection保护,已完成
        //详见Developing/Channel handleEvent函数
        chan_->setReadCallback(&Connection::handleRead, this);//TODO 改成move
        chan_->setWriteCallback(&Connection::handleWrite, this);
        chan_->setCloseCallback(&Connection::handleClose, this);
        chan_->setErrorCallback(&Connection::handleError, this);
        chan_->enableRead();
        //TODO 日志输出
    }
    /*
        handleRead连同this指针保存于function,并传给Channel
        Channel在调用handle系列函数之前需要先保护Connection
     */
    void handleRead()
    {
        int ret = inputbuffer_.readFd(soc_.getFd());
        if (ret > 0)
        {
            readCallback_(&inputbuffer_, shared_from_this() );
        }
        else if (ret == 0)
        {
            handleClose();
        }
        else
        {
            //非堵塞返回-1,日志报告即可
        }
    }

    void handleWrite()
    {
        if (outputbuffer_.readable() > 0 && chan_->waitToWrite() )//TODO 条件是否充足
        {
            //FIXME 不能直接使用sendData,会重复写入
            //int remain = sendData(outputbuffer_.current(), outputbuffer_.readable());
            int haswrite = ::write(soc_.getFd(), outputbuffer_.current(), outputbuffer_.readable());
            if (haswrite > 0)
            {
                outputbuffer_.retrieve(haswrite);
                if (outputbuffer_.readable() == 0)
                {
                    if (chan_->waitToWrite())
                    {
                        chan_->disableWrite();
                    }
                    if (writeFinishCallBack)
                    {
                        loop_->runInloop(bind(&Connection::writeFinishCallBack, shared_from_this() ) );//注意保护Connection
                    }
                } 
            }
        }
        else
        {
            //TODO
            //没有数据要发送,出现异常,日志报告
            chan_->disableWrite();
        }
    }

    void handleError()
    {
        //TODO 打印日志?
    }

    void handleClose()
    {
        state_ = kClosed;
        chan_->disableAll();
        //此时channel已被Poller移除
        //TODO 是否为了效率在Poller中暂时保存?
        closeCallback(shared_from_this() );
        //在主线程中移除连接,注意保护Connection,防止提前析构
        //在主线程移除连接之后应正常Connection析构,生命周期结束
    }

    int send(string str)
    {
        return send(str.c_str(), str.size());
    }

    int send(char *data, size_t len)
    {
        if (loop_->inloop())
            return sendData(data, len);
        else
        {
            loop_->insertQueue(bind(&Connection::sendData, shared_from_this(), data, len) );
            //TODO 返回-1较突兀
            return -1;
        }
    }
    
    //重点测试该部分
    //return remain
    size_t sendData(const char *data, size_t len)
    {
        int remain = len;
        //没有待发送数据,直接发送
        if (outputbuffer_.readable() == 0 && !chan_->waitToWrite() )//TODO 条件是否充足
        {
            int haswrite = ::write(soc_.getFd(), data, len);
            if (haswrite < 0)
            {
                //TODO 日志输出EAGIN错误
                outputbuffer_.append(data, len);
                remain = len;
                chan_->enableWrite();
            }
            else if (haswrite == len)
            {
                if (chan_->waitToWrite())
                {
                    chan_->disableWrite();
                }
                if (writeFinishCallBack)
                {
                    loop_->runInloop(bind(&Connection::writeFinishCallBack, shared_from_this() ) );//注意保护Connection
                }
                remain = 0;
            }
            else if (haswrite >= 0 && haswrite < len)
            {
                //assert(haswrite < len && haswrite >= 0);
                outputbuffer_.append(data + haswrite, len - haswrite);
                remain = len - haswrite;
                chan_->enableWrite();
            }
            else
            {
                //TODO 出现错误
                return -1;
            }
        }
        else//缓冲区中还有数据待发送,将需要发送数据放入缓冲区尾部
        {
            if (outputbuffer_.readable() >= highWaterMark)
            {
                if (highWaterCallBack)
                {
                    loop_->runInloop(bind(&Connection::highWaterCallBack, shared_from_this(), &outputbuffer_ ) );//注意保护Connection
                }
                else
                {
                    //TODO 默认丢掉数据
                    outputbuffer_.retrieveAll();
                }
            }
            outputbuffer_.append(data, len);
            chan_->enableWrite();
        }

        return remain;
    }

    void setReadCallback(function<void (Buffer *, Connection *)> readCallback)
    {
        readCallback_ = readCallback;
    }

    void setCloseCallback(function<void (Connptr)> closeCallback)
    {
        closeCallback_ = closeCallback;
    }

    private:
    unique_ptr<Socket> soc_;
    unique_ptr<net::Channel> chan_;
    Buffer inputbuffer_;
    Buffer outputbuffer_;
    //function<void (Buffer *, Connection *)> writeCallback;
    Eventloop * loop_;
    int highWaterMark;
    enum {kInit, kConnected, kClosed} state_;
    function<void (Buffer *, Connection *)> readCallback_;
    function<void (Connptr)> closeCallback_;
};

#endif