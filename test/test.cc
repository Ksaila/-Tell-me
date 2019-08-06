#include <iostream>
#include "json/json.h"

int main()
{
    std::string nick_name = "zhangsan";
    std::string school = "SUST";
    std::string passwd = "12343";
	
   Json::Value root;

    root["name"] = nick_name;
    root["school"] = school;
    root["passwd"] = passwd;
    
    Json::FastWriter w;
    std::string s = w.write(root);
    std::cout<<s<<std::endl;	
}

