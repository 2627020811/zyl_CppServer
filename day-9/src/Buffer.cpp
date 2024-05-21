#include"Buffer.h"
#include<string.h>
#include<iostream>

Buffer::Buffer()
{

}

Buffer::~Buffer()
{

}

void Buffer::append(const char* _str,int _size)
{
    for(int i=0;i<_size;i++)
    {
        if(_str[i]=='\0') break;
        buf.push_back(_str[i]);
    }
}

ssize_t Buffer::size()
{
    return buf.size();
}

const char* Buffer::c_str()
{
    return buf.c_str();
}

void Buffer::clear()
{
    buf.clear();
}

void  Buffer::getline()
{
    buf.clear();
    std::getline(std::cin,buf); //这行代码从标准输入流 std::cin 中读取一行文本，并将其存储到名为 buf 的字符串中。
}