* 在socket中为Socket加入对client Connect的支持
* 在NetAddr中加入直接将ip:port对转化为sockaddr_t的方法
* Server中需要将连接保存在map中 //ok
* EventLoop中的eventfd错误使用 //ok
* Channel里的针对Connection的保护需测试
* Thread可以保存更多信息
* ThreadLoopPool,ThreadLoop待测试
* Channel中对EPOLL状态的处理未完全
* 注意在close()之前一定保证fd不在epoll中 详见https://blog.codingnow.com/2017/05/epoll_close_without_del.html
* 为保证remove fd, 是否在Server移除Connection之后再使Connecion Thread在判断状态,如果状态不为closed则移除fd?
* Channel需减少不必要的setXXCallback函数
* Channel构造函数的conn指针有问题
* weak_ptr不能由裸指针构造

* Connection.h中Channel传未构造完成的shared_ptr<Connection> core dump
* 需要测试是否Connection析构

* readv可能返回-1,不能用size_t,什么情况下返回-1? 打印的结果是connection reset by peer
* 注意一些可能返回-1的返回值不能用size_t存储,否则将达到一个非常大的数值
* 放入全局 ::signal(SIGPIPE, SIG_IGN);

* Connection先析构造成handleEvent调用到一半coredump

* shared_from_this包裹的类不能在类还未构造完成并被shared_ptr管理前使用