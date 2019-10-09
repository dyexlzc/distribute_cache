# Distribute cache 项目  

** dcache **  
项目开始日期：2019-9-28  
这个项目包含了三个文件：   main_serv.cpp peer_serv.cpp  Distribute_cache.cpp  
main_serv.cpp：				网关服务器，负责处理缓存节点服务器和查询节点的接入  
peer_serv.cpp：				缓存服务器，用于分担主服务器的压力  
Distribute_cache.cpp		 查询节点，用于向网关服务器发起请求  
  
目前框架为 boost-1.58  

*依赖安装：*  
系统是在deepin 15.11开发的  
g++/gcc   :g++/gcc (Debian 6.3.0-18+deb9u1) 6.3.0 20170516 【一般是自带的】  
boost-1.58   :libboost-dev【一般不自带，使用apt-get install libboost-dev  安装】  
  
*编译命令:*  
make   
即可在目录下生成main_serv    peer_serv  dcache 三个文件  


*启动参数:*   
main_serv  启动网关服务器  
peer_serv  0.0.0.0  1007  缓存节点连接到网关服务器的端口，在服务器上线  
dcache 0.0.0.0 1007 连接到网关服务器的端口  
