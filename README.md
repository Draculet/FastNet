# FastNet

FastNet是一个以C++11标准构建的 One Loop Per Thread 模型的轻量网络库。该网络库提供了事件框架、基于事件框架的计时器、事件线程线程池、连接管理、心跳等现成模块和功能，支持HTTP协议、WebSocket。目前FastNet仍处于构建和测试结合阶段，模块仍不完整，功能尚未完善


网络库结构：

类 | 概述
--|--
Poller|epoll的封装，提供事件注册接口
Channel|关注事件集合的封装，保存回调函数，提供回调接口
Buffer|非堵塞网络库必备的应用缓冲
Eventloop|事件循环的抽象，跨线程调用的核心，线程之间的桥梁
Connection|对TCP连接的抽象，提供对连接的操作
Socket|Socket的抽象
Server|各类的集合体、管理连接的生命周期
Timer|基于事件驱动的计时器

## 导航

- [背景](#背景)
- [安装](#安装)
- [使用](#使用)

## 背景

开始这个项目的原因是，每次当我准备编写一个网络程序时都需要从头开始编写出一个简单的程序框架，然后才开始在这个简陋的框架上写程序逻辑。由于精力大多放在程序逻辑上，这个简单的框架一般是由epoll+loop这样简单的模式构成，这使得编写成的程序多半吞吐量低下，而瓶颈就是应用层的IO。由于一开始就敲定这个网络库应以One Loop Per Thread为模型，因此项目初期是以大名鼎鼎的木铎库为模仿对象进行构建，很多设计思想和C++程序设计思路都源自木铎，我从木铎的源码中受益良多。项目事件框架完成后，我开始使用FastNet写应用，通过开发和测试同步进行的方式不断完善、稳定它，并且根据各种不同的需求开发完善它的功能和模块。目前FastNet仍处于这样的阶段，模块仍不完整，功能尚未完善。


## 安装

大多数程序示例都附带CMakeList,安装方式如下
```sh
$ cmake .
$ make
```


## 使用

FastNet由于是异步的、事件驱动的网络库，程序逻辑主要是通过编写回调函数的方式来表达的。下面是一个最简单的echo回调程序编写示例

```c++
void onRead(Buffer *buf, shared_ptr<Connection> conn)
{
    conn->send(buf->retrieveAllAsString());
}
```
主函数编写
```c++
int main(void)
{
    Server s(threadnum, port);
    s.setReadCallback(bind(onRead, placeholders::_1, placeholders::_2));
    s.start();
}

```


## 待开发清单
* 编写日志库，取代目前低效的日志打印方式
* 编写RPC框架
* 事件框架优化
* 基础设施封装
* 提供HTTPS支持
* ...