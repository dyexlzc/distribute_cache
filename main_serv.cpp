/**
 * 中心节点，用于处理和转发客户端的请求
*/
#include "lite_serv.hpp"
#include "global_def.hpp"
#include "ThreadPool.hpp" //引入线程池，在查询的时候必须用线程查询，否则会导致回调阻塞
#include <iostream>
#include <vector>
#include <unistd.h>
using namespace std;
class main_serv : lite_serv
{
    bool write_tok;
    unsigned char buf[BUFFER_MAX]; //公共缓冲区
    struct peer_endpoint
    {
        string address;
        int port;
        type_ptr_sock psocket;
    };
    std::vector<type_ptr_sock> peer_list; //保存每一个节点
    std::vector<type_ptr_sock> client_list;//保存大家的ptr,不然放在函数里都析构掉了
    ThreadPool tpool;                     //用于查询的线程池

    class cache_job : public ThreadPool::job
    { //用于向服务器查询键值对
        main_serv *main_ptr;
        type_ptr_sock psocket;
        cache_msg ptr_cache_msg;

    public:
        cache_job(main_serv *p, type_ptr_sock _psocket, cache_msg _ptr_cache_msg)
        {
            main_ptr = p;
            psocket = _psocket;
            ptr_cache_msg = _ptr_cache_msg;
        }
        void run()
        {
            //使用boost的deadline_time
           // std::cout << "in thread:" << ptr_cache_msg->operate << std::endl;
            int n = main_ptr->peer_list.size();
            struct cache_msg temp_msg;
            unsigned char buf__[BUFFER_MAX];
            cout<<"start select.."<<endl;
            memcpy(buf__, &ptr_cache_msg, sizeof(cache_msg));
            cout<<"buf:"<<endl;
            for(int i=0;i<70;i++){
                 cout<<"["<<buf__[i]<<"]";
            }
            cout<<endl;
            for (int i = 0; i < n; i++)
            {
                cout<<i<<endl;
                //向每一个服务器发送消息查询
                main_ptr->peer_list[i]->write_some(boost::asio::buffer(
                    buf__,
                    sizeof(buf__)));
                cout<<"send end"<<endl;
                
                main_ptr->peer_list[i]->read_some(boost::asio::buffer(buf__, sizeof(buf__))); //接受消息
                memcpy(&temp_msg, buf__, sizeof(temp_msg));
                cout<<"receive end"<<endl;
                if (temp_msg.operate == CACHE_GET_NONE)
                {
                    //这个服务器不存在
                    continue;
                }
                //如果存在，就直接向客户端发送回h去,按照cache_msg的格式,然后客户端那边再解析
                else
                {
                    
                    psocket->write_some(boost::asio::buffer(
                                    buf__,
                                    sizeof(buf__))
                                );
                    cout<<"end select.."<<endl;
                    //main_ptr->handler_recv(psocket);
                    return; //结束线程
                }
            }
            //如果全部都不存在就发送CACHE_GET_NONE回去
            memcpy(buf__, &temp_msg, sizeof(temp_msg));
            psocket->write_some(boost::asio::buffer(
                buf__,
                sizeof(buf__)));
            //main_ptr->handler_recv(psocket);
            cout<<"end select.."<<endl;
            return; //结束线程
        }
    };

public:
    void handler_doing(type_ptr_sock psocket, unsigned char *buf_in, err ec)
    {
        //处理节点发送的数据
        if (ec)
        {
            msg(msg_type(error), "accept error : " + ec.message());
            return;
        }
        else
        {
            write_tok=false;
            connect_msg msg1; //接受并且解析消息
            memcpy(&msg1, buf_in, sizeof(connect_msg));    //从缓冲区读取数据
           // cout << "peer type:" << msg1.peer_type << endl;
            switch (msg1.peer_type)
            {
            case PEER_TYPE_CLIENT:
            {
                //如果是客户端发来的请求，就解析data里的数据
                
                cache_msg msg2;
                //cout<<"cpy start"<<endl;
                memcpy(&msg2, msg1.data, sizeof(cache_msg));
                // cout<<"cpy end"<<endl;
               // cout << "main op:" << msg2.operate << endl;
                switch (msg2.operate)
                {
                    //判断客户端的缓存请求
                    case CACHE_GET:
                    {
                        //如果客户端要获得某一个缓存，则需要分别到每一个服务器查询
                        //在线程池中查询，避免卡死主线程
                        //cache_job job(this, psocket, msg2);
                        tpool.addTask(*new cache_job(this, psocket, msg2));
                        //memset(buf,0,sizeof(buf));
                        //cout << "add task" << endl;
                        
                        usleep(5000);
                        handler_recv(psocket);
                        //sleep(1);
                        //return;//不在这里调用recv，因为可能peer还没发送完毕
                        return;
                        cout<<"after return"<<endl;
                        break;
                    }
                    case CACHE_SET:
                    {
                        //设置/更新键值对
                        //直接发送给节点处理
                        //随机选取一个节点，平衡负载
                        int peer_int=randInt(peer_list.size());
                        pair ttt;
                        memcpy(&ttt,msg2.data,sizeof(ttt));
                        cout<<boost::format("set:%1%:%2%")%ttt.key%ttt.val<<endl;
                        memcpy(buf,&msg2,sizeof(msg2));
                        
                        peer_list[peer_int]->async_write_some(
                            boost::asio::buffer(buf, sizeof(buf)),
                        // boost::asio::buffer("aaaaa", sizeof("aaaaa")),
                            boost::bind(&main_serv::handler_null, this,
                                        peer_list[peer_int],
                                        boost::asio::placeholders::error));
                    }
                    case CACHE_DEL:
                    {

                        break;
                    }
                    default:{
                        msg(msg_type(error),boost::str(boost::format("unknown type [%1%] and buf")%msg2.operate));
                        break;
                    }
                }
                handler_recv(psocket);
                return;
            }
            case PEER_TYPE_SERVER:
            {
                /**
                     * 如果是节点服务器，则与节点服务器建立连接，并且保存其远程地址，
                     * 当客户端连接的时候检查这个列表，如果没有则表示暂时没有微服务上线，
                     * 如果有则断开与main_serv的连接，和peer_serv建立连接
                    */
                peer_endpoint pend = {
                    psocket->remote_endpoint().address().to_string(),
                    psocket->remote_endpoint().port(),
                    psocket};
                //type_ptr_sock ppp(psocket);
                peer_list.push_back(psocket); //添加服务器节点进去，因此在客户端连接的时候，可以从里面分配不同的服务器地址
                //通知peer节点：上线成功
                msg(msg_type(success), boost::str(boost::format("peer %1%:%2% has connected.") % pend.address % pend.port));
                client_msg re_msg; //返回给peer服务器的消息
                re_msg.operate = CLIENT_OP_INFO;
                string _s = boost::str(boost::format("you has conncted to serv %1%:%2%.") % psocket->local_endpoint().address().to_string() % psocket->local_endpoint().port());
                strcpy(re_msg.data, _s.c_str());
                //sleep(1);
                memcpy(buf, &re_msg, sizeof(re_msg));
                //设置缓冲区，看看能不能缓解堵塞的情况
                /*boost::asio::socket_base::send_buffer_size size_option(512*50);
                psocket->set_option(size_option);
                boost::asio::socket_base::receive_buffer_size size_option2(512*50);
                psocket->set_option(size_option2);*/

                psocket->async_write_some(boost::asio::buffer(buf, sizeof(buf)), //发送消息给peer服务器
                                    boost::bind(&main_serv::handler_null, this,
                                                psocket,
                                                //true,//true是保存peer服务器节点
                                                boost::asio::placeholders::error));

               /* ppp->async_send(boost::asio::buffer(buf, sizeof(buf)), //发送消息给peer服务器
                                    boost::bind(&main_serv::handler_null, this,
                                                psocket,
                                                boost::asio::placeholders::error));*/

                break;
            }
            case PEER_TYPE_DISCONNECT:
            {
                /**
                     * 如果客户端提出断开连接的请求，就处理
                    */
                msg(msg_type(info), boost::str(boost::format("%1%:%2% has disconnected.") % psocket->remote_endpoint().address().to_string() % psocket->remote_endpoint().port()));
                for(auto it=peer_list.begin();it!=peer_list.end();it++){  //将节点从列表中删除
                    if(*it==psocket){
                        peer_list.erase(it);
                        break;
                    }
                }
                psocket->shutdown(boost::asio::socket_base::shutdown_both);
                psocket->close();
                return;
            }
            default:{
                        msg(msg_type(error),boost::str(boost::format("unknown peer_type [%1%] and buf")%msg1.peer_type));
                        break;
            }
            }
        }

        handler_recv(psocket); //处理完消息后，就会到这个回调函数，一定要循环处理这个psocket上的内容，否则不会成功

    
    }
    void handler_recv(type_ptr_sock psocket){
        static int i=0;
        cout<<"recv count:"<<i++<<endl;
        
        {
            //if(write_tok==true)
            //接受节点并且读取数据
            psocket->async_read_some(boost::asio::buffer(buf, sizeof(buf)), boost::bind(&main_serv::handler_doing, this,
                                                                                      psocket,
                                                                                      buf,
                                                                                      boost::asio::placeholders::error));
        }
    }
    void handler_accept(type_ptr_sock psocket, err ec)
    {
        
        if (ec)
        {
            // cout<<"accept error : "<<ec.message()<<endl;
            msg(msg_type(error), "accept error : " + ec.message());
        }
        else
        {
            //if(write_tok==true)
            //接受节点并且读取数据
            /*psocket->async_read_some(boost::asio::buffer(buf, sizeof(buf)), boost::bind(&main_serv::handler_doing, this,
                                                                                      psocket,
                                                                                      buf,
                                                                                        boost::asio::placeholders::error));*/
            handler_listen(); //继续监听
            handler_recv(psocket);
        }
        
        
    }
    void handler_listen()
    {
        //监听客户端请求,不管是peer服务器还是客户端都会通过这个转发
        //static int i = 0;
        //cout << i++ << endl;
        //type_ptr_sock psocket(new type_tcp::socket(m_io_serv));
        client_list.push_back(new type_tcp::socket(m_io_serv));
        type_ptr_sock psocket=client_list[client_list.size()-1];
        m_acceptor.async_accept(*psocket, boost::bind(
                                              &main_serv::handler_accept,
                                              this,
                                              psocket,
                                              boost::asio::placeholders::error));
        
    }

    void handler_null(type_ptr_sock psocket,err ec)
    {
        if (ec)
        {
            msg(msg_type(error), "null error : " + ec.message());
            return;
        }
        //cout<<"null func"<<endl;
        
    }
    main_serv(int port) : lite_serv(port),write_tok(true)
    {
        //cout << "core serv started in " << port << endl;
        msg(msg_type(info), boost::str((boost::format("core serv started in %1%") % port)));
        msg(msg_type(info),boost::str(boost::format("has %1% thread.")%tpool.core_num()));
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
                    tpool.free();
                    for (unsigned i = 0; i < peer_list.size(); i++)
                    {
                        peer_list[i]->close();
                    }
                    this->free(); //释放socket资源和地址
                }
            }
        }))
            .detach();
        handler_listen();
        run(); //启动epoll监听
    }
};
int main()
{
    main_serv serv(1008);
    serv.start();
    return 0;
}