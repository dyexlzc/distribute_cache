CFLAGS = -Wall -g  
OTHRES = -lboost_system -lpthread -std=c++11
serv:main_serv.o peer_serv.o Distributed_cache.o
	g++ ${CFLAGS} main_serv.o -o main_serv ${OTHRES}
	g++ ${CFLAGS} peer_serv.o -o peer_serv ${OTHRES}
	g++ ${CFLAGS} Distributed_cache.o -o dcache ${OTHRES}
main_serv.o:main_serv.cpp ThreadPool.hpp lite_serv.hpp lite_serv.hpp global_def.hpp	#主节点
	g++ ${CFLAGS} -c main_serv.cpp ${OTHRES}
peer_serv.o:peer_serv.cpp lite_serv.hpp lite_serv.hpp global_def.hpp	#peer微服务节点
	g++ ${CFLAGS} -c peer_serv.cpp ${OTHRES}
Distributed_cache.o:lite_serv.hpp  global_def.hpp distribute_tools.hpp Distributed_cache.cpp  #客户端节点
	g++ ${CFLAGS} -c Distributed_cache.cpp ${OTHRES}
clean:
	rm -rf *.o