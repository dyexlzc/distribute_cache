# Distribute cache ��Ŀ

** dcache **
��Ŀ��ʼ���ڣ�2019-9-28
�����Ŀ�����������ļ���   main_serv.cpp peer_serv.cpp  Distribute_cache.cpp
main_serv.cpp��				���ط���������������ڵ�������Ͳ�ѯ�ڵ�Ľ���
peer_serv.cpp��				��������������ڷֵ�����������ѹ��
Distribute_cache.cpp		 ��ѯ�ڵ㣬���������ط�������������

Ŀǰ���Ϊ boost-1.58

*������װ��*
ϵͳ����deepin 15.11������
g++/gcc   :g++/gcc (Debian 6.3.0-18+deb9u1) 6.3.0 20170516 ��һ�����Դ��ġ�
boost-1.58   :libboost-dev��һ�㲻�Դ���ʹ��apt-get install libboost-dev  ��װ��

*��������:*
make
������Ŀ¼������main_serv    peer_serv  dcache �����ļ�


*��������:*
main_serv  �������ط�����
peer_serv  0.0.0.0  1007  ����ڵ����ӵ����ط������Ķ˿ڣ��ڷ���������
dcache 0.0.0.0 1007 ���ӵ����ط������Ķ˿�
