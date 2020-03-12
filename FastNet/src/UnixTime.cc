#include "../include/UnixTime.h"
using namespace std;
using namespace net;

UnixTime::UnixTime()
    :usFromEpoch_(0)
{

}

UnixTime::UnixTime(int64_t usFromEpoch)
    :usFromEpoch_(usFromEpoch)
{
    
}

int64_t UnixTime::getUsFromEpoch()
{
    return usFromEpoch_;
}

UnixTime UnixTime::now()
{
    struct timeval tval;
    gettimeofday(&tval, nullptr);
    return UnixTime(tval.tv_sec * million + tval.tv_usec);
}

UnixTime UnixTime::fromTimeType(time_t t)
{
    return UnixTime(t * million);
}

UnixTime UnixTime::fromTimeType(time_t t, int us)
{
    return UnixTime(static_cast<int64_t>(t) * million + us);
}

time_t UnixTime::toTimeType(UnixTime t)
{
    return static_cast<time_t>(t.usFromEpoch_ / million);
}

UnixTime &UnixTime::operator+=(double sec)
{
    int64_t us = static_cast<int64_t>(sec * million);
    usFromEpoch_ += us;
    return *this;
}

UnixTime &UnixTime::operator+=(UnixTime t)
{
    usFromEpoch_ += t.getUsFromEpoch();
    return *this;
}

//TODO 对format支持
std::string UnixTime::toString() const
{
    struct tm utctime;
    char buf[128] = {0};
    time_t sec = static_cast<time_t>(usFromEpoch_ / million);
    int usec = static_cast<int>(usFromEpoch_ % million);
    //有锁影响写日志性能
    gmtime_r(&sec, &utctime);
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.%06d",
    utctime.tm_year + 1900, utctime.tm_mon + 1, utctime.tm_mday, utctime.tm_hour,
    utctime.tm_min, utctime.tm_sec, usec);
    return buf;
}

std::string UnixTime::toLocString() const
{
    struct tm utctime;
    char buf[128] = {0};
    time_t sec = static_cast<time_t>(usFromEpoch_ / million);
    int usec = static_cast<int>(usFromEpoch_ % million);
    //有锁影响写日志性能
    localtime_r(&sec, &utctime);
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.%06d",
    utctime.tm_year + 1900, utctime.tm_mon + 1, utctime.tm_mday, utctime.tm_hour,
    utctime.tm_min, utctime.tm_sec, usec);
    return buf;
}