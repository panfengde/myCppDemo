//
// Created by 潘峰 on 2024/9/19.
//
#include <iostream>
#include <string>
#include <cstring>
#include <new>    // For placement new
#include <vector>
#include <random>
#include "timeCountMacro.h"

std::string generateRandomString(size_t length)
{
    const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string randomString;

    // 使用随机数生成器
    std::random_device rd; // 随机数种子
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, charset.size() - 1);

    // 生成随机字符串
    for (size_t i = 0; i < length; ++i)
    {
        randomString += charset[distribution(generator)];
    }

    return randomString;
}

class mBase;
class myString;
class myObj;
class ManualBuffer;

std::vector<std::vector<std::string>> allKeys;
std::vector<std::vector<size_t>> allValues;


class ManualBuffer
{
public:
    ManualBuffer() : offset(0)
    {
        // 初始化缓冲区，预分配 1 MB
        //buffer.resize(1024 * 1024);
    }

    char* allocate(size_t needDataSize)
    {
        if (offset + needDataSize > buffer.size())
        {
            // 增加缓冲区容量
            size_t newCapacity = (buffer.size() + needDataSize); // 扩容策略
            //std::cout << "add---" << std::endl;
            buffer.resize(newCapacity);
        }
        char* currentPtr = buffer.data() + offset;
        offset += needDataSize;
        return currentPtr;
    }

    // 获取buffer数据指针
    char* GetData()
    {
        return buffer.data();
    }

    // 获取buffer大小
    size_t GetSize() const
    {
        return offset;
    }

    // 克隆方法
    [[nodiscard]] ManualBuffer* clone() const
    {
        auto* newBuffer = new ManualBuffer();
        newBuffer->offset = offset;
        newBuffer->buffer = buffer; // 直接复制 vector，避免手动复制
        return newBuffer;
    }

    std::vector<char> buffer; // 使用 std::vector 代替裸指针
    size_t offset; // 当前偏移
};

class mBase
{
public:
    //virtual ~mBase() = default;

    int type = 0;

    __attribute__((always_inline))
    myString* toMyString()
    {
        return reinterpret_cast<myString*>(this);
    }

    __attribute__((always_inline))
    myObj* toMyObj()
    {
        return reinterpret_cast<myObj*>(this);
    }
};

class myString : public mBase
{
    size_t strOffst = 1; // 字符串长度,注意：已经包括终止符

public:
    myString()
    {
        type = 1;
    };

    myString(size_t strLength)
    {
        type = 1;
        strOffst = strLength;
    };

    std::string getValue() const
    {
        // 计算新的指针位置
        return std::string(static_cast<const char*>(reinterpret_cast<const void*>(this)) - strOffst);
    }
};

class myObj : public mBase
{
public:
    size_t keyValueIndex = 0;

    ~myObj() = default;

    myObj()
    {
        type = 2;
        allKeys.emplace_back();
        allValues.emplace_back();
        keyValueIndex = allKeys.size() - 1;
    }

    void insertKeyValue(std::string key, size_t valueOffset) const
    {
        allKeys[keyValueIndex].push_back(key);
        allValues[keyValueIndex].push_back(valueOffset);
    }

    void sayHello() const
    {
        std::cout << keyValueIndex << std::endl;
    }

    void getValueByName(std::string& key, mBase* value) const
    {
    }
};


class myFrame : public myObj
{
public:
    size_t bufferStart = 0;
    size_t bufferEnd = 0;
    myFrame() = default;

    explicit myFrame(ManualBuffer* bufferStore)
    {
        markStart(bufferStore);
        type = 3;
    }

    inline void markStart(ManualBuffer* aManualBuffer)
    {
        bufferStart = aManualBuffer->offset;
    }

    inline void markEnd(ManualBuffer* aManualBuffer)
    {
        bufferEnd = aManualBuffer->offset;
    }
};

size_t createAFrame(ManualBuffer* bufferStore)
{
    auto aFrame = new(bufferStore->allocate(sizeof(myFrame))) myFrame(bufferStore);
    return reinterpret_cast<const char*>(aFrame) - bufferStore->GetData();
}

size_t createAStr(ManualBuffer* bufferStore, std::string& strValue)
{
    auto cStr = strValue.c_str();
    // 计算字符串长度
    size_t len = strlen(cStr);

    // 在 ManualBuffer 中分配空间
    char* strBuffer = bufferStore->allocate(len + 1);
    std::strcpy(strBuffer, cStr);

    // 在 ManualBuffer 中分配空间给 myString 对象
    myString* aStr = new(bufferStore->allocate(sizeof(myString))) myString(len + 1);

    // 返回 aStr 相对于 bufferStore->buffer 的偏移量
    return reinterpret_cast<const char*>(aStr) - bufferStore->GetData();
}

size_t createAStr(ManualBuffer* bufferStore, const char* cStr)
{
    // 计算字符串长度
    size_t len = strlen(cStr);
    std::strcpy(bufferStore->allocate(len + 1), cStr);
    auto aStr = new(bufferStore->allocate(sizeof(myString))) myString(len + 1);
    return reinterpret_cast<const char*>(aStr) - bufferStore->GetData();
}

size_t createAObj(ManualBuffer* bufferStore)
{
    auto aObj = new(bufferStore->allocate(sizeof(myObj))) myObj();
    return reinterpret_cast<const char*>(aObj) - bufferStore->GetData();
}

int dev()
{
    auto bufferStore = new ManualBuffer();
    std::vector<size_t> allOffset;

    for (int i = 0; i < 2000; i++)
    {
        //std::cout <<  std::string("abcdefg" + std::to_string(i)) << std::endl;
        //auto aString = std::string("abcdefg" + std::to_string(i)).c_str();
        auto frameOffset = createAFrame(bufferStore);
        allOffset.push_back(frameOffset);

        auto aString = generateRandomString(10).c_str();
        allOffset.push_back(createAStr(bufferStore, aString));

        auto createAObjResult = createAObj(bufferStore);
        allOffset.push_back(createAObjResult);

        auto createAStringResult_1 = createAStr(bufferStore, "123");
        auto createAStringResult_2 = createAStr(bufferStore, "456");

        auto thisPtr = bufferStore->GetData() + createAObjResult;
        auto thisObj = reinterpret_cast<const myObj*>(thisPtr);
        //std::cout << thisObj->keyValueIndex << std::endl;
        thisObj->insertKeyValue("a", createAStringResult_1);
        thisObj->insertKeyValue("b", createAStringResult_2);

        auto aFrame = reinterpret_cast<myFrame*>(bufferStore->GetData() + frameOffset);
        aFrame->markEnd(bufferStore);
    }

    std::cout << "ok---" << std::endl;

    auto newBufferStore1 = bufferStore->clone();
    auto newBufferStore = newBufferStore1->clone();

    for (int i = 0; i < 2000; i += 100)
    {
        std::cout << "offset:" << allOffset[i] << std::endl;
        auto thisPtr = newBufferStore->GetData() + allOffset[i];
        if (reinterpret_cast<const mBase*>(thisPtr)->type == 1)
        {
            std::cout << "string value:" << reinterpret_cast<const myString*>(thisPtr)->getValue() << std::endl;
        }
        else if (reinterpret_cast<const mBase*>(thisPtr)->type == 2)
        {
            std::cout << "obj value:" << reinterpret_cast<const myObj*>(thisPtr)->keyValueIndex << std::endl;
        }

        auto nextPtr = newBufferStore->GetData() + allOffset[i + 1];
        if (reinterpret_cast<const mBase*>(nextPtr)->type == 1)
        {
            std::cout << "string value:" << reinterpret_cast<const myString*>(nextPtr)->getValue() << std::endl;
        }
        else if (reinterpret_cast<const mBase*>(nextPtr)->type == 2)
        {
            std::cout << "obj value:" << reinterpret_cast<const myObj*>(nextPtr)->keyValueIndex << std::endl;
        }
    }

    return 0;
}

__attribute__((optimize("O0")))
int testBuffer()
{
    auto bufferStore = new ManualBuffer();


    std::vector<size_t> allOffset;

    for (int i = 0; i < 10000; i++)
    {
        //std::cout <<  std::string("abcdefg" + std::to_string(i)) << std::endl;
        //auto aString = std::string("abcdefg" + std::to_string(i)).c_str();
        auto aString = generateRandomString(10).c_str();
        allOffset.push_back(createAStr(bufferStore, aString));

        auto createAObjResult = createAObj(bufferStore);
        allOffset.push_back(createAObjResult);
    }


    START_TIMER("buffer");
    for (int k = 0; k < 100; k++)
    {
        auto newBufferStore1 = bufferStore->clone();
        delete newBufferStore1;
    }
    STOP_TIMER("buffer");
    return 0;
}
__attribute__((optimize("O0")))
void xxx()
{

}

__attribute__((optimize("O0")))
void testNew()
{
    START_TIMER("new");
    for (int k = 0; k < 100; k++)
    {
        for (int i = 0; i < 10000; i++)
        {
            auto a = new std::string("1231231");
            auto b = new myString(11 + 1);
            auto c = new myObj();
            delete a;
            delete b;
            delete c;
        }
    }
    STOP_TIMER("new");
}

void goTest()
{
    dev();
}

void performanceTest()
{
    testNew();
    testBuffer();
    showTimeMNap();
}
