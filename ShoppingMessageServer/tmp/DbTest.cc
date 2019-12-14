#include "DbConnPool.h"

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

    size_t mess_id;
    size_t mess_itemid;
    string mess_time;
    string mess_content;
    string mess_username;
};
string itos(int num)
{
    char tmp[20] = {0};
    sprintf(tmp, "%d", num);
    return string(tmp);
}

int main(void)
{
    DbConnPool pool("Message");
    pool.connect(2);
    MYSQL *conn1 = pool.getConn();
    printf("%0x\n", conn1);
    if (conn1)
    {
        printf("Sucess\n");
    }
    MYSQL *conn2 = pool.getConn();
    printf("debug %0x\n", conn2);
    MYSQL *conn5 = conn2;
    printf("debug %0x\n", conn5);
    MYSQL *conn6 = conn2;
    printf("debug %0x\n", conn6);
    if (conn2)
    {
        printf("Sucess\n");
    }
    MYSQL *conn3 = pool.getConn();
    if (conn3)
    {
        printf("Sucess\n");
    }
    else
    {
        printf("Failed\n");
    }
    mysql_query(conn1,"select * from message");
    MYSQL_RES *res_ptr = mysql_store_result(conn1);
    MYSQL_ROW result_row;
    while (1)
    {
        result_row = mysql_fetch_row(res_ptr);
        if (!result_row)
        {
            mysql_free_result(res_ptr);
            break;
        }
        int mess_id = atoi(result_row[0]);
        int mess_itemid = atoi(result_row[1]);
        string mess_time = result_row[2];
        string mess_content = result_row[3];
        string mess_username = result_row[4];
        printf("%d %d %s %s %s\n" , mess_id, mess_itemid, mess_time.c_str(), mess_content.c_str(), mess_username.c_str());
    }
    pool.putBack(conn1);
    MYSQL *conn4 = pool.getConn();
    if (conn4)
    {
        printf("Sucess\n");
    }
    if (mysql_query(conn4,"delete from message"))
    {
        printf("%s\n", mysql_error(conn4));
    }
    
    {
        sleep(15);
        DbConnPool pool("Message");
        pool.connect(3);
    }
    printf("END\n");
    message mess(1, 2, "2019-9-9 23:33", "ggg", "hello");
    string sql = "insert into message values(null, " + itos(mess.mess_itemid) + ", \'" + mess.mess_time + "\', \'" + mess.mess_content + "\', \'" + mess.mess_username + "\')";
    cout << sql << endl;
}