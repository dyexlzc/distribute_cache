/**
 * 微服务节点，主要是再main_serv启动后，指定网关服务器的地址端口动态注册到上面
 * 客户端请求可以被转发到这些服务器上，减小服务器压力
*/
#include "lite_serv.hpp"
#include "global_def.hpp"
#include <iostream>
#include <map>
using namespace std;
class peer_serv : lite_serv
{

    type_ptr_sock pserverSocket; //保存连接到主服务器的socket对象
    type_tcp::endpoint servEndpoint;
    unsigned char buf[BUFFER_MAX];
    std::map<std::string, std::string> cache_map;

public:
    struct peer_endpoint
    {
        string address;
        int port;
    };
    peer_endpoint server_endpoint; //储存网关服务器地址
    peer_serv(peer_endpoint &ep, int port) : lite_serv(port)
    {
        //启动节点
        //初始话服务器节点
        pserverSocket = type_ptr_sock(new type_tcp::socket(m_io_serv));
        server_endpoint.address = ep.address;
        server_endpoint.port = ep.port;
        servEndpoint = type_tcp::endpoint( //显式调用构造函数
            boost::asio::ip::address::from_string(ep.address),
            ep.port);
        msg(msg_type(info), boost::str((boost::format("core serv started in %1%") % port)));
    }
    void start()
    {
        move(thread([this] { //给服务器准备操作面板
            string cmd;
            while (1)
            {
                cin >> cmd;
                if (cmd == "q")
                {
                   
                    free(); //释放socket资源和地址
                    pserverSocket->close();
                    m_io_serv.stop();
                    exit(0); 

                }else if(cmd=="v"){
                    cout<<"current map:"<<endl;
                    map<string,string>::iterator it;
                    it=cache_map.begin();
                    while(it!=cache_map.end()){
                        cout<<it->first<<":"<<it->second<<endl;
                        it++;
                    }
                }
            }
        }))
            .detach();
        handler_connect_core(); //先连接到服务器，上线服务
       
    }
    void handler_connect_core()
    {
        //连接到中心服务器以获取信息
        //连接到中心服务器不用异步，一定要连接上才能处理其他客户端的请求
        
        msg(msg_type(info), "connecting...");
        pserverSocket->connect(servEndpoint); //连接服务器
        
            connect_msg msg;
            msg.peer_type = PEER_TYPE_SERVER;
            memcpy(buf,&msg,sizeof(msg));
            pserverSocket->write_some(boost::asio::buffer(&buf, sizeof(buf))); //发送连接请求
        
            //连接成功，获取消息
            client_msg re_msg; //获取回复消息

            pserverSocket->read_some(boost::asio::buffer(buf, sizeof(buf)));
            memcpy(&re_msg, buf, sizeof(re_msg)); //解析消息
            string _s(re_msg.data, strlen(re_msg.data));
            this->msg(msg_type(info), _s);
            
            this->msg(msg_type(info), "start listening...");
            //和网关服务器连接成功后，开始异步监听请求
            handler_listen();
            m_io_serv.run();
        
    }

    //////////////////////handler///////////////////////////////////////
    void handler_null(type_ptr_sock pserverSocket, err ec)
    {
        if (ec)
        {
            msg(msg_type(error), "null error : " + ec.message());
            return;
        }
    }
    void handler_cache(type_ptr_sock pserverSocket, unsigned char *buf, err ec)
    {
        handler_listen();
        cache_msg msg;
        memcpy(&msg, buf, sizeof(msg));
        //cout<<msg.data<<endl;
        /*cout<<"buf:"<<endl;
        for(int i=0;i<100;i++){
            cout<<(int)buf[i]<<" ";
        }
        cout<<endl;*/
        //cout<<msg.operate;
        switch (msg.operate)
        {
            case CACHE_GET:
            {
                cout<<"get"<<endl;
                unsigned char buf__[BUFFER_MAX];
                cache_msg re_msg;
                pair msg2;
                memcpy(&msg2,msg.data,sizeof(pair));//解析需要获取的键值
                string _key(msg2.key,strlen(msg2.key));
                if(cache_map.count(_key)==0)//判断是否有这个元素 
                {
                    re_msg.operate=CACHE_GET_NONE;
                }
                else
                {
                    re_msg.operate=CACHE_GET_EXIST;
                    strcpy(msg2.val,cache_map[_key].c_str());
                    cout<<"val:"<<msg2.val<<" ";
                }
                memcpy(re_msg.data,&msg2,sizeof(msg2)); //填充返回数组
                memcpy(buf__,&re_msg,sizeof(re_msg));

                
                cout<<msg2.key<<":"<<msg2.val<<endl;
               /* for(int i=0;i<100;i++){
                    cout<<"["<<(char)buf__[i]<<"]";
                }
                cout<<endl;
*/
                cout<<"start send"<<endl;
                pserverSocket->async_write_some(              //返回数据
                boost::asio::buffer(buf__,sizeof(buf__)),
                                    boost::bind(&peer_serv::handler_null,   //异步发送
                                                this,
                                                pserverSocket,
                                                boost::asio::placeholders::error)
                );
                cout<<"send end"<<endl;
                
                break;
            }
            case CACHE_SET:
            {   //设置键值对,包含了插入和更新的功能
                cout<<"set"<<endl;
                pair msg2;
                memcpy(&msg2,msg.data,sizeof(msg2));
                cache_map[msg2.key]=msg2.val;
                
                break;
            }
            case CACHE_DEL:
            {

                break;
            }
            default:{
                this->msg(msg_type(error),"unknown cache request.");
                break;
            }
        }
        return;
        
    }
    void handler_listen()
    {
        //接受cache的CRUD请求,在已经建立连接的pserverSocket上操作
        //网关服务器把缓存请求发送到peer
        pserverSocket->async_read_some(boost::asio::buffer(buf, sizeof(buf)),
                                     boost::bind(&peer_serv::handler_cache, this,
                                                 pserverSocket,
                                                 buf,
                                                 boost::asio::placeholders::error));
        
        
    }
    void free()
    {
        //清空资源，断开连接
        {
            this->msg(msg_type(info), "peer has cleaing...");
             //如果没有这行代码，服务器就会直接收到end of file 错误
            connect_msg msg_disconnect;
            msg_disconnect.peer_type = PEER_TYPE_DISCONNECT;
            memcpy(buf,&msg_disconnect,sizeof(msg_disconnect));
            pserverSocket->write_some(boost::asio::buffer(buf, sizeof(buf))); //发送一条消息给服务器，告诉他断开连接
            
            pserverSocket->close();
        }
    }
};
int main(int argc, char *argv[]) //通过参数连接服务器
{
    // 启动方式：peer_serv 192.168.1.108 1008
    if (argc < 2)
    {
        cout << "no address has filled" << endl;
        return 0;
    }
    peer_serv::peer_endpoint ep = {
        argv[1],
        atoi(argv[2])};
    peer_serv serv(ep, 1000 + lite_serv::randInt(1000));
    serv.start();
    return 0;
}