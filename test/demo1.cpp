#include <mymuduo/TcpServer.h>
#include <string>

class ChatServer
{
public:
    ChatServer(EventLoop *loop, InetAddress &listenAddr, const std::string &nameArg)
        : _server(loop, listenAddr, nameArg), _loop(loop)
    {
        _server.setConnectionCallbck(std::bind(&ChatServer::onConnection, this, std::placeholders::_1));
        _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        _server.setThreadNums(4);
    }

    void start()
    {   
        _server.start();
    }
private:
    void onConnection(const TcpConnectionPtr &connPtr) 
    {
        if(connPtr->connected())
            std::cout << "peerAddress" << connPtr->peerAddress().toIpPort() << "->" << "localAddress" << connPtr->localAddress().toIpPort() << " state: Online" <<std::endl;
        else{
            std::cout << "peerAddress" << connPtr->peerAddress().toIpPort() << "->" << "localAddress" << connPtr->localAddress().toIpPort() << " state: Offline" << std::endl;
            connPtr->shutdown();
        }
    }

    void onMessage(const TcpConnectionPtr &connPtr,Buffer *buf,Timestamp Timestamp)
    {
        std::string msg(buf->retrieveAllAsString());
       
        connPtr->send(msg);
    }

    TcpServer _server; // epoll_ctl
    EventLoop *_loop;
};

int main()
{
    EventLoop loop; // epoll_create
    InetAddress listenAddr(6666);
    ChatServer server(&loop,listenAddr,"EchoServer");
    server.start();
    loop.loop(); // epoll_wait
    return 0;
}