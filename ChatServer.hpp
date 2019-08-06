#pragma once
#include <iostream>
#include <pthread.h>
#include "ProtocolUtil.hpp"
#include "UserManager.hpp"
#include "Log.hpp"
#include "Window.hpp"
#include "DataPool.hpp"
#include "Message.hpp"
//服务器类的声明
class ChatServer;
class Param {
    public:
       ChatServer *sp;
       int sock;
       std::string ip;
       int port;
    public:
	Param(ChatServer *sp_,int &sock_,const std::string &ip_,const int &port_):
		sp(sp_),
		sock(sock_),
		ip(ip_),
		port(port_)
	{}
	~Param()
	{}
    
};

class ChatServer{
    private:
	int tcp_listen_sock;
    	int tcp_port;

	int udp_work_sock;
	int udp_port;

	UserManager um;
	DataPool pool;
    public:
	//构造函数
	ChatServer(int tcp_port_ = 8080,int udp_port_ = 8888):
	    tcp_port(tcp_port_),
	    tcp_listen_sock(-1),
	    udp_port(udp_port_),
	    udp_work_sock(-1)
	{}
	//初始化套接字
	//tcp的话需要创建、绑定、监听
	//udp的话只需创建和绑定
	void InitServer()
	{
  	    tcp_listen_sock = SocketApi::Socket(SOCK_STREAM);//创建套接字
  	    udp_work_sock = SocketApi::Socket(SOCK_DGRAM);
	    SocketApi::Bind(tcp_listen_sock,tcp_port);//绑定
	    SocketApi::Bind(udp_work_sock,udp_port);

	    SocketApi::Listen(tcp_listen_sock);
	    
	}
	//将注册的用户的信息添加到用户列表中
	unsigned int RegisterUser(const std::string &name,const std::string &school,const std::string &passwd)
	{
	    return um.Insert(name,school,passwd);//注册的用户的基本信息    
	}
	//检测登陆的用户的id和密码是否正确
	unsigned int LoginUser(const unsigned int &id, const std::string &passwd,const std::string &ip,int port)
	{
	    return um.Check(id,passwd);
	}
	void SubUser(unsigned int id)
	{	
		um.SubOnlineUser(id);
	}
	//接收数据
	void Product()
	{
	    std::string message;
	    struct sockaddr_in peer;
	    //接收客户端发送来的消息
	    Util::RecvMessage(udp_work_sock,message,peer);
	    std::cout<< "debug: recv message: "<<message<<std::endl;
	    Message ms;
	    ms.ToRecvValue(message);
		//关机处理
	    if(strcmp(ms.Text().c_str(),"init 0") == 0)
	    {
		std::string s("quit");
                Message m(ms.NickName(),ms.School(),s,ms.Id());
                std::string send;
                m.ToSendString(send);
                sendto(udp_work_sock,send.c_str(),send.size(),0,(struct sockaddr*)&peer,sizeof(peer));
		um.SubOnlineUser(ms.Id());
            }
	    //如果收到的数据不为空的话，
	    if(!message.empty())
	    {		
		Message m;
		//将这个接收到的信息反序列化，将其中的信息提取出来
		m.ToRecvValue(message);
		if(m.Type() == LOGIN_TYPE)
		{
		    //如果这个用户是刚登陆的话，加这个用户添加到在线用户表中
	            um.AddOnLineUser(m.Id(),peer);
		    std::string name_,school_;
		    //获取登录用户的信息
		    um.GetUserInfo(m.Id(),name_,school_);
		    //创建一条新的信息
		    Message new_msg(name_,school_,m.Text(),m.Id(),m.Type());
		    //将信息序列化
		    new_msg.ToSendString(message);  		   
		}
	    }
	    //将数据放到数据池中
	    pool.PutMessage(message);
	}
	//从数据池中拿数据发送
	void Consume()
	{
	    std::string message;
	    //新创建一条消息
	    pool.GetMessage(message);
	    std::cout<<"debug: send massage: "<<message<<std::endl;
	    auto online = um.OnLineUser();//获取用户在线列表
	    for(auto it = online.begin(); it != online.end(); it++){
		Util::SendMessage(udp_work_sock,message,it->second);//通过udp向在线列表中的用户发送信息
	    }
	}
	//对客户端的请求进行处理
	static void *HandlerRequest(void* arg)
	{
	    Param *p = (Param*)arg;
            //首先将信息进行分开保存
	    int sock = p->sock;
	    ChatServer *sp = p->sp;
	    std::string ip = p->ip;
	    int port = p->port;
	    delete p;
	    //线程分离，因为我们不关心他的退出状态，所以直接线程分离，也就不需要线程等待
	    pthread_detach(pthread_self());
	    //开始读取客户端发送的消息
            Request rq;
            Util::RecvRequest(sock,rq);
            Json::Value root;
	    //将这个消息打印出来
	    LOG(rq.text,NORMAL);
	    //将这个接收到的字符串反序列化
	    Util::UnSeralize(rq.text,root);
	    //如果他是一个注册的请求的话
	    if(rq.method == "REGISTER"){
		//将用户的个人信息保存起来
		std::string name = root["name"].asString();
		std::string school = root["school"].asString();
		std::string passwd = root["passwd"].asString();
		//将数据放入到用户表中，如果这个是新的用户的话，给他返回一个登录的id
		unsigned int id = sp->RegisterUser(name,school,passwd);//注册的id
	        send(sock,&id,sizeof(id),0);//返回注册的id
	    }
	    //如果这是一个登录的请求的话
	    //取出信息中的id和密码进行判断，如果id和密码都正确的话，将这个用户放到在线用户表中
	    else if(rq.method == "LOGIN"){
		//取出id和密码
		unsigned int id = root["id"].asInt();
		std::string passwd = root["passwd"].asString();
		//检验，将用户放入在线列表中，如果id或者密码错误的话，返回一个-1，如果没错的话返回他的id
		unsigned int ret = sp->LoginUser(id,passwd,ip,port);
		//将检测的返回值发送给客户端，好让客户端检测是否发送成功
		send(sock,&ret,sizeof(ret),0);
	    }
	    //退出
	    //将退出的用户从在线用户表中去除
	    //返回退出
	    else{
	//	Window w;
		unsigned int id = root["id"].asInt();
		sp->SubUser(id);
	//	w.PutUserToOnline(sp->um.OnLineUser());
	    }            
	    close(sock);
	}
	//服务器开始运行
	void Start()
	{
	    std::string ip;
	    int port;
	    for(;;){
		//接收由客户端发送的链接
	        int sock = SocketApi::Accept(tcp_listen_sock,ip,port);
	        if(sock > 0){
		    std::cout <<"get a new client"<<ip<<":"<<port<<std::endl;
		     
		    Param *p = new Param(this,sock,ip,port); 
		    pthread_t tid;
		    //新创建一个线程去执行这个客户端这个链接的处理
		    pthread_create(&tid,NULL,HandlerRequest,p);
	        }
	    }
	}
	~ChatServer()
	{}

};














































