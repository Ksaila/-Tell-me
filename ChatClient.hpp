#pragma once
#include <signal.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include "ProtocolUtil.hpp"
#include "Message.hpp"
#include "Window.hpp"
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/types.h>
#define TCP_PORT 8080
#define UDP_PORT 8888


class ChatClient;

void Menu(int &s)
{

    std::cout<<"################################################"<<std::endl;
    std::cout<<"####  1. Register                 2.Login  #####"<<std::endl;
    std::cout<<"####                              3.Exit   #####"<<std::endl;
    std::cout<<"################################################"<<std::endl;
    std::cout<<"Please Select:>";
    std::cin>>s;
}

void Select(std::string ip);
std::string Name;

struct ParamPair{
    Window *wp;
    ChatClient *cp;
};


class ChatClient{
    private:
	int tcp_sock;//tcp套接字
	int udp_sock;//udp套接字
	std::string peer_ip;
	
	std::string passwd;
	struct sockaddr_in server;
    public:
	std::string nick_name;
	std::string school;
	unsigned int id;
    public:
	//构造函数
	//初始化信息
	ChatClient()
	{}
	ChatClient(std::string ip_):peer_ip(ip_)
	{
	    id = 0;
	    tcp_sock = -1;
	    udp_sock = -1;
	    server.sin_family = AF_INET;
	    server.sin_port = htons(UDP_PORT);
	    server.sin_addr.s_addr = inet_addr(peer_ip.c_str());
	}
	//初始化客户端
	void InitClient()
	{
	    //创建一个sock套接字
	    udp_sock = SocketApi::Socket(SOCK_DGRAM);
	}
	//建立连接
	bool ConnectServer()
	{
	    //创建套接字
	    //建立链接
	    tcp_sock = SocketApi::Socket(SOCK_STREAM);
	    return SocketApi::Connect(tcp_sock,peer_ip,TCP_PORT);
	}
	//注册
	//首先输入用户的信息
	//然后将信息序列化
	//将序列化之后的信息发送给服务器
	//服务器接收到之后返回一个id
	//检查id是否合理
	bool Register()
	{
	    if( Util::RegisterEnter(nick_name,school,passwd) && ConnectServer()){
		Request rq;
		Name = nick_name;
		rq.method = "REGISTER\n";
		Json::Value value;
		value["name"] = nick_name;
		value["school"] = school;
		value["passwd"] = passwd;
		//将信息序列化
		Util::Seralize(value,rq.text);
	
		rq.content_length = "Content_Length: ";
		rq.content_length += Util::IntToString((rq.text).size());
		rq.content_length += "\n";
		//向服务器发送注册的请求
	   	Util::SendRequest(tcp_sock,rq);
		//接收服务器返回的信息请求
	        recv(tcp_sock,&id,sizeof(id),0);
		bool res = false;
		//检测id是否合理 
	        if(id >= 10000){
		    res = true;
		    std::cout<<"Register Success! Your Login ID Is: "<<id<<std::endl;
		}
		else{
		    std::cout<< "Register Failed! Code is : "<<id<<std::endl;
		}
		close(tcp_sock); 
	        return res;
	     }
	}
	//登录
	//输入id和密码，然后将这个信息序列化发送给服务器
	//如果登陆成功的话，在向服务器发送一条消息，让服务器将这个用户保存在在线用户表中
	bool Login()
	{    //登陆成功后 连接服务器 发送请求
	    if( Util::LoginEnter(id,passwd) && ConnectServer()){
		Request rq;
		rq.method = "LOGIN\n";
		Json::Value value;
		value["id"] = id;
		value["passwd"] = passwd;
		//序列化
		Util::Seralize(value,rq.text);
		
		rq.content_length = "Content_Length: ";
		rq.content_length += Util::IntToString((rq.text).size());
		rq.content_length += "\n";
		//向服务器发送请求
	   	Util::SendRequest(tcp_sock,rq);
		//接收请求
		unsigned int ret = 0;
		bool res = false;
		//接收服务器的返回值
	        recv(tcp_sock,&ret,sizeof(ret),0); 
	        if(ret >= 10000){
		     //如果登陆成功的话，发送一条自动消息，将这个用户添加到在线用户表中
		     res = true;
		     std::string name_ = "None";
		     std::string school_ = "None";
		     std::string text_ = "I am Login! talk with me...";
		     unsigned int type_ = LOGIN_TYPE;
		     unsigned int id_ = ret;
	   	     //首先创建一个信息对象，将要发送的信息添加进去
		     Message m(name_,school_,text_,id_,type_);
		     std::string sendString;
		     //将要发送的消息序列化
		     m.ToSendString(sendString);
		     //使用udp进行数据的发送
		     UdpSend(sendString);
		     //std::cout<<"Login Success! Your Login ID Is: "<<ret<<std::endl;  
		}
		else{
		   // std::cout<< "Login  Failed! Code is : "<<ret<<std::endl;
		}
		close(tcp_sock);
		return res; 
	    }
	}
	//发消息
	void UdpSend(const std::string &message)
	{
	    Util::SendMessage(udp_sock,message,server);
	}
	//接收数据
	void UdpRecv(std::string &message)
	{
	    struct sockaddr_in peer;
	    Util::RecvMessage(udp_sock,message,peer);
	}
	//欢迎条目
	static void  *Welcome(void *arg)
	{
	    pthread_detach(pthread_self());//进行线程分离
	    Window *wp = (Window*)arg;
	    wp->Welcome();//调用欢迎条目的函数
	}
	//输入框
	static void *Input(void *arg)
	{
	    pthread_detach(pthread_self());
	    struct ParamPair *pptr = (struct ParamPair*)arg;
	    Window *wp = pptr->wp;
	    ChatClient *cp = pptr->cp;
	    std::string text;
	    for(;;){
	    	wp->DrawInput();
	   	wp->GetStringFromInput(cp->id,text);//从input窗口中获得字符串
		//传输文件部分
		//判断命令是不是为传输字符串，如果是的话
		std::string tmp1 = "init 0";
		std::string tmp = "Transfer files";
		if(text == tmp)
		{
			Message msg1(cp->nick_name,cp->school,text,cp->id);
			std::string sendString1;
			//将信息序列化
			msg1.ToSendString(sendString1);
			cp->UdpSend(sendString1);//发送字符串
			//获取目录
	   		wp->GetStringFromInput(cp->id,text);//从input窗口中获得字符串
			//将文件的名字发送过去
			Message msg2(cp->nick_name,cp->school,text,cp->id);
			std::string sendString2;
			//将信息序列化
			msg2.ToSendString(sendString2);
			cp->UdpSend(sendString2);//发送字符串
			FILE* fp;
			//打开文件
			fp = fopen(text.c_str(),"r");
			if(fp == NULL)
				exit(0);
			char buffer[1024];
			int len = 0;
			//循环发送文件的内容
			while(len = (fread(buffer,sizeof(char),1024,fp))>0)
			{
				std::string ptr(buffer);
				//发送文件的内容
				Message msg3(cp->nick_name,cp->school,ptr,cp->id);
				std::string sendString3;
				//将信息序列化
				msg3.ToSendString(sendString3);
				cp->UdpSend(sendString3);//发送字符串
				bzero(buffer,1024);
				sleep(1);
			}
			//将text变成文件结束的信号
			text = "EOFEND";
			fclose(fp);
		}
		int out = 0;
		if(text == tmp1)
		{
			out = 1;
		}
		//私聊部分
		//判断如果是私聊命令的话
		//发送数据
		//私聊结束之后发送exit代表私聊结束
		Message msg(cp->nick_name,cp->school,text,cp->id);
		std::string sendString;
		//将信息序列化
		msg.ToSendString(sendString);
		cp->UdpSend(sendString);//发送字符串
		if(out == 1)
			break;
	    }
	}
	//聊天
	void Chat()
	{
	    Window w;
	    pthread_t h,m,l;
	    struct ParamPair pp = {&w,this};
	    //创建一个去执行欢迎框的线程
	    pthread_create(&h,NULL,Welcome,&w); 
	    //创建一个去执行输入的线程
	    int sp = pthread_create(&l,NULL,Input,&pp);
	    if(sp != 0)
		return ;
	    //创建一个输出框
	    w.DrawOutput();
	    //创建一个在线用户表
	    w.DrawOnline();
	    std::string showString;
	    std::vector<std::string> online;
	    //首先接收数据
	    //将数据反序列化
	    //将数据显示到输出显示框
	    //如果这个用户不在用户输出框中的话，将这个用户打印出来
	    for(;;){
	    	std::string recvString;
		Message msg;
		//接收数据
		UdpRecv(recvString);
		int i = 0;
		//将数据反序列化
		msg.ToRecvValue(recvString);
		if(strcmp(msg.Text().c_str(),"quit") == 0)
		{
			pthread_cancel(h);
			pthread_cancel(l);
			break ;
		}
		std::string pi = "Transfer files";
		if(msg.Text() == pi)
	        {
			if(msg.Id() != id)
			{
				//接收文件的名字
				std::string recvString;
				UdpRecv(recvString);
				//反序列化
				msg.ToRecvValue(recvString);
				std::string filename = msg.Text();
				int k=0;               
				for(int j = filename.size()-1;j>=0;j--)                  
				{             
				         if(filename[j]!='/')         
		                	 {           
			        	       k++;     
		                 	 }
	                        	 else 
		                         	 break; 
                        	}
				filename = filename.substr(filename.size()-1-k,k+1);
				std::string pp = "/home/zhh/";
				pp += filename;
				filename = pp;
				//创建出文件，并打开
				FILE* fp;
				fp = fopen(filename.c_str(),"w");
				while(1)
				{
					UdpRecv(recvString);
					//反序列化
					msg.ToRecvValue(recvString);
					std::string tmp("EOFEND");
					if(tmp == msg.Text())
					{
						break;
					}
					tmp = msg.Text();
					fwrite(tmp.c_str(),sizeof(char),tmp.size(),fp);
				}
				//开始循环的读取数据，并且保存在文件中，如果读取到的是EOF的话代表文件的结束
				fclose(fp);
	    			showString ="# ";
	 			showString += "Recv  Sussess";
				//将数据放到输出框
				w.PutMessageToOutput(showString);
			}
			else
			{
				UdpRecv(recvString);
				//反序列化
				msg.ToRecvValue(recvString);
				std::string tmp("EOFEND");
				while(msg.Text() != tmp)
				{
					UdpRecv(recvString);
					msg.ToRecvValue(recvString);
				}
	    			showString ="# ";
	 			showString += "Send  Sussess";
				//将数据放到输出框
				w.PutMessageToOutput(showString);
			}
			continue;
		}
		//如果这个用户在在线用户表中的话，获取他的姓名和学校
		//下面输出消息的时候需要打印出来
		if(msg.Id() == id && msg.Type() == LOGIN_TYPE)
		{
		    nick_name = msg.NickName();
		    school = msg.School();
		}		
		//信息输出的格式的修饰
		showString = msg.NickName();
		showString += "-";
	    	showString += msg.School();
		std::string f = showString;//zhangsan-qinghua
		Util::addUser(online,f);	
		w.PutUserToOnline(online);
	    	showString +="# ";
	 	showString += msg.Text();
		//将数据放到输出框
		w.PutMessageToOutput(showString);
	    }
	}
	//退出
	void LoginOut()
	{
	   // 构建loginout报文
		Request rq;
		rq.method = "LOGOUT\n";
		rq.content_length = "Content_Length: ";
		rq.content_length += "0";
		rq.content_length += "\n";
		Json::Value value;
		value["id"] = id;
		//序列化
		Util::Seralize(value,rq.text);
		
		rq.content_length = "Content_Length: ";
		rq.content_length += Util::IntToString((rq.text).size());
	   //发送数据到服务端
	   	Util::SendRequest(tcp_sock,rq);
	}
	~ChatClient()
	{
	}
};


void Select(std::string ip)
{ 
    ChatClient *cp = new ChatClient(ip);
    cp->InitClient();//xian chushihua kehuduan
    int select = 0;
    while(1)
    {
	Menu(select);
  	switch(select)
	{
        	case 1://Register
           	 cp->Register();
	            break;
        	case 2://Login
	              if(cp->Login())
		      {
        	        	cp->Chat();
	              }
		      else{
        	        std::cout<<"Login Failed!"<<std::endl;
	              }
        	    break;
	        case 3://Exit
        	    exit(0);
 	           break;
 	       default:
        	    exit(1);
   	         break;
	   }
     }
}





















