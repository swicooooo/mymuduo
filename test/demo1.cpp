#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>

using namespace muduo;
using muduo::net::TcpServer;

class ChatServer
{
public:
    ChatServer(net::EventLoop *loop, const net::InetAddress &listenAddr, const string &nameArg)
        : _server(loop, listenAddr, nameArg), _loop(loop)
    {
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, std::placeholders::_1));
        _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        _server.setThreadNum(4);
    }

    void start()
    {   
        _server.start();
    }
private:
    void onConnection(const net::TcpConnectionPtr &connPtr) 
    {
        if(connPtr->connected())
            std::cout << "peerAddress" << connPtr->peerAddress().toIpPort() << "->" << "localAddress" << connPtr->localAddress().toIpPort() << " state: Online" <<std::endl;
        else{
            std::cout << "peerAddress" << connPtr->peerAddress().toIpPort() << "->" << "localAddress" << connPtr->localAddress().toIpPort() << " state: Offline" << std::endl;
            connPtr->shutdown();
        }
    }

    void onMessage(const net::TcpConnectionPtr &connPtr,net::Buffer *buf,Timestamp timestamp)
    {
        std::string msg(buf->retrieveAllAsString());
        std::cout << "recv data: " << msg << " time: " << timestamp.toFormattedString() << std::endl;
        connPtr->send(msg);
    }

    TcpServer _server; // epoll_ctl
    net::EventLoop *_loop;
};

int main()
{
    net::EventLoop loop; // epoll_create
    const net::InetAddress listenAddr("127.0.0.1", 6666);
    ChatServer server(&loop,listenAddr,"EchoServer");
    server.start();
    loop.loop(); // epoll_wait
    return 0;
}