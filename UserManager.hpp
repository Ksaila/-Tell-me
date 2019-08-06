#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <pthread.h>
//存放的是个人的信息类
class User{
    private:
	std::string nick_name;
	std::string school;
	std::string passwd;
    public:
	User()
	{
	}
	User(const std::string &n,const std::string &s,\
	   	const std::string pwd):
	    nick_name(n),
	    school(s),
	    passwd(pwd)
	{}
	bool IsPasswdTrue(const std::string &passwd_)
	{
	    return passwd == passwd_? true : false; 
	}
	std::string &GetNickName()
	{
	    return nick_name;
	}
	std::string &GetSchool()
	{
	    return school;
	}
	~User()
	{}    
};
//用户管理类
class UserManager{
    private:
	unsigned int assgin_id;
	//存放的是注册的用户的信息
	std::unordered_map<unsigned int,User> users;
	//存放的是在线的用户
	std::unordered_map<unsigned int,struct sockaddr_in> online_users;
	pthread_mutex_t lock;
	void Lock()
	{
 	    pthread_mutex_lock(&lock);
	}
	void UnLock()
	{
	    pthread_mutex_unlock(&lock);
	}
    public:
	UserManager():assgin_id(10000)
	{
	    pthread_mutex_init(&lock,NULL);
	}
	//注册的用户信息插入到用户表中
	unsigned int Insert(const std::string &n,\
		const std::string &s,const std::string &p)
	{
	    Lock();//加锁  防止多个用户同时进行注册操作导致出现错误 保持一致性
	    unsigned int id = assgin_id++;//注册的id模式
	    User u(n,s,p);
	    if(users.find(id) == users.end()){
		users.insert({id,u});//假如用户中没有对应的id 那么就将该用户数据插入到用户中
		UnLock();
		return id;
	    }
	    UnLock();
	    return 1;
	}
	//登录检测
	unsigned int Check(const int &id, const std::string &passwd)
	{
	    Lock();
	    auto user = users.find(id);
	    if(user != users.end()){//是否能在存储的用户信息中找到相应id
	        User &u = user->second;//取出密码
	        if(u.IsPasswdTrue(passwd)){//判断密码时候正确
		    UnLock();
		    return id;
	        }
	    }
	    UnLock();
	    return 2;
	}
	//获取用户的信息
	void GetUserInfo(const unsigned int &id, std::string &name_, std::string &school_)
	{
	    Lock();
	    name_ = users[id].GetNickName();
	    school_ = users[id].GetSchool();
	    UnLock();
	}
	//将新登录在线的用户存放在用户列表中
	void AddOnLineUser(unsigned int id, struct sockaddr_in &peer)
	{
	    Lock();
	    auto it = online_users.find(id);
	    if(it == online_users.end()){
		//假如用户在线列表中没有该用户则将该用户加入到用户列表中
		online_users.insert({id,peer});
	    }
		UnLock();
	}
	//将一个用户从用户列表中删除
	void SubOnlineUser(unsigned int id)
	{
	        Lock();
		online_users.erase(id);
		UnLock();
	}
	//返回在线用户列表
	std::unordered_map<unsigned int,struct sockaddr_in> OnLineUser()
	{
	    Lock();
	    auto online = online_users;
	    UnLock();
	    return online;
	}
	
	~UserManager()
	{
		//销毁锁
 	    pthread_mutex_destroy(&lock);
	}	    
	    
};
























