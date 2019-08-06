#pragma once
#include <iostream>
#include <vector>
#include <semaphore.h>
#include <string>
class DataPool{
    private:
	std::vector<std::string> pool;
	int cap;
	sem_t data_sem;
	sem_t blank_sem;
	int product_step;
	int consume_step;
    public:
	DataPool(int cap_ = 512):
	    cap(cap_),
	    pool(cap_)
	{
		//刚开始的时候，数据池中还没有数据，所以为0
	    sem_init(&data_sem,0,0);
		//刚开始的时候，数据池中可以存放数据的大小为数据池的容量
	    sem_init(&blank_sem,0,cap);
	    product_step = 0;
	    consume_step = 0;
	}
	void PutMessage(const std::string &msg)
	{
	    //等待数据池中有空位置
	    sem_wait(&blank_sem);
	    //将数据放入数据池中
	    pool[product_step] = msg;
	    //标记向后移动
	    product_step++;	
	    product_step %= cap;
	    //代表存放的数据的个数+1
	    sem_post(&data_sem);
	}
	void GetMessage(std::string &msg)
	{
	    //如果数据池中有数据话
	    sem_wait(&data_sem);
	    //从数据池中拿数据
	    msg = pool[consume_step];
	    //向后移动
	    consume_step++;	
	    consume_step %= cap;
	    //增加一个空位置
	    sem_post(&blank_sem);
	}
	~DataPool()
	{
		//释放信号量
	    sem_destroy(&data_sem);
	    sem_destroy(&blank_sem);
	}
};


















