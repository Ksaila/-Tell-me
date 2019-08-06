#include <iostream>
#include "ChatClient.hpp"


static void Usage(std::string proc)
{
    std::cout<<"Usage: "<<proc<<" peer_ip"<<std::endl;

}


// ./ChatClient ip
int main(int argc, char *argv[])
{
    if(argc != 2){
	Usage(argv[0]);
	exit(1);
    }
    while(1)
    {
    	Select(argv[1]);
    }
    return 0;
}
























































