#include "../Server.h"
#include "../Connection.h"
#include "../Buffer.h"
#include "../ThreadLoopPool.h"
#include "DbConnPool.h"
#include "Mess_Dbop.h"
#include "webSocketCodec.h"
#include <set>

class messageServer
{
    public:
    messageServer():
        serv_(new Server(4, 8832)),
        pool_(new ThreadLoopPool()),
        dbpool_(new DbConnPool("Message")),
        itemToConn_(),
        codecMap_(),
        mutex_()
    {
        dbpool_->connect(4);
        serv_->setReadCallback(bind(&messageServer::onRead, this, placeholders::_1, placeholders::_2));
        serv_->setConnCallback(bind(&messageServer::onConnection, this, placeholders::_1));
        serv_->setDisConnCallback(bind(&messageServer::onDisConnection, this, placeholders::_1));
    }

    void onRead(Buffer *buf, shared_ptr<Connection> conn)
    {
        shared_ptr<webSocketCodec> codec;
        {
            MutexGuard m(mutex_);
            if (codecMap_.find(conn) == codecMap_.end())
            {
                codecMap_[conn] = shared_ptr<webSocketCodec>(new webSocketCodec(buf));
                codec = codecMap_[conn];
            }
            else
            {
                codec = codecMap_[conn];
            }
        }
        if (buf->preViewAsString(3) == "GET" || buf->preViewAsString(4) == "POST" ) //http
        {
            printf("*debug* 解析HTTP协议\n");
            requestParser parser(buf);//用法有误
            //由于http头长度较短,一般不会分段,所以暂时保留
            parser.parse();
            //如果请求头不完整,则不会进入循环,直接返回
            for (int i = 0; i < parser.size(); i++)
            {
                if ( webSocketCodec::isWebSocket( &parser.reqs[i]) )
                {
                    string path = parser.reqs[i].getPath();
                    int mess_itermid = atoi(string(path, 1).c_str() );
                    printf("*PREVIEW* %d\n",mess_itermid);
                    {
                        MutexGuard m(mutex_);
                        itemToConn_[mess_itermid].insert(conn);
                    }
                    conn->send( webSocketCodec::toHttpResponse( &parser.reqs[i]) );
                    pool_->getNextLoop()->runInloop( bind(&messageServer::getMessAndSend, this, conn, mess_itermid) );
                }
            }
        }
        else //webSocket Frame
        {
            printf("*debug* 解析WebSocket Frame\n");
            //如果请求头不完整,则不会进入循环,直接返回
            int num = codec->parse();
            //string mark = "abcd";
            for (int i = 0; i < num; i++)
            {
                codec->frames[i].debugPrint();
                printf("\n");
                if (codec->frames[i].getOpcode() == 1)
                {
                    cout << "*debug* From Client:\n" << codec->frames[i].getMessege() << endl;
                    if (codec->frames[i].getMessege().size() != 0)
                    {
                        string data = codec->frames[i].getMessege();
                        message mess = message::zeroSplitStrToMess(data);
                        pool_->getNextLoop()->runInloop( bind(&messageServer::insertMessAndSend, this, conn, mess) );
                    }
                }
                else if (codec->frames[i].getOpcode() == 8)
                {
                    //结束帧
                }
                else if (codec->frames[i].getOpcode() == 9)
                {
                    //心跳帧
                    string data = "PONG";
                    printf("*debug* Pong");
                    conn->send(frame::toFrame(data, 10));
                }
                //codec->frames[i].debugPrint();
                //conn->send(frame::toFrameWithMask(codec->frames[i].getMessege(), 1, mark));
            }
            codec->frames.clear();
        }
    }

    void getMessAndSend(shared_ptr<Connection> conn, int mess_itermid)
    {
        MYSQL *sql = dbpool_->getConn();
        vector<message> messes;
        getMessagesByItem(mess_itermid, sql, &messes);
        dbpool_->putBack(sql);
        //在同一个线程中send可以保证不会乱序
        for (auto mess : messes)
        {
            //1是文本
            string data = mess.to_zeroSplit_str();
            conn->send(frame::toFrame(data, 1));
        }
    }

    void insertMessAndSend(shared_ptr<Connection> conn, message mess)
    {
        MYSQL *sql = dbpool_->getConn();
        insertMess(sql, mess);
        dbpool_->putBack(sql);
        set<shared_ptr<Connection> > conns = itemToConn_[mess.mess_itemid];
        for (auto conn : conns)
        {
            string message = mess.to_zeroSplit_str();
            conn->send(frame::toFrame(message, 1));
        }
    }

    void onDisConnection(shared_ptr<Connection> conn)
    {
        printf("*debug* onDisConnection\n");
        MutexGuard m(mutex_);
        for (auto &mapelem : itemToConn_)
        {
            set<shared_ptr<Connection> > &ref = mapelem.second;
            auto iter = ref.find(conn);
            if (iter != ref.end())
            {
                ref.erase(iter);
                break;
            }
        }
        auto citer = codecMap_.find(conn);
        if (citer != codecMap_.end())
        {
            codecMap_.erase(citer);
        }
    }

    void onConnection(shared_ptr<Connection> conn)
    {
        
    }

    void start()
    {
        serv_->start();
    }

    private:
    unique_ptr<Server> serv_;
    unique_ptr<ThreadLoopPool> pool_;
    unique_ptr<DbConnPool> dbpool_;
    map<int, set< shared_ptr<Connection> > > itemToConn_;
    map<shared_ptr<Connection>, shared_ptr<webSocketCodec> > codecMap_;
    Mutex mutex_;
};

int main(void)
{
    messageServer serv;
    serv.start();
}
