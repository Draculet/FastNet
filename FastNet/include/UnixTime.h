#ifndef __UNIXTIME_H__
#define __UNIXTIME_H__
#include <time.h>
#include <string>
#include <sys/time.h>
#include <stdint.h>

namespace net
{
class UnixTime
{
    public:
    UnixTime();
    UnixTime(int64_t usFromEpoch);
    int64_t getUsFromEpoch();
    static UnixTime now();
    static UnixTime fromTimeType(time_t t);
    static UnixTime fromTimeType(time_t t, int us);
    static time_t toTimeType(UnixTime t);
    UnixTime &operator+=(double sec);
    UnixTime &operator+=(UnixTime t);
    //TODO 对format支持
    std::string toString() const;
    std::string toLocString() const;

    static const int million = 1000 * 1000;
    private:
    int64_t usFromEpoch_;
};

//需要内联,否则编译会出现multi define错误
inline bool operator<(UnixTime t1, UnixTime t2)
{
    return t1.getUsFromEpoch() < t2.getUsFromEpoch();
}

inline UnixTime operator+(UnixTime t, double sec)
{
    int64_t us = static_cast<int64_t>(sec * UnixTime::million);
    return UnixTime(t.getUsFromEpoch() + us);
}

inline UnixTime operator+(UnixTime t, UnixTime t2)
{
    return UnixTime(t.getUsFromEpoch() + t2.getUsFromEpoch());
}
}
#endif