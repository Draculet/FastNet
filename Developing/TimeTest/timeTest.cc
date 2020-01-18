#include "../UnixTime.h"
#include <iostream>
#include <map>
using namespace std;

/* class TimeNode
{
    public:
    TimeNode(UnixTime timeout, double round, )
    private:
    UnixTime timeout_;
    int round_;
    string name_;
}
*/
int main(void)
{
    map<UnixTime, string> timeMap;
    UnixTime t = UnixTime::now();
    cout << t.toLocString() << endl;
    t += 4.1;
    cout << t.toLocString() << endl;
    UnixTime t1;
    t1 = t + 1;
    if (t1 < t)
        cout << "t1 < t" << endl;
    else
        cout << "t1 > t" << endl;
    cout << t.toLocString() << endl;
    cout << t1.toLocString() << endl;
    UnixTime t2(t);
    timeMap.insert(pair<UnixTime, string>(t, "t-1"));
    auto iter = timeMap.find(t);
    if (iter != timeMap.end())
    {
        timeMap[t] = "t-2";
    }
    //timeMap.insert(pair<UnixTime, string>(t, "t-2"));
    iter = timeMap.find(t1);
    if (iter != timeMap.end())
    {
        timeMap[t] = "t1-1";
    }
    else
        timeMap.insert(pair<UnixTime, string>(t1, "t1-1"));
    iter = timeMap.find(t1);
    if (iter != timeMap.end())
    {
        timeMap[t1] = "t1-2";
    }
    else
        timeMap.insert(pair<UnixTime, string>(t1, "t1-2"));
    //timeMap.insert(pair<UnixTime, string>(t1, "t1-2"));

    //timeMap.insert(pair<UnixTime, string>(t2, "t2-1"));
    //timeMap.insert(pair<UnixTime, string>(t2, "t2-2"));
    timeMap[t2] = "t2-1";
    timeMap[t2] = "t2-2";
    for (auto iter : timeMap)
    {
        cout << iter.first.toString() << " " << iter.second << endl;
    }
    UnixTime t3 = t + t2;
    cout << t3.toLocString() << endl;
    t3 += t;
    cout << t3.toLocString() << endl;
}