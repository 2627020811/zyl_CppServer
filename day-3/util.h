#pragma once
#include<stdio.h>
#include<stdlib.h>

void errif(bool condition,const char *errmsg) //为了方便编码以及代码的可读性，可以封装一个错误处理函数
{
    if(condition)
    {
        perror(errmsg);
        exit(EXIT_FAILURE);
    }
}
