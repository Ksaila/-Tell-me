#pragma once

#include <iostream>
#include <string>
#include "ProtocolUtil.hpp"
#include "json/json.h"

#define NORMAL_TYPE 0
#define LOGIN_TYPE 1

class Message{
    private:
	std::string nick_name;
	std::string school;
	std::string text;
	unsigned int id;
	unsigned int type;
    public:
	Message()
	{}
	Message(const std::string &n,const std::string &s,const std::string &t,const unsigned int &id, unsigned int type_ = NORMAL_TYPE)
	   : nick_name(n),
	    school(s),
	    text(t),
	    id(id),
	    type(type_)
	{}
	//构建正文
	void ToSendString(std::string &sendString)
	{
	    Json::Value root;
	    root["name"] = nick_name;
	    root["school"] = school;
	    root["text"] = text;
	    root["id"] = id;
	    root["type"] = type;
	    Util::Seralize(root,sendString);
	}
	//提取正文中数据
	void ToRecvValue(std::string &recvString)
	{
	    Json::Value root;
	    Util::UnSeralize(recvString,root);
	    nick_name = root["name"].asString();
	    school = root["school"].asString();
	    text = root["text"].asString();
	    id = root["id"].asInt();
	    type = root["type"].asInt();
	}
	const std::string &NickName()
	{
	    return nick_name;
	}
	const std::string &School()
	{
	    return school;
	}
	const std::string &Text()
	{
	    return text;
	}
	const unsigned int &Id()
	{
	    return id;
	}
	const unsigned int &Type()
	{
	    return type;
	}
	~Message()
	{
	}
};













