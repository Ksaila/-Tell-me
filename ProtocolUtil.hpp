#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "json/json.h"
#include "Log.hpp"
#define BACKLOG 5
#define MESSAGE_SIZE 1024 
class Request{
    public:
	std::string method;// REGISTER, LOGIN, LOGOUT
	std::string content_length;//报文的长度
	std::string blank;//空行
	std::string text;//正文
    public:
	Request():blank("\n")
	{}
	~Request()
	{}
};
class Util{
    public:
	//注册时候的提示的信息
    static bool RegisterEnter(std::string &n_,std::string &s_,std::string &passwd)
    {
	std::cout<<"Please Enter Nick Name: ";
	std::cin >>n_;
	std::cout<<"Please Enter School: ";
	std::cin >>s_;
	std::cout<<"Please Enter Passwd: ";
	std::cin >>passwd;
	std::string again;
	std::cout<<"Please Enter Passwd Again: ";
	std::cin >> again;
	if(passwd == again){
	    return true;
	}
	return false;
    }
	//登录的提示信息 
    static bool LoginEnter(unsigned int &id,std::string &passwd)
    {
	std::cout<< "Please Enter Your ID: ";
	std::cin>>id;
	std::cout<< "Please Enter Your Passwd: ";
	std::cin>>passwd;
        return true;
    }
	//序列化
    static void Seralize(Json::Value &value,std::string &outString)
    {
	Json::FastWriter w;
	outString = w.write(value);//序列化
    }
	//反序列化
    static void UnSeralize(std::string &inString,Json::Value &value)
    {
	Json::Reader r;
	r.parse(inString,value,false);//反序列化
    }
	//将数据从int转换为char型
    static std::string IntToString(int x)
    {
	std::stringstream ss;
	ss << x;
	return ss.str();
    }
	//将数据从字符型转换为int型
    static int StringToInt(std::string &str)
    {
 	int x;
	std::stringstream ss(str);
	ss >> x;
	return x;
    }
	//接收数据
    static void RecvOneLine(int sock,std::string &outString)
    {
  	char c = 'x';
	while(c != '\n'){
	    ssize_t s = recv(sock,&c,1,0);
	    if(s > 0){
		if(c == '\n'){
		    break;
		}
		outString.push_back(c);
	}
	else{
	    break;
    	}
        }
    }
 	//TCP接收数据，并且解析数据包	
    static void RecvRequest(int sock,Request &rq)
    {
	RecvOneLine(sock,rq.method);
	RecvOneLine(sock,rq.content_length);
	RecvOneLine(sock,rq.blank);
	//取出其中的正文的长度
	std::string &cl = rq.content_length;//获得这一行字符串 content_length: 55
	std::size_t pos = cl.find(": ");//找到“：”的位置准备获得后面的长度 
 	if(std::string::npos == pos){
	    return;
	} 
	//现：的后面的第二位开始时正文的数据长度
	std::string sub = cl.substr(pos+2);
	//将取出的正文的长度转换为整形
	int size = StringToInt(sub);
	char c;//用作接受字符
	//知道了正文的长度，接收正文
	for(auto i = 0;i < size;i++)
	{
		//一次接收一个字符
	    recv(sock,&c,1,0);
		//保存在正文中
	    (rq.text).push_back(c);
	}
	
    }
	//TCP发送数据
	static void SendRequest(int sock,Request &rq)
	{
	    std::string &method_ = rq.method;
	    std::string &c_l = rq.content_length;
	    std::string &b = rq.blank;
	    std::string &text = rq.text;
	    send(sock,method_.c_str(),method_.size(),0);
	    send(sock,c_l.c_str(),c_l.size(),0);
	    send(sock,b.c_str(),b.size(),0);
	    send(sock,text.c_str(),text.size(),0);
	}
	//UDP接收数据
	static void RecvMessage(int sock, std::string &message,struct sockaddr_in &peer)
	{
	    char msg[MESSAGE_SIZE];//定长的报文
	    socklen_t len = sizeof(peer);
	    ssize_t s = recvfrom(sock,msg,sizeof(msg)-1,0,\
		    (struct sockaddr*)&peer,&len);//接受报文
	    if(s <= 0){
		LOG("recvfrom message error",WARNING);
	    }
	    else{
		message = msg;
	    }
	}
	//UDP发送数据
	static void SendMessage(int sock, const std::string &message,struct sockaddr_in &peer)
	{
	    sendto(sock,message.c_str(),message.size(),0,\
		    (struct sockaddr*)&peer,sizeof(peer));
	}
	//判断这个用户在不在在线链表中，如果不在的话，将他添加到在线用户链表中
	static void addUser(std::vector<std::string> &online,std::string &f)
	{
	    for (auto it = online.begin();it != online.end(); it++)
	    {
	        if(*it == f){
		    return;
	        }
	    }
		online.push_back(f);
	}	
	static void SubUser(std::vector<std::string> &online,std::string &f)
	{
		for(auto it = online.begin(); it != online.end(); it++)
		{
			if(*it == f)
			{
				online.erase(it);
				break;
			}
		}
	}
};

class SocketApi{
    public:
	static int Socket(int type)
	{
	   int sock = socket(AF_INET,type,0);
	   if(sock < 0){
	       LOG("socket error!",ERROR);
		exit(2);    
            }
	   
	}
	static void Bind(int sock,int port)
	{
	    struct sockaddr_in local;
	    local.sin_family = AF_INET;
	    local.sin_addr.s_addr = htonl(INADDR_ANY);
	    local.sin_port = htons(port);
	    
  	    if(bind(sock,(struct sockaddr*)&local,sizeof(local)) < 0)
		{
			LOG("socket error",ERROR);
			exit(3);
		
		}
	}
 	 static void Listen(int sock)
	{
	  if(listen(sock,BACKLOG) < 0){
	     LOG("Listen error",ERROR);
	     exit(4);   
	  }
	}
	static int Accept(int listen_sock,std::string &out_ip,int &out_port)
	{
	   struct sockaddr_in peer;
	   socklen_t len = sizeof(peer);
	   int sock = accept(listen_sock,(struct sockaddr*)&peer,&len);
	   if(sock < 0){
	       LOG("accept error",WARNING);
	       return -1;
   	   }
	   out_ip = inet_ntoa(peer.sin_addr);
	   out_port = htons(peer.sin_port);
	   return sock;
	}
	static bool Connect(int sock, std::string peer_ip,int port)
	{
	    struct sockaddr_in peer;
	    peer.sin_family = AF_INET;
	    peer.sin_addr.s_addr = inet_addr(peer_ip.c_str());
	    peer.sin_port = htons(port);

	    if(connect(sock, (struct sockaddr*)&peer,sizeof(peer)) < 0 ){
		LOG("connect error",WARNING);
		return false;
	    }
	    return true;
	}

};










































