#include <vector>
#include <string>
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

string itos(int num)
{
    char tmp[20] = {0};
    sprintf(tmp, "%d", num);
    return string(tmp);
}

struct message
{
    message(size_t m_id, size_t m_iid, string m_ti, string m_con, string m_un)
        :mess_id(m_id),
        mess_itemid(m_iid),
        mess_time(m_ti),
        mess_content(m_con),
        mess_username(m_un)
    {

    }
    
    string to_zeroSplit_str()
    {
        char zero = 0;
        string res = itos(mess_id) + string(&zero, 1) + itos(mess_itemid) + string(&zero, 1);
        return res + mess_time + string(&zero, 1) + mess_content + string(&zero, 1) + mess_username;
    }

    static message zeroSplitStrToMess(string str)
    {
        size_t mess_id;
        size_t mess_itemid;
        string mess_time;
        string mess_content;
        string mess_username;

        int pos = str.find(char(0));
        if (pos != string::npos)
        {
            mess_id = atoi(string(str, 0, pos).c_str());
        }
        str = string(str, pos + 1);
        pos = str.find(char(0));
        if (pos != string::npos)
        {
            mess_itemid = atoi(string(str, 0, pos).c_str());
        }
        str = string(str, pos + 1);
        pos = str.find(char(0));
        if (pos != string::npos)
        {
            mess_time = string(str, 0, pos);
        }
        str = string(str, pos + 1);
        pos = str.find(char(0));
        if (pos != string::npos)
        {
            mess_content = string(str, 0, pos);
        }
        str = string(str, pos + 1);
        mess_username = str;
        return message(mess_id, mess_itemid, mess_time, mess_content, mess_username);
    }

    size_t mess_id;
    size_t mess_itemid;
    string mess_time;
    string mess_content;
    string mess_username;
};

void getMessagesByItem(int mess_itemid, MYSQL *conn, vector<message> *mess)
{
    if (conn == nullptr)
        return;
    string sql = "select * from message where mess_itemid=" + itos(mess_itemid);
    mysql_query( conn, sql.c_str() );
    MYSQL_RES *res_ptr = mysql_store_result(conn);
    MYSQL_ROW result_row;
    while (1)
    {
        result_row = mysql_fetch_row(res_ptr);
        if (!result_row)
        {
            mysql_free_result(res_ptr);
            return;
        }
        int mess_id = atoi(result_row[0]);
        int mess_itemid = atoi(result_row[1]);
        string mess_time = result_row[2];
        string mess_content = result_row[3];
        string mess_username = result_row[4];
        mess->push_back( message(mess_id, mess_itemid, mess_time, mess_content, mess_username) );
    }
}

//mess_id 值任意,无要求
int insertMess(MYSQL *conn, message mess)
{
    if (conn == nullptr)
        return -1;
    string sql = "insert into message values(null, " + itos(mess.mess_itemid) + ", \'" + mess.mess_time + "\', \'" + mess.mess_content + "\', \'" + mess.mess_username + "\')";
    //printf("*debug* %s\n", sql.c_str());
    int res = mysql_query(conn,sql.c_str());
    if (res)
    {
        printf("*ERROR* %s\n", mysql_error(conn));
        return -1;
    }

    return 1;
}
