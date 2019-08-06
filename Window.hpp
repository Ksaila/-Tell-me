#pragma once

#include <iostream>
#include <string>
#include <string.h>
#include <ncurses.h>
#include <vector>
#include <unistd.h>
#include <cstring>
#include <pthread.h>
class Window{
    public:
	int line;
	WINDOW *header;
	WINDOW *output;
	WINDOW *online;
	WINDOW *input;
	pthread_mutex_t lock;
    public:
	Window()
	{
	    line = 1;
	    initscr();
	    curs_set(0);//隐藏光标
	    pthread_mutex_init(&lock,NULL);
	}
	//刷新串口
	void SafeWrefresh(WINDOW *w)
	{
	    pthread_mutex_lock(&lock);//加锁
	    wrefresh(w);//刷新窗口
	    pthread_mutex_unlock(&lock);//解锁
	}
	//欢迎条目的窗口
	void DrawHeader()
	{
	    int h = LINES*0.2;
	    int w = COLS;
	    int y = 0;
	    int x = 0;
	    header = newwin(h,w,y,x);
	    box(header,0,0);
	    SafeWrefresh(header);//因为显示器是一个临界资源 多个线程控制的多个窗口同时刷新到显示器上可能会导致窗口变花	
	}
	//信息打印的窗口
	void DrawOutput()
	{
	    int h = LINES*0.6;
	    int w = COLS*0.75;
	    int y = LINES*0.2;
	    int x = 0;
	    output = newwin(h,w,y,x);
	    box(output,0,0);
	    SafeWrefresh(output);	
	}
	//在线用户的窗口
	void DrawOnline()
	{
	    
	    int h = LINES*0.6;
	    int w = COLS*0.25;
	    int y = LINES*0.2;
	    int x = COLS*0.75;
	    online = newwin(h,w,y,x);
	    box(online,0,0);
	    SafeWrefresh(online);	
	}
	//输入窗口
	void DrawInput()
	{	    
	    int h = LINES*0.2;
	    int w = COLS;
	    int y = LINES*0.8;
	    int x = 0;
	    input = newwin(h,w,y,x);
	    box(input,0,0);
	    std::string tips = "Please Enter# ";
	    PutStringToWin(input,2,2,tips);
	    SafeWrefresh(input);	
	}
	void GetStringFromInput(unsigned int id,std::string &message)
	{
 	    char buffer[1024];
	    memset(buffer,0,sizeof(buffer));
	    //获取从输入窗口输入的数据
	    //refresh();
	    wgetnstr(input,buffer,sizeof(buffer));
	    message = buffer;
	    //将旧的窗口释放掉
	    delwin(input);
	}	
	void Delwin()
	{
		delwin(input);
	}
	void PutMessageToOutput(const std::string &message)
	{
	    int y,x;
	    //获取窗口的高度和宽度
	    getmaxyx(output,y,x);
	    if(line > y-2){
		delwin(output);
		DrawOutput();
		line = 1;
	    }
	    PutStringToWin(output,line++,2,message);
	}
	//将数据打印到显示框中
	void PutStringToWin(WINDOW *w,int y,int x,const std::string &message)
	{
	    mvwaddstr(w,y,x,message.c_str());
	    SafeWrefresh(w);
	}
	//将在线的用户打印到显示框中
	void PutUserToOnline(std::vector<std::string> &online_user)
	{
		delwin(online);
		DrawOnline();
	    int size = online_user.size();
	    for(auto i = 0; i < size; i++)
	    {
		PutStringToWin(online,i+1,2,online_user[i]);
	    }
	    SafeWrefresh(online);
	}
	//打印欢迎框
	void Welcome()
	{
	    const std::string welcome = "welcome to my chat system!";
	    int num = 1;
	    int x,y;
	    int dir = 0;
 	    for( ; ; )
	    {
		DrawHeader();
 		getmaxyx(header,y,x);
		PutStringToWin(header,y/2,num,welcome);
		if(num > x - welcome.size() - 3){
		    dir = 1;
		}
		if(num <= 1){
		    dir = 0;
		}
		if(dir == 0){
		    num++;
		}else{
		    num--;
		}
		usleep(100000);
		delwin(header);
	    }
	}
	void Delete()
	{
	    delwin(header);
	    delwin(online);
	    delwin(output);
	    delwin(input);	
	    endwin();
	}
	~Window()
	{
	    //释放窗口
	    endwin();
	    //销毁锁
	    pthread_mutex_destroy(&lock);
	}
};























