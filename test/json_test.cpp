#include"json.hpp"
#include<iostream>
int main()
{
    json::json j;
    j["1"]=1;
    j["2"]="[\"11\",\"22\"]";
    j["3"]=2;
    std::cout<<j.formatString()<<std::endl;
}