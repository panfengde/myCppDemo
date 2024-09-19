//
// Created by 潘峰 on 2024/9/19.
//
#include <iostream>
#include <string>
#include <cstring>
#include <new>    // For placement new

class myString;
class ManualBuffer;

class mBase
{
public:
    int type = 0;

    __attribute__((always_inline))
    myString* insertTypeV()
    {
        return reinterpret_cast<myString*>(this);
    }

    // static size_t getSize()
    // {
    //     return sizeof(mBase);
    // }

    virtual mBase* writeBuffer(ManualBuffer*);
};

class myString : public mBase
{
    size_t length = 0; // 字符串长度,注意：已经包括终止符
public:
    char* data;

    myString()
    {
        type = 1;
    };

    myString(char* value)
    {
        type = 1;
        data = value;
    };


    // static size_t getSize()
    // {
    //     return sizeof(myString);
    // }
};


class mObj : public mBase
{
public:
    size_t keyValueIndex;

    mBase* writeBuffer(ManualBuffer*) override
    {
        return this;
    };

    // static size_t getSize() override
    // {
    //     return sizeof(mObj);
    // }
};

class ManualBuffer
{
public:
    ManualBuffer() : buffer(nullptr), capacity(0), currentSize(0)
    {
    }

    ~ManualBuffer()
    {
        if (buffer)
        {
            std::free(buffer);
        }
    }

    char* allocate(size_t neddDataSize)
    {
        if (capacity < currentSize + neddDataSize)
        {
            // 增加缓冲区容量
            capacity = currentSize + neddDataSize;
            buffer = static_cast<char*>(std::realloc(buffer, capacity));
            return buffer
        }
        else
        {
            auto bufferOder = buffer + currentSize;
            bufferOder += neddDataSize;
            return bufferOder;
        }
    };

    // 获取buffer数据指针
    const void* GetData() const
    {
        return buffer;
    }

    // 获取buffer大小
    size_t GetSize() const
    {
        return currentSize;
    }

private:
    char* buffer;
    size_t capacity; //容量
    size_t currentSize; //当前位置
};

int testMystring()
{
    auto bufferLocation = new ManualBuffer();

    std::vector<std::vector<std::string>> allKeys;
    std::vector<std::vector<mBase*>> allValues;
    auto aStringPtr = std::strcpy(bufferLocation->allocate(sizeof("123") + 1), "123");
    bufferLocation = new(bufferLocation->allocate(sizeof(myString))) myString(aStringPtr);


    auto xx = test.size();
    return 0;
}
