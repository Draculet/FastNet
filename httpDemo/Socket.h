#ifndef __NET_SOCKET_H__
#define __NET_SOCKET_H__

class Socket : base::noncopyable
{
    public:
    Socket(int fd):
        fd_(fd)
    {

    }
    ~Socket()
    {
        ::close(fd_);
    }
    private:
    int fd_;
}
#endif