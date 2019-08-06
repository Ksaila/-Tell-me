#include <iostream>
#include "ChatServer.hpp"
// ./ChatServer tcp_port udp_port 

static void Usage(std::string proc)
{
    std::cout<<"Usage: "<<proc<<" tcp_port udp_port"<<std::endl;

}
void *RunProduct(void *arg)
{
    pthread_detach(pthread_self());
    ChatServer *sp = (ChatServer*)arg;
    for(;;){
	sp->Product();//生产	
    }
}
void *RunConsume(void *arg)
{
    pthread_detach(pthread_self());
    ChatServer *sp = (ChatServer*)arg;
    for(;;){
	sp->Consume();//消费
    }
}

int main(int argc, char *argv[])
{
    if(argc != 3){
	Usage(argv[0]);
	exit(1);
    }
    int tcp_port = atoi(argv[1]);
    int udp_port = atoi(argv[2]);

    ChatServer *sp = new ChatServer(tcp_port,udp_port);
    sp->InitServer();
    
    pthread_t c,p;
//接收数据并将数据放入数据池中
    pthread_create(&p,NULL,RunProduct,(void*)sp);
//将数据池中的数据发送给在线用户
    pthread_create(&c,NULL,RunConsume,(void*)sp);
    sp->Start();
    
    return 0;
}























