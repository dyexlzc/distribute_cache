/**
 * 打算使用asio来实现一个通用的微服务器，能够方便的拓展
 * 实际上是实现了一个封装好的服务器类
 * 其实就是把一些常用的写好，当然回调函数还是要自己写的
*/
#ifndef _LITE_SERV_HPP
#define _LITE_SERV_HPP
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <cstdlib>
#include <string>
#include <thread>
#include <time.h>
#include <cstdlib>
#include <exception>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>                //UUID 相关库
typedef boost::asio::ip::tcp type_tcp;                   //简化命名空间
typedef type_tcp::socket* type_ptr_sock; //智能指针
typedef boost::system::error_code err;                   //boost的错误处理
 
class lite_serv
{

public:
    struct connect_msg
    {
        int peer_type;   //说明此节点是服务器还是客户端  PEER_TYPE_CLIENT/PEER_TYPE_SERVER
        char data[200]; //一个节点可以包含2048B[2KB]的数据
    };
    struct client_msg
    {
        //客户端消息格式
        int operate;     //客户端请求的操作
        char data[150]; //客户端请求的数据大小
    };
    struct cache_msg
    {
        //客户端请求的cache操作
        int operate; //客户端请求的cache操作类型   CACHE_GET,CACHE_DEL,CACHE_SET
        char data[130];
    };
    struct pair
    {
        //键值对结构
        char key[50];
        char val[50];
    };
    static int randInt(int range)
    {
        srand((int)time(0) + rand()); //顺便设置一下服务器的随机数
        return rand() % range;
    }
    enum msg_type
    {
        info,
        error,
        success
    }; //枚举的消息类型
    void msg(msg_type type, std::string msg)
    {
        //向服务器控制台发送消息

        switch (type)
        {
        case info:
        {
            std::cout << "\033[34m"
                      << "【info】"
                      << "\033[37m\t" << msg << "\033[0m" << std::endl;
            break;
        }
        case error:
        {
            std::cout << "\033[31m"
                      << "【error】"
                      << "\033[37m\t" << msg << "\033[0m" << std::endl;
            break;
        }
        case success:
        {
            std::cout << "\033[32m"
                      << "【success】"
                      << "\033[37m\t" << msg << "\033[0m" << std::endl;
            break;
        }
        }
    }
    boost::asio::io_service m_io_serv;         //io服务
    boost::asio::ip::tcp::acceptor m_acceptor; //服务器对象
    lite_serv(int port) : m_acceptor(type_tcp::acceptor(m_io_serv, type_tcp::endpoint(type_tcp::v4(), port)))
    {
    }
    static int big_little()
    { //判断大小端,1大端2小端
        union p {
            int a;
            char b;
        };
        p _t;
        _t.a = 1;
        return _t.b == 0 ? 1 : 2; //1大端2小端
    }
    void run()
    {
        m_io_serv.run(); //开始epoll监听回调
    }
    void free()
    {
        
        m_acceptor.close();
        m_io_serv.stop();
    }
};
#endif