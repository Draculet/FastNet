#include "../UnixTime.h"
#include <iostream>
#include <vector>
#include <functional>
#include <map>
#include <set>
#include <unistd.h>

using namespace std;

class TimeNode
{
    public:
    TimeNode(function<void()> callback, UnixTime timeout, double round, string name = "default")
        :callback_(callback),
        timeout_(timeout),
        round_(round),
        name_(name)
    {

    }

    TimeNode()
    {

    }

    bool round()
    {
        return round_ > 0.0;
    }

    void cb()
    {
        callback_();
    }

    void update()
    {
        if (round_ > 0.0)
        {
            timeout_ += round_;
        }
    }

    string getName() const
    {
        return name_;    
    }

    UnixTime getTime() const
    {
        return timeout_;
    }
    

    private:
    UnixTime timeout_;
    double round_;
    string name_;
    function<void()> callback_;
};

bool operator<(TimeNode tn1, TimeNode tn2)
{
    return tn1.getName() < tn2.getName();
}

//Timer所有方法都需要处理为同步调用
/*
    名字指定唯一计时器
*/
class Timer
{
    public:
    //跨线程调用,使用runInloop
    void addTime(TimeNode &tn)
    {
        if (insertMap(tn))
        {
            resetTimeFd(/*arg1, arg2*/);
        }
    }

    //跨线程
    void cancelTime(string name)
    {
        auto it = times_.find(name);
        if (it != times_.end())
        {
            UnixTime t = it->second.getTime();
            auto iter = timeMap_[t].find(it->second);
            if (iter != timeMap_[t].end())
            {
                timeMap_[t].erase(iter);
                //FIXME
                if (timeMap_[t].empty())
                {
                    timeMap_.erase(t);
                }
                times_.erase(it);
            }
            else
            {
                cout << "Error" << endl;
            }
        }
        else
            cout << "Time Not Found" << endl;
    }

    void handleRead()
    {
        UnixTime now(UnixTime::now());
        auto itend = timeMap_.lower_bound(now);
        for (auto iter = timeMap_.begin(); iter != itend; iter++)
        {
            for (auto &obj : iter->second)
            {
                timeouts_.push_back(obj);
            }
        }
        for (auto &obj : timeouts_)
        {
            times_.erase(obj.getName());
        }
        timeMap_.erase(timeMap_.begin(), itend);
        for (auto &obj : timeouts_)
        {
            obj.cb();
            if (obj.round())
            {
                obj.update();
                insertMap(obj);
            }
        }
        cout << "in handleRead()" << endl;
        printMap();
        timeouts_.clear();
        resetTimeFd();
    }


    bool insertMap(TimeNode &tn)
    {
        bool reset = false;
        UnixTime t = tn.getTime();
        string name = tn.getName();
        auto it = timeMap_.begin();
        if (it == timeMap_.end() || t < it->first)
        {
            reset = true;
        }
        auto iter = times_.find(tn.getName());
        if (iter != times_.end())
        {
            cancelTime(name);
            times_[name] = tn;
            timeMap_[t].insert(tn);
        }
        else
        {
            times_[name] = tn;
            timeMap_[t].insert(tn);
        }
        /* 
        string tname = tn.getName();
        auto iter = timeMap_.find(t);
        if (iter != timeMap_.end())
        {
            auto iter = timeMap_[t].find(tn);
            if (iter != timeMap_[t].end())
            {
                
                    //注意set中的元素默认const,是不能任意修改的,修改会造成排序失效
                
            }
            else //iter == timeMap_[t].end()
            {
                timeMap_[t].insert(tn);
            }
        }
        else //iter == timeMap_.end()
        {
            timeMap_[t].insert(tn);
        }
        */


        return reset;
    }

    void resetTimeFd()
    {
        auto iter = timeMap_.begin();
        if (iter != timeMap_.end())
        {
            cout << "resetTimeFd() Latest Timer :" << iter->first.toLocString() << endl;
            cout << "TImeName:" << endl;
            for (auto &o: iter->second)
            {
                cout << o.getName() << endl;
            }
            cout << endl;
        }
        else
            cout << "No Timer" << endl;
    }

    void printMap()
    {
        for (auto &p : timeMap_)
        {
            cout << "UnixTime: " << p.first.toLocString() << endl;
            for (auto &obj : p.second)
            {
                cout << "Timeout: "  << obj.getTime().toLocString() << endl;
                cout << "Name: " << obj.getName() << endl;
            }
        }
        cout << endl;
        cout << "TimeOut Queue: " << endl;
        for (auto &obj : timeouts_)
        {
            cout << "Timeout: "  << obj.getTime().toLocString() << endl;
            cout << "Name: " << obj.getName() << endl;
        }
        cout << endl;
        cout << "Times Queue: " << endl;
        for (auto &obj : times_)
        {
            cout << "TimeName: " << obj.first << endl;
        }
    }
    private:
    map< UnixTime, set<TimeNode> > timeMap_;
    map< string, TimeNode> times_;
    vector<TimeNode> timeouts_;
};

void call()
{
    cout << "Time' Out " << endl;
}
int main(void)
{
    Timer timer;
    UnixTime t(UnixTime::now());
    TimeNode tn(call, t, 0);
    TimeNode tn1(call, t + 1, 0);
    TimeNode tn2(call, t + 2, 0);
    TimeNode tn3(call, t + 3, 5);
    TimeNode tn4(call, t + 3, 0, "hello");
    TimeNode tn5(call, t + 5, 10, "hello");
    TimeNode tn6(call, t + 3, 0, "hello2");
    TimeNode tn7(call, t + 3, 5);
    TimeNode tn8(call, t + 3, 5);
    TimeNode tn9(call, t + 3, 0);
    timer.addTime(tn1);
    timer.addTime(tn2);
    timer.addTime(tn3);
    timer.addTime(tn);
    timer.addTime(tn4);
    timer.addTime(tn5);
    timer.addTime(tn6);
    timer.addTime(tn7);
    timer.addTime(tn9);
    timer.addTime(tn8);
    timer.printMap();
    cout << endl;
    cout << endl;
    //timer.cancelTime(t + 3, "default");
    //timer.cancelTime(t, "default");
    //timer.cancelTime(t + 1, "default");
    //timer.printMap();
    sleep(5);
    timer.handleRead();
    timer.printMap();
    timer.cancelTime("hello2");
    timer.cancelTime("hello2");
    timer.cancelTime("hello");
    timer.cancelTime("default");
    timer.handleRead();
    timer.printMap();
    cout << "<=========================<" << endl;
    UnixTime tnow(UnixTime::now());
    TimeNode ntn(call, tnow + 1, 1, "de1");
    TimeNode ntn2(call, tnow, 0.5, "de2");
    TimeNode ntn3(call, tnow, 2, "de3");
    TimeNode ntn4(call, tnow + 5, 0, "de4");
    TimeNode ntn5(call, tnow + 7, 0, "de5");
    timer.addTime(ntn);
    timer.addTime(ntn2);
    timer.addTime(ntn3);
    timer.addTime(ntn4);
    timer.addTime(ntn5);
    while(1)
    {
        cout << "Now is " << UnixTime::now().toLocString() << endl;
        cout << "=========================" << endl;
        timer.handleRead();
        cout << "=========================" << endl;
        timer.printMap();
        usleep(0.5 * 1000000);
    }
}