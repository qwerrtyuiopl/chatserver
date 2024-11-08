#include"thread.hpp"
#include<iostream>
void test(int i)
{
    static int num=0;
    pthread_t tid = pthread_self();
    if((num++)%10000==0)
    {
        std::cout<<(unsigned long)tid<<":"<<num<<std::endl;
    }
}

int main()
{
    thread::ThreadPool pool(2);
    pool.start();
    size_t i=0;
    while(++i)
    {
        pool.push_back(std::bind(test,i));
    }
}