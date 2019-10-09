#include "distribute_tools.hpp"
#include "lite_serv.hpp"
#include "global_def.hpp"
#include <iostream>
#include <boost/format.hpp>
/**
 * 分布式缓存的客户端，用于和服务器连接
*/
using namespace std;
int main(int argc, char *argv[]){
     // 启动方式：xxx 192.168.1.108 1008
    if (argc < 2)
    {
        cout << "no address has filled" << endl;
        return 0;
    }
    dcache conn(argv[1],atoi(argv[2]));
    char key[5],val[5];
    for(int i=0;i<20;i++){
        strcpy(key,boost::str(boost::format("%1%")%char(i+'a')).c_str());
        strcpy(val,boost::str(boost::format("%1%")%i).c_str());
        cout<<key<<":"<<val<<endl;
        conn.set(key,val);
    }
    string res;
    while(1)
    {cin>>key;
    if(!strcmp(key,"~"))break;
    conn.get(key,res);
    cout<<res;}


    return 0;
}