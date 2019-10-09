#ifndef GLOBAL_DEF_HPP
#define GLOBAL_DEF_HPP
#define INFO_SUCCESS 0x0001
#define INFO_ERR 0x0002
#define INFO_NORMAL 0x0003

#define PEER_TYPE_CLIENT 0x001
#define PEER_TYPE_SERVER 0x002
#define PEER_TYPE_DISCONNECT 0x100

#define CLIENT_OP_INFO 0x001

//缓存请求格式 
#define CACHE_GET 0x0001
#define CACHE_DEL 0x0002
#define CACHE_SET 0x0003

#define CACHE_GET_NONE 0x0010  //所获取的key不存在
#define CACHE_GET_EXIST 0x0020 //所获取的key存在 

#define BUFFER_MAX 204UL
#endif // GLOBAL_DEF_HPP