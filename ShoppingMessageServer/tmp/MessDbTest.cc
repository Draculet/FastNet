#include "Mess_Dbop.h"
#include "DbConnPool.h"
#include <iostream>

int main(void)
{
    vector<message> mess;
    message m(0, 9, "2019-9-9 19:00", "hello, man", "name");
    DbConnPool pool("Message");
    pool.connect(3);
    insertMess(pool.getConn(), m);
    insertMess(pool.getConn(), m);
    insertMess(pool.getConn(), m);
    getMessagesByItem(9, pool.getConn(), &mess);
    for (auto m : mess)
    {
        cout << m.mess_id << " " << m.mess_itemid << " " << m.mess_time << " " << m.mess_content << " " << m.mess_username << endl;
    }
}