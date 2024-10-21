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
#include <fstream>

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


std::vector<size_t> objOffset;
std::vector<size_t> stringOffset;
std::vector<size_t> frameOffset;

class mBase;
class myString;
class myObj;
class myFrame;
class ManualBuffer;

class ManualBuffer
{
public:
    ManualBuffer() : currentOffset(0)
    {
        // buffer.resize(1024 * 1024);// 初始化缓冲区，预分配 1 MB
        buffer.resize(8); // 初始化缓冲区，预分配 1 B
    }

    ~ManualBuffer()
    {
        std::vector<char>().swap(buffer);
    };

    char* allocate(size_t needDataSize)
    {
        if (currentOffset + needDataSize > buffer.size())
        {
            // 增加缓冲区容量
            size_t newCapacity = needDataSize + buffer.size(); // 扩容策略
            buffer.resize(newCapacity);
        }
        char* currentPtr = buffer.data() + currentOffset;
        currentOffset += needDataSize;
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
        return currentOffset;
    }

    void shrink_to_fit()
    {
        if (currentOffset < buffer.size())
        {
            buffer.resize(currentOffset);
            buffer.shrink_to_fit();
        }
    }

    // 克隆方法
    [[nodiscard]] ManualBuffer* clone() const
    {
        auto* newBuffer = new ManualBuffer();
        newBuffer->currentOffset = currentOffset;
        newBuffer->buffer = buffer; // 直接复制 vector，避免手动复制
        newBuffer->bufferOffsetComparedOriginBuffer = bufferOffsetComparedOriginBuffer;
        return newBuffer;
    }

    std::vector<char> buffer; // 使用 std::vector 代替裸指针
    size_t currentOffset; // 当前偏移
    size_t bufferOffsetComparedOriginBuffer = 0; // 当前buffer相对于原始buffer的偏移
};

template <class T>
T* getPtr(size_t offset, ManualBuffer* aManualBuffer)
{
    return reinterpret_cast<T*>(aManualBuffer->GetData() + aManualBuffer->bufferOffsetComparedOriginBuffer + offset);
}

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

class myObj : public mBase
{
public:
    // 使用 std::shared_ptr 管理内存

    std::vector<std::string>* Keys = nullptr;
    std::vector<size_t>* ValuesOffset = nullptr;
    std::vector<mBase*>* valuesPtr = nullptr;

    size_t writeBufferOffset = 0; //在原始buffer的偏移量

    ~myObj()
    {
        if (Keys)
        {
            delete Keys;
            delete ValuesOffset;
        }
    };

    myObj()
    {
        type = 2;
        Keys = new std::vector<std::string>();
        ValuesOffset = new std::vector<size_t>();
    }

    void insertKeyValue(std::string key, size_t aValueOffset)
    {
        Keys->push_back(key);
        ValuesOffset->push_back(aValueOffset);
    }

    void writeToBuffer(ManualBuffer* bufferStore)
    {
        writeBufferOffset = bufferStore->currentOffset;

        size_t totalSize = sizeof(size_t) + ValuesOffset->size() * sizeof(size_t);
        for (const auto& aKey : (*Keys))
        {
            totalSize += sizeof(size_t) + aKey.size(); // 保存每个字符串的大小和内容
        }

        //allocate很可能改变原来buffer，导致指针失效。所以要在改变之前，计算偏移量；
        auto thisOffset = reinterpret_cast<const char*>(this) - bufferStore->GetData();
        auto ptr = bufferStore->allocate(totalSize);
        //计算得到新的this
        auto newThis = getPtr<myObj>(thisOffset, bufferStore);
        auto newValuesOffset = newThis->ValuesOffset;
        auto newKeys = newThis->Keys;

        auto offsetSize = newValuesOffset->size();
        memcpy(ptr, &offsetSize, sizeof(size_t));
        ptr += sizeof(size_t);
        memcpy(ptr, newValuesOffset->data(), newValuesOffset->size() * sizeof(size_t));
        ptr += newValuesOffset->size() * sizeof(size_t);

        for (const auto& key : *newKeys)
        {
            size_t keySize = key.size();
            memcpy(ptr, &keySize, sizeof(size_t));
            ptr += sizeof(size_t);
            memcpy(ptr, key.data(), keySize);
            ptr += keySize;
        }

        delete Keys;
        delete ValuesOffset;
        Keys = nullptr;
        ValuesOffset = nullptr;
    }

    void readFromBuffer(ManualBuffer* bufferStore)
    {
        const char* ptr = bufferStore->buffer.data() + (writeBufferOffset - bufferStore->
            bufferOffsetComparedOriginBuffer);

        // 读取ValuesOffset
        size_t offsetSize;
        memcpy(&offsetSize, ptr, sizeof(size_t));

        ptr += sizeof(size_t);

        ValuesOffset = new std::vector<size_t>();
        ValuesOffset->resize(offsetSize); //使用memcpy复制数据到vector中时，vector内部的空间大小应该准备好了

        memcpy(ValuesOffset->data(), ptr, offsetSize * sizeof(size_t));
        ptr += offsetSize * sizeof(size_t);

        // 读取keys
        Keys = new std::vector<std::string>();
        Keys->reserve(offsetSize);

        for (size_t i = 0; i < offsetSize; ++i)
        {
            size_t keySize;
            memcpy(&keySize, ptr, sizeof(size_t)); // 读取字符串大小
            ptr += sizeof(size_t);
            std::string key(ptr, keySize); // 读取字符串内容
            //std::cout << "read:" << key << std::endl;
            Keys->push_back(std::move(key));
            ptr += keySize;
        }
    }

    void sayHello() const
    {
        std::cout << Keys->size() << std::endl;
    }

    void getValueByName(std::string& key, mBase* value) const
    {
    }
};

class myString : public myObj
{
    size_t strLen = 0; // 字符串长度,注意：已经包括终止符
    const char* cStr = nullptr;

public:
    myString()
    {
        type = 1;
    };


    myString(const char* cStr, ManualBuffer* bufferStore)
    {
        type = 1;
        // 计算字符串长度
        strLen = strlen(cStr);
        std::strcpy(reinterpret_cast<char*>(this) + sizeof(myString), cStr);
    };

    std::string getValue() const
    {
        // 计算新的指针位置
        return std::string(static_cast<const char*>(reinterpret_cast<const void*>(this)) + sizeof(myString));
    }
};

class myFrame : public myObj
{
public:
    size_t frameBufferStartOffset = 0;
    size_t frameBufferEndOffset = 0;

    //myFrame() = default;

    explicit myFrame(ManualBuffer* bufferStore)
    {
        markStart(bufferStore);
        type = 3;
    }

    inline void markStart(ManualBuffer* bufferStore)
    {
        frameBufferStartOffset = bufferStore->currentOffset;
    }

    inline void markEnd(ManualBuffer* bufferStore)
    {
        writeToBuffer(bufferStore);
        frameBufferEndOffset = bufferStore->currentOffset;
    }

    myFrame* clone(ManualBuffer* bufferStore)
    {
        // if (startOffset >= endOffset || endOffset > buffer.size())
        // {
        //     throw std::out_of_range("Invalid offset range");
        // }

        auto* newBuffer = new ManualBuffer();
        newBuffer->bufferOffsetComparedOriginBuffer = frameBufferStartOffset;
        newBuffer->buffer = std::vector<char>(bufferStore->buffer.begin() + frameBufferStartOffset,
                                              bufferStore->buffer.begin() + frameBufferEndOffset);
        newBuffer->currentOffset = newBuffer->buffer.size() - 1;
        return reinterpret_cast<myFrame*>(newBuffer);
    };
};

size_t createAFrame(ManualBuffer* bufferStore)
{
    auto aFrame = new(bufferStore->allocate(sizeof(myFrame))) myFrame(bufferStore);
    auto offset = reinterpret_cast<const char*>(aFrame) - bufferStore->GetData();
    frameOffset.push_back(offset);
    return offset;
}

size_t createAStr(ManualBuffer* bufferStore, const char* cStr)
{
    // 计算字符串长度
    auto aStr = new(bufferStore->allocate(sizeof(myString) + strlen(cStr) + 1)) myString(cStr, bufferStore);
    // 返回 aStr 相对于 bufferStore->buffer 的偏移量
    auto offset = reinterpret_cast<const char*>(aStr) - bufferStore->GetData();
    stringOffset.push_back(offset);

    return offset;
}

size_t createAStr(ManualBuffer* bufferStore, std::string strValue)
{
    auto cStr = strValue.c_str();
    // 在 ManualBuffer 中分配空间给 myString 对象
    return createAStr(bufferStore, cStr);
}

size_t createAObj(ManualBuffer* bufferStore)
{
    auto aObj = new(bufferStore->allocate(sizeof(myObj))) myObj();
    auto offset = reinterpret_cast<const char*>(aObj) - bufferStore->GetData();
    objOffset.push_back(offset);

    return offset;
}

int objLevel = 0;

void printObj(size_t objOffset, ManualBuffer* aManualBuffer)
{
    objLevel++;
    std::cout << objLevel << "---------obj info--------start----" << std::endl;
    auto aObj = getPtr<myObj>(objOffset, aManualBuffer);
    aObj->readFromBuffer(aManualBuffer);

    for (int i = 0; i < aObj->Keys->size(); i++)
    {
        auto& Keys = *(aObj->Keys);
        auto& ValuesOffset = *(aObj->ValuesOffset);

        auto aBase = getPtr<mBase>(ValuesOffset[i], aManualBuffer);
        if (aBase->type == 2)
        {
            printObj(ValuesOffset[i], aManualBuffer);
        }
        else
        {
            std::cout << "key:" << Keys[i] << "|" <<
                "value:" << getPtr<myString>(ValuesOffset[i], aManualBuffer)->getValue() << std::endl;
        }
    }
    std::cout << objLevel << "---------obj info--------end----" << std::endl;
    objLevel--;
}

int frameLevel = 0;

void prinFrame(size_t objOffset, ManualBuffer* aManualBuffer)
{
    frameLevel++;
    std::cout << frameLevel << "*********frame info*********start***" << std::endl;
    auto aFrame = getPtr<myFrame>(objOffset, aManualBuffer);

    aFrame->readFromBuffer(aManualBuffer);

    for (int i = 0; i < aFrame->Keys->size(); i++)
    {
        auto& Keys = *(aFrame->Keys);
        auto& ValuesOffset = *(aFrame->ValuesOffset);

        auto aBase = getPtr<mBase>(ValuesOffset[i], aManualBuffer);
        if (aBase->type == 3)
        {
            prinFrame(ValuesOffset[i], aManualBuffer);
        }
        else if (aBase->type == 2)
        {
            printObj(ValuesOffset[i], aManualBuffer);
        }
        else
        {
            std::cout << "key:" << Keys[i] << "|" <<
                "value:" << getPtr<myString>(ValuesOffset[i], aManualBuffer)->getValue() << std::endl;
        }
    }
    std::cout << frameLevel << "*********frame info*********end***" << std::endl;
    frameLevel--;
}

int stringIndex = 0;
int insertFrameIndex = 0;

void createData(ManualBuffer* bufferStore, size_t topFrameOffset)
{
    createAStr(bufferStore, std::string("aString_") + std::to_string(++stringIndex));
    createAStr(bufferStore, std::string("aString_") + std::to_string(++stringIndex));
    auto aObjOffestt = createAObj(bufferStore);

    auto threeStringOffest = createAStr(bufferStore, std::string("aString_") + std::to_string(++stringIndex));
    auto aObj = getPtr<myFrame>(aObjOffestt, bufferStore);
    aObj->insertKeyValue("key_1", threeStringOffest);
    auto fourStringOffest = createAStr(bufferStore, std::string("aString_") + std::to_string(++stringIndex));

    aObj = getPtr<myFrame>(aObjOffestt, bufferStore);
    aObj->insertKeyValue("key_2", fourStringOffest);
    aObj->writeToBuffer(bufferStore);

    auto aframeOffset = createAFrame(bufferStore);
    auto fiveStringOffest = createAStr(bufferStore, std::string("aString_") + std::to_string(++stringIndex));
    auto sixStringOffest = createAStr(bufferStore, std::string("aString_") + std::to_string(++stringIndex));
    auto aFrame = getPtr<myFrame>(aframeOffset, bufferStore);
    auto topFrame = getPtr<myFrame>(topFrameOffset, bufferStore);
    topFrame->insertKeyValue(std::string("___^^^^^^^_____frame_") + std::to_string(++insertFrameIndex), aframeOffset);

    aFrame->insertKeyValue(std::string("frame_var_1"), fiveStringOffest);
    aFrame->insertKeyValue(std::string("frame_var_2"), sixStringOffest);

    auto bObjOffestt = createAObj(bufferStore);
    auto sevenStringOffest = createAStr(bufferStore, std::string("aString_") + std::to_string(++stringIndex));
    auto bObj = getPtr<myFrame>(bObjOffestt, bufferStore);
    bObj->insertKeyValue("key_1", sevenStringOffest);
    bObj->writeToBuffer(bufferStore);

    aFrame = getPtr<myFrame>(aframeOffset, bufferStore);
    aFrame->insertKeyValue(std::string("frame_var_3"), bObjOffestt);
    aFrame->markEnd(bufferStore);
}

void writeVectorToFile(ManualBuffer* aManualBuffer, const std::string& filename)
{
    const std::vector<char>& vec = aManualBuffer->buffer;
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    if (file.is_open())
    {
        // 写入整个 vector 数据到文件中
        file.write(reinterpret_cast<const char*>(vec.data()), vec.size());
        file.close();
    }
    else
    {
        std::cerr << "无法打开文件进行写入: " << filename << std::endl;
    }
}

ManualBuffer* readVectorFromFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::in | std::ios::binary);

    auto aManualBuffer = new ManualBuffer();
    auto& vec = aManualBuffer->buffer;
    if (file.is_open())
    {
        // 获取文件大小
        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        // 调整 vector 大小并读取文件内容
        vec.resize(size);
        file.read(vec.data(), size);
        file.close();
    }
    else
    {
        std::cerr << "无法打开文件进行读取: " << filename << std::endl;
    }

    return aManualBuffer;
}

void readBufferFromFile()
{
    auto bufferStore = readVectorFromFile("/Users/panfeng/coder/myProject/CppDemo/src/CustomAllocator/dist/testBuffer");
    prinFrame(0, bufferStore);
}

void writeBufferToFile()
{
    auto bufferStore = new ManualBuffer();
    int stringIndex = 0;
    auto topFrameOffset = createAFrame(bufferStore);

    for (int i = 0; i < 10; i++)
    {
        createData(bufferStore, 0);
    }

    getPtr<myFrame>(topFrameOffset, bufferStore)->markEnd(bufferStore);
    bufferStore->shrink_to_fit();

    // auto newBufferStore_1 = bufferStore->clone();
    // auto newBufferStore_2 = newBufferStore_1->clone();
    // delete bufferStore;
    // delete newBufferStore_1;

    writeVectorToFile(bufferStore, "/Users/panfeng/coder/myProject/CppDemo/src/CustomAllocator/dist/testBuffer");
}

int dev()
{
    auto bufferStore = new ManualBuffer();
    int stringIndex = 0;
    auto topFrameOffset = createAFrame(bufferStore);

    for (int i = 0; i < 200; i++)
    {
        createData(bufferStore, 0);
    }

    getPtr<myFrame>(topFrameOffset, bufferStore)->markEnd(bufferStore);
    bufferStore->shrink_to_fit();

    auto newBufferStore_1 = bufferStore->clone();
    auto newBufferStore_2 = newBufferStore_1->clone();
    delete bufferStore;
    delete newBufferStore_1;

    writeVectorToFile(newBufferStore_2, "/Users/panfeng/coder/myProject/CppDemo/src/CustomAllocator/dist/testBuffer");

    for (auto& aStringOffset : stringOffset)
    {
        auto aStringPtr = getPtr<myString>(aStringOffset, newBufferStore_2);
        std::cout << "string value:" << aStringPtr->getValue() << std::endl;
    }

    for (auto& aObjOffset : objOffset)
    {
        printObj(aObjOffset, newBufferStore_2);
    }

    for (auto& oneFrameOffset : frameOffset)
    {
        prinFrame(oneFrameOffset, newBufferStore_2);
    }

    /*
    auto newBufferStore1 = bufferStore->clone();
    auto newBufferStore = newBufferStore1->clone();
    delete bufferStore;
    delete newBufferStore1;

    for (int i = 0; i < 1000; i += 100)
    {
        // std::cout << "offset:" << allOffset[i] << std::endl;
        auto thisPtr = newBufferStore->GetData() + allOffset[i];
        if (reinterpret_cast<const mBase*>(thisPtr)->type == 1)
        {
            std::cout << "string value:" << reinterpret_cast<const myString*>(thisPtr)->getValue() << std::endl;
        }
        else if (reinterpret_cast<const mBase*>(thisPtr)->type == 2)
        {
            auto aObj = reinterpret_cast<myObj*>(thisPtr);
            aObj->readFromBuffer(newBufferStore);
            std::cout << "obj value:" << aObj->Keys->size() << std::endl;
        }

        auto nextPtr = newBufferStore->GetData() + allOffset[i + 1];
        if (reinterpret_cast<const mBase*>(nextPtr)->type == 1)
        {
            std::cout << "string value:" << reinterpret_cast<const myString*>(nextPtr)->getValue() << std::endl;
        }
        else if (reinterpret_cast<const mBase*>(nextPtr)->type == 2)
        {
            auto aObj = reinterpret_cast<myObj*>(nextPtr);
            aObj->readFromBuffer(newBufferStore);
            std::cout << "obj value:" << aObj->Keys->size() << std::endl;
        }
    }
    */

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
    for (int k = 0; k < 10000; k++)
    {
        auto newBufferStore1 = bufferStore->clone();
        delete newBufferStore1;
    }
    STOP_TIMER("buffer");
    return 0;
}

__attribute__((optimize("O0")))
void testNew()
{
    START_TIMER("new");
    for (int k = 0; k < 10000; k++)
    {
        for (int i = 0; i < 10000; i++)
        {
            auto a = new std::string("1231231");
            // auto b = new myString("12");
            auto c = new myObj();
            delete a;
            //delete b;
            delete c;
        }
    }
    STOP_TIMER("new");
}

void goTest()
{
    START_TIMER("new");
    //dev();
    //writeBufferToFile();
    readBufferFromFile();
    STOP_TIMER("new");
    showTimeMNap();
}

void performanceTest()
{
    testNew();
    testBuffer();
    showTimeMNap();
}
