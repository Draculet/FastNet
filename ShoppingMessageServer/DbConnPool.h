#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <mysql/mysql.h>
#include <iostream>
#include <string.h>
#include <vector>
#include "../Mutex.h"

using namespace std;
using namespace net;

struct Conn
{
    Conn(MYSQL *c, bool u)
    :conn(c),
    used(u)
    {
    }

    Conn()
    {
    }

    ~Conn()
    {
        //不能直接断开连接,原因是vector.resize会造成前面元素的析构
    }

    MYSQL * conn;
    bool used;
};

class DbConnPool
{
    public:
    DbConnPool(string dbname)
        :dbconns_(),
        mutex_(),
        num_(0),
        dbname_(dbname)
    {
        
    }
    ~DbConnPool()
    {
        for (int i = 0; i < num_; i++)
        {
            mysql_close(dbconns_[i].conn);
        }
    }

    void connect(int num)
    {
        if (num > 0)
        {
            num_ = num;
        }
        else
        {
            return;
        }
        dbconns_.resize(num);
        for (int i = 0; i < num; i++)
        {
            dbconns_[i].conn = mysql_init(NULL);
            dbconns_[i].conn = mysql_real_connect(dbconns_[i].conn, "127.0.0.1", "root", "335369376", dbname_.c_str(), 0, NULL, 0);
            if (dbconns_[i].conn == nullptr)
            {
                printf("mysql_real_connect Failed\n");
                break;
            }
            mysql_query(dbconns_[i].conn,"set names UTF8");
            dbconns_[i].used = false;
        }
    }

    MYSQL *getConn()
    {
        MutexGuard m(mutex_);
        for (int i = 0; i < num_; i++)
        {
            if (dbconns_[i].used == false)
            {
                dbconns_[i].used = true;
                return dbconns_[i].conn;
            }
        }
        printf("*WARNNING* Run out of Conn\n");
        MYSQL *conn = mysql_init(NULL);
        conn = mysql_real_connect(conn, "127.0.0.1", "root", "335369376", dbname_.c_str(), 0, NULL, 0);
        if (conn == nullptr)
        {
            printf("mysql_real_connect Failed\n");
            return nullptr;
        }
        mysql_query(conn, "set names UTF8");
        Conn c(conn, true);
        dbconns_.push_back(c);
        dbconns_.back().used = true;
        return dbconns_.back().conn;
    }

    void putBack(MYSQL *conn)
    {
        MutexGuard m(mutex_);
        for (int i = 0; i < num_; i++)
        {
            if (conn == dbconns_[i].conn)
            {
                dbconns_[i].used = false;
            }
        }
    }


    private:
    vector<Conn> dbconns_;//保持数据库连接
    Mutex mutex_;//
    int num_;
    string dbname_;
};