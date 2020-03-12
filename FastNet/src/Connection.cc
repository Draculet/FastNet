#include "../include/Connection.h"
using namespace std;
using namespace net;
using namespace base;

Connection::Connection(int fd, NetAddr &addr, Eventloop *loop)
    :soc_(new Socket(fd, addr)),
    //FIXME Channel传未构造完成的shared_ptr<Connection> core dump了
    chan_(new net::Channel(fd, loop)),
    inputbuffer_(),
    outputbuffer_(),
    loop_(loop),
    highWaterMark(1024 * 10 * 10),
    state_(kInit)
{}

Connection::~Connection()
{
    printf("*debug* ~Connection fd %d\n", soc_->getFd());
}

void Connection::handleEstablish()//本线程同步调用
{
    printf("*debug* hanleEstablish %d\n", gettid());
    state_ = kConnected;
    //不能传shared_from_this(),不能将指针保存在Channel中,会造成析构失败
    //需要在Channel调用ReadCallback函数时对Connection保护,已完成
    //详见Developing/Channel handleEvent函数
    chan_->setReadCallback(bind(&Connection::handleRead, this));//TODO 改成move
    chan_->setWriteCallback(bind(&Connection::handleWrite, this));
    chan_->setCloseCallback(bind(&Connection::handleClose, this));
    chan_->setErrorCallback(bind(&Connection::handleError, this));
    chan_->bindConn(shared_from_this());
    chan_->enableRead();
    if (connCallback_)
        connCallback_(shared_from_this());
    //TODO 日志输出
}
/*
    handleRead连同this指针保存于function,并传给Channel
    Channel在调用handle系列函数之前需要先保护Connection
    */
void Connection::handleRead()
{
    long ret = inputbuffer_.readFd(soc_->getFd());
    printf("*debug* Fd %d readFd ret %ld\n", soc_->getFd(), ret);
    if (ret > 0)
    {
        if (readCallback_)
            readCallback_(&inputbuffer_, shared_from_this() );
    }
    else if (ret == 0)
    {
        handleClose();
    }
    else
    {
        if (errno ==  EAGAIN || errno == EWOULDBLOCK)
            perror("handleRead");
        //非堵塞返回-1,日志报告即可
        else
        {
            perror("handleRead");
            //TODO
            handleClose();
        }
    }
}

void Connection::handleWrite()
{
    if (outputbuffer_.readable() > 0 && chan_->waitToWrite() )//TODO 条件是否充足
    {
        //FIXME 不能直接使用sendData,会重复写入
        //int remain = sendData(outputbuffer_.current(), outputbuffer_.readable());
        int haswrite = ::write(soc_->getFd(), outputbuffer_.current(), outputbuffer_.readable());
        printf("*debug* write %d bytes in HandleWrite\n", haswrite);
        if (haswrite > 0)
        {
            outputbuffer_.retrieve(haswrite);
            if (outputbuffer_.readable() == 0)
            {
                if (chan_->waitToWrite())
                {
                    chan_->disableWrite();
                }
                if (writeFinishCallBack_)
                {
                    loop_->runInloop(bind(&Connection::writeFinishCallBack_, shared_from_this() ) );//注意保护Connection
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

void Connection::handleError()
{
    //TODO 打印日志?
    printf("*debug* handleError\n");
}

void Connection::handleClose()
{
    printf("*debug* handleClose()\n");
    state_ = kClosed;
    chan_->disableAll();
    if (disConnCallback_)
        disConnCallback_(shared_from_this() );
    //此时channel已被Poller移除
    //TODO 是否为了效率在Poller中暂时保存?
    closeCallback_(shared_from_this() ); //使主线程去删除连接
    //在主线程中移除连接,注意保护Connection,防止提前析构
    //在主线程移除连接之后应正常Connection析构,生命周期结束
    /*
        Connection保护工作在Channel::handleEvent调用readCallback前通过shared_ptr完成保护
        另外通过runInloop(bind(handleClose, (shared_ptr<Connection>)conn)))调用handleClose也可起到保护作用
    */
    /*
        debug用,故意延长时间
    */
    //sleep(10);
}

/*
int send(string str)
{
    return send(str.c_str(), str.size());
}
*/

int Connection::send(string str)
{
    if (loop_->inloop())
        return sendData(str);
    else
    {
        loop_->insertQueue(bind(&Connection::sendData, shared_from_this(), str) );
        //TODO 返回-1较突兀
        return -1;
    }
}

/*
int send(const char *data, size_t len)
{
    if (loop_->inloop())
        return sendData(data, len);
    else
    {
        loop_->insertQueue(bind(&Connection::sendData, shared_from_this(), data, len) );
        for (int i = 0; i < len; i++)
        {
            printf("=============Debug2==================\n");
            printf("%d%c  ", data[i], data[i]);
        }
        //TODO 返回-1较突兀
        return -1;
    }
}
*/

//string版本
size_t Connection::sendData(string str)
{
    const char *data = str.c_str(); 
    size_t len = str.size();
    int remain = len;
    //没有待发送数据,直接发送
    if (outputbuffer_.readable() == 0 && !chan_->waitToWrite() )//TODO 条件是否充足
    {

        /*for (int i = 0; i < len; i++)
        {
            printf("=============Debug==================\n");
            printf("%d%c  ", data[i], data[i]);
        }*/

        int haswrite = ::write(soc_->getFd(), data, len);
        printf("*debug* write %d bytes in sendData\n", haswrite);
        if (haswrite < 0)
        {
            //TODO 日志输出EAGIN错误
            printf("*debug* In Thread %d send ret -1\n", gettid());
            outputbuffer_.append(data, len);
            printf("*debug* Current bufsize: %ld\n", outputbuffer_.readable());
            remain = len;
            chan_->enableWrite();
        }
        else if (haswrite == len)
        {
            if (chan_->waitToWrite())
            {
                chan_->disableWrite();
            }
            printf("*debug* Write Finish\n");
            if (writeFinishCallBack_)
            {
                loop_->runInloop(bind(&Connection::writeFinishCallBack_, shared_from_this() ) );//注意保护Connection
            }
            remain = 0;
        }
        else if (haswrite >= 0 && haswrite < len)
        {
            printf("*debug* In Thread %d send unFinish\n", gettid());
            //assert(haswrite < len && haswrite >= 0);
            outputbuffer_.append(data + haswrite, len - haswrite);
            printf("*debug* Current bufsize: %ld\n", outputbuffer_.readable());
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
            if (highWaterCallBack_)
            {
                loop_->runInloop(bind(highWaterCallBack_, &outputbuffer_ ) );//注意保护Connection
            }
            else
            {
                //TODO 默认丢掉数据
                printf("*debug* drop data %ld bytes\n", outputbuffer_.readable());
                outputbuffer_.retrieveAll();
            }
        }
        outputbuffer_.append(data, len);
        printf("*debug* After Drop Current bufsize: %ld\n", outputbuffer_.readable());
        chan_->enableWrite();
    }

    return remain;
}

//该版本用于传string时有错误
//重点测试该部分
//return remain
/*
size_t sendData(const char *data, size_t len)
{
    int remain = len;
    //没有待发送数据,直接发送
    if (outputbuffer_.readable() == 0 && !chan_->waitToWrite() )//TODO 条件是否充足
    {

        for (int i = 0; i < len; i++)
        {
            printf("=============Debug==================\n");
            printf("%d%c  ", data[i], data[i]);
        }

        int haswrite = ::write(soc_->getFd(), data, len);
        printf("*debug* write %d bytes in sendData\n", haswrite);
        if (haswrite < 0)
        {
            //TODO 日志输出EAGIN错误
            printf("*debug* In Thread %d send ret -1\n", gettid());
            outputbuffer_.append(data, len);
            printf("*debug* Current bufsize: %d\n", outputbuffer_.readable());
            remain = len;
            chan_->enableWrite();
        }
        else if (haswrite == len)
        {
            if (chan_->waitToWrite())
            {
                chan_->disableWrite();
            }
            printf("*debug* Write Finish\n");
            if (writeFinishCallBack_)
            {
                loop_->runInloop(bind(&Connection::writeFinishCallBack_, shared_from_this() ) );//注意保护Connection
            }
            remain = 0;
        }
        else if (haswrite >= 0 && haswrite < len)
        {
            printf("*debug* In Thread %d send unFinish\n", gettid());
            //assert(haswrite < len && haswrite >= 0);
            outputbuffer_.append(data + haswrite, len - haswrite);
            printf("*debug* Current bufsize: %d\n", outputbuffer_.readable());
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
            if (highWaterCallBack_)
            {
                loop_->runInloop(bind(highWaterCallBack_, &outputbuffer_ ) );//注意保护Connection
            }
            else
            {
                //TODO 默认丢掉数据
                printf("*debug* drop data %d bytes\n", outputbuffer_.readable());
                outputbuffer_.retrieveAll();
            }
        }
        outputbuffer_.append(data, len);
        printf("*debug* After Drop Current bufsize: %d\n", outputbuffer_.readable());
        chan_->enableWrite();
    }

    return remain;
}
*/

void Connection::setConnCallback(function<void (shared_ptr<Connection>)> connCallback)
{
    connCallback_ = connCallback;
}

void Connection::setDisConnCallback(function<void (shared_ptr<Connection>)> disConnCallback)
{
    disConnCallback_ = disConnCallback;
}

void Connection::setReadCallback(function<void (Buffer *, shared_ptr<Connection>)> readCallback )
{
    readCallback_ = readCallback;
}


void Connection::setCloseCallback(function<void (shared_ptr<Connection>)> closeCallback)
{
    closeCallback_ = closeCallback;
}

void Connection::setWriteFinishCallBack(function<void ()> writeFinishCallBack)
{
    writeFinishCallBack_ = writeFinishCallBack;       
}

NetAddr Connection::getPeerAddr()
{
    return *(soc_->getAddr());
}

Channel *Connection::getChannel()
{
    return &*chan_;
}

int Connection::getFd()
{
    return soc_->getFd();
}

Eventloop *Connection::getLoop()
{
    return loop_;
}

/*
    debug用
    */
int Connection::getInputSize()
{
    return inputbuffer_.readable();
}

int Connection::getOutputSize()
{
    return outputbuffer_.readable();
}