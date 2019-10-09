/**
 * 分布式缓存工具
 * */
#ifndef _DISTRIBUTE_TOOLS_HPP
#define _DISTRIBUTE_TOOLS_HPP
#include <iostream>
#include <string>
#include "lite_serv.hpp"
#include "global_def.hpp"
#include <boost/asio.hpp>
class dcache{
    boost::asio::io_service m_io;
    type_tcp::endpoint ep;
    type_ptr_sock psocket;
    unsigned char buf[BUFFER_MAX]; //缓冲区
public:
    dcache(std::string address,int port):ep(boost::asio::ip::address::from_string(address.c_str()),port){
        psocket=type_ptr_sock(new type_tcp::tcp::socket(m_io));
        psocket->connect(ep);//连接上服务器
    }
    ~dcache(){
        //用于释放和服务器的连接
        std::cout<<"free";
        psocket->close();
    }
    void set(const char* key,const char* val){
        
        lite_serv::connect_msg msg_conn;//向网关服务器报告自己的信息
        msg_conn.peer_type=PEER_TYPE_CLIENT;

        lite_serv::pair data1; //填充键值对
        strcpy(data1.key,key);
        strcpy(data1.val,val);
        
        lite_serv::cache_msg msg;
               msg.operate=CACHE_SET; 
        memcpy(msg.data,&data1,sizeof( lite_serv::pair));//打包数据

        
        memcpy(msg_conn.data,&msg,sizeof(lite_serv::cache_msg));
        memcpy(buf,&msg_conn,sizeof(msg_conn));//应该是粘包的问题，发送数据偶尔成功，现在固定每一条消息都是2048*2字节

        psocket->write_some(boost::asio::buffer(buf,sizeof(buf)));//发送
        usleep(100);
    }
    void get(const char* key,std::string &res){
        using namespace std;
       // char buf__[BUFFER_MAX];
        lite_serv::connect_msg msg_conn;//向网关服务器报告自己的信息
        msg_conn.peer_type=PEER_TYPE_CLIENT;

        lite_serv::pair data1;
        strcpy(data1.key,key);

        lite_serv::cache_msg msg;
        msg.operate=CACHE_GET;

        memcpy(msg.data,&data1,sizeof( lite_serv::pair));//打包数据
        memcpy(msg_conn.data,&msg,sizeof(lite_serv::cache_msg));
        memcpy(buf,&msg_conn,sizeof(msg_conn));

        memcpy(&msg_conn,buf,sizeof(msg_conn));
        cout<<"client peer_type:"<<(int)msg_conn.peer_type<<endl;
        /*cout<<"buf:"<<endl;
        for(int i=0;i<70;i++){
            cout<<"["<<buf__[i]<<"]";
        }
        cout<<endl;*/
        psocket->write_some(boost::asio::buffer(buf,sizeof(buf)));//发送
        usleep(100);
        //////////////////////////////////////////
        psocket->read_some(boost::asio::buffer(buf,sizeof(buf)));//接受数据
        memcpy(&msg,buf,sizeof(msg));
        switch(msg.operate)
        {
            case CACHE_GET_NONE:{
                res="not exist";
                break;
            }
            case CACHE_GET_EXIST:{
                memcpy(&data1,msg.data,sizeof(lite_serv::pair));
                res=data1.val;
            }
        }
    }
};
#endif