#ifndef __NET_BUFFER_H__
#define __NET_BUFFER_H__

#include <vector>
#include <cstring>
#include <assert.h>
#include "Noncopyable.h"
#include <string>
#include <cstdio>
#include <iostream>
#include <sys/uio.h>

namespace net
{
    const size_t kinitSize = 1024;
    const size_t kinitPreSize = 8;
    class Buffer : base::noncopyable
    {
        public:
        Buffer():kinitPreSize_(kinitPreSize),
                kinitSize_(kinitSize),
                kreadIndex_(kinitPreSize),
                kwriteIndex_(kreadIndex_),
                buffer_(kinitSize)
        {
            assert(readable() == 0);
            assert(writable() == buffer_.size() - kinitPreSize_);
            assert(prependable() == 8);
        }

        const char *current()
        {
            return begin() + kreadIndex_;
        }

        size_t readable()
        {
            return kwriteIndex_ - kreadIndex_;
        }

        size_t writable()
        {
            return buffer_.size() - kwriteIndex_;
        }

        size_t prependable()
        {
            return kreadIndex_;
        }

        void enlarge(size_t len)
        {
            if (len > writable() + kreadIndex_ - kinitPreSize_)
            {
                printf("*debug* enlarge resize\n");
                //Need Test
                //debug();
                //printf("Before enlager: %s\n", std::string(&(*(buffer_.begin())) + kreadIndex_, readable()).c_str());
                //printf("vec.size() = %d\n", buffer_.size());
                //printf("readable() = %d\n", readable());
                //
                std::vector<char> tmp;
                size_t tmpSize = buffer_.size();
                buffer_.swap(tmp);
                buffer_.resize(tmpSize + 2 * len);
                memcpy(&(*(buffer_.begin())), &(*(tmp.begin())), tmpSize);
                printf("*debug* now buffer size %ld\n", buffer_.size());
                //debug()
                //printf("After enlager: %s\n", std::string(&(*(buffer_.begin())) + kreadIndex_, readable()).c_str());
                //printf("vec.size() = %d\n", buffer_.size());
                //printf("readable() = %d\n", readable());
                //printf("================================================\n");
                //
            }
            else
            {
                printf("*debug* enlarge to init\n");
                size_t readableSize = readable();
                memcpy(&(*(buffer_.begin())) + kinitPreSize_, &(*(buffer_.begin())) + kreadIndex_, readableSize);
                kreadIndex_ = kinitPreSize_;
                kwriteIndex_ = kreadIndex_ + readableSize;
            }
        }

        void append(const char *p, size_t len)
        {
            if (len > writable())
                enlarge(len);
            assert(len <= writable());
            memcpy(&(*(buffer_.begin())) + kwriteIndex_, p, len);
            kwriteIndex_ += len;
        }

        std::string retrieveAsString(size_t len)
        {
            std::string s(&(*(buffer_.begin())) + kreadIndex_, len > readable()? readable(): len);
            retrieve(len);
            return s;
        }

        std::string preViewAsString(size_t len)
        {
            std::string s(&(*(buffer_.begin())) + kreadIndex_, len > readable()? readable(): len);
            return s;
        }

        void retrieve(size_t len)
        {
            if (len >= readable())
                retrieveAll();
            else
                kreadIndex_ += len;
        }

        void retrieveAll()
        {
            kreadIndex_ = kinitPreSize_;
            kwriteIndex_ = kreadIndex_;
        }

        std::string retrieveAllAsString()
        {
            std::string s = retrieveAsString(readable());
            assert(readable() == 0);
            return s;
        }
        
        void prepend(const char *p, size_t len)
        {
            assert(len <= prependable());
            kreadIndex_ -= len;
            memcpy(&*buffer_.begin() + kreadIndex_, p, len);
        }

        int readFd(int fd)
        {
            char extrabuf[1024 * 10] = {0};
            struct iovec vec[2];
            vec[0].iov_base = &*buffer_.begin() + kwriteIndex_;
            vec[0].iov_len = writable();
            vec[1].iov_base = extrabuf;
            vec[1].iov_len = sizeof(extrabuf);
            const int iovcnt = (writable() < sizeof(extrabuf)) ? 2 : 1;
            //FIXME readv可能返回-1,不能用size_t,什么情况下返回-1?
            //打印的结果是connection reset by peer
            //使用long是否不妥
            const long n = readv(fd, vec, iovcnt); //readv可能返回-1,不能用size_t
            printf("*debug* readv ret %ld\n", n);
            if (n <= 0)
            {
                if (n < 0)
                {
                    perror("readFd");
                }
                
                return n;
            }
            else if (n <= writable())
            {
                kwriteIndex_ += n;
            }
            else
            {
                //debug();
                size_t hasWrite = writable();
                kwriteIndex_ = buffer_.size();
                //std::cout << "================debug===============" << std::endl;
                //std::cout << preViewAsString(readable()) << std::endl;
                //std::cout << "==================center========================" << std::endl;
                //出错在先修改index,writable()计算结果得0
                append(extrabuf, n - hasWrite);
                //std::cout << preViewAsString(readable()) << std::endl;
                //std::cout << "==============debug end==============" << std::endl;
                //debug();
            }
            return n;
        }

        void debug()
        {
            printf("bufferAddress = %p\n", &*buffer_.begin());
            printf("kreadIndex_ = %ld\n", kreadIndex_);
            printf("kwriteIndex_ = %ld\n", kwriteIndex_);
            printf("readable() = %ld\n", readable());
            printf("writable() = %ld\n", writable());
            printf("vec.size() = %ld\n", buffer_.size());
            printf("vec.capacity() = %ld\n", buffer_.capacity());
            printf("================================\n");
        }

        const char *begin()
        {
            return &*(buffer_.begin());
        }
        //TODO pretend
        
        private:
        size_t kreadIndex_;
        size_t kwriteIndex_;
        size_t kinitPreSize_;
        size_t kinitSize_;
        std::vector<char> buffer_;
    };
}

#endif