#include <time.h>
#include <string>
#include <sys/time.h>
#include <stdint.h>
class UnixTime
{
    public:
    UnixTime()
        :usFromEpoch_(0)
    {

    }

    UnixTime(int64_t usFromEpoch)
        :usFromEpoch_(usFromEpoch)
    {
        
    }

    int64_t getUsFromEpoch()
    {
        return usFromEpoch_;
    }
    
    static UnixTime now()
    {
        struct timeval tval;
        gettimeofday(&tval, nullptr);
        return UnixTime(tval.tv_sec * million + tval.tv_usec);
    }

    static UnixTime fromTimeType(time_t t)
    {
        return UnixTime(t * million);
    }

    static UnixTime fromTimeType(time_t t, int us)
    {
        return UnixTime(static_cast<int64_t>(t) * million + us);
    }

    static time_t toTimeType(UnixTime t)
    {
        return static_cast<time_t>(t.usFromEpoch_ / million);
    }

    UnixTime &operator+=(double sec)
    {
        int64_t us = static_cast<int64_t>(sec * million);
        usFromEpoch_ += us;
        return *this;
    }

    UnixTime &operator+=(UnixTime t)
    {
        usFromEpoch_ += t.getUsFromEpoch();
        return *this;
    }
    
    //TODO 对format支持
    std::string toString() const
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

    std::string toLocString() const
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


    static const int million = 1000 * 1000;
    private:
    int64_t usFromEpoch_;
};

bool operator<(UnixTime t1, UnixTime t2)
{
    return t1.getUsFromEpoch() < t2.getUsFromEpoch();
}

UnixTime operator+(UnixTime t, double sec)
{
    int64_t us = static_cast<int64_t>(sec * UnixTime::million);
    return UnixTime(t.getUsFromEpoch() + us);
}

UnixTime operator+(UnixTime t, UnixTime t2)
{
    return UnixTime(t.getUsFromEpoch() + t2.getUsFromEpoch());
}