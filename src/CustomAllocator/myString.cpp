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

std::string generateRandomString(size_t length) {
    const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string randomString;

    // 使用随机数生成器
    std::random_device rd; // 随机数种子
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, charset.size() - 1);

    // 生成随机字符串
    for (size_t i = 0; i < length; ++i) {
        randomString += charset[distribution(generator)];
    }

    return randomString;
}

class mBase;
class myString;
class myObj;
class myFrame;
class ManualBuffer;

template<class T>
T *getPtr(size_t offset, ManualBuffer *aManualBuffer) {
    return reinterpret_cast<T *>(aManualBuffer->GetData() + aManualBuffer->bufferOffset + offset);
}

class ManualBuffer {
public:
    ManualBuffer() : offset(0) {
        // 初始化缓冲区，预分配 1 MB
        // buffer.resize(1024 * 1024);
    }

    ~ManualBuffer() = default;

    char *allocate(size_t needDataSize) {
        if (offset + needDataSize > buffer.size()) {
            // 增加缓冲区容量
            size_t newCapacity = (buffer.size() + needDataSize); // 扩容策略
            //std::cout << "add---" << std::endl;
            buffer.resize(newCapacity);
        }
        char *currentPtr = buffer.data() + offset;
        offset += needDataSize;
        return currentPtr;
    }

    // 获取buffer数据指针
    char *GetData() {
        return buffer.data();
    }

    // 获取buffer大小
    size_t GetSize() const {
        return offset;
    }

    // 克隆方法
    [[nodiscard]] ManualBuffer *clone() const {
        auto *newBuffer = new ManualBuffer();
        newBuffer->offset = offset;
        newBuffer->buffer = buffer; // 直接复制 vector，避免手动复制
        newBuffer->bufferOffset = bufferOffset;
        return newBuffer;
    }

    std::vector<char> buffer; // 使用 std::vector 代替裸指针
    size_t offset; // 当前偏移
    size_t bufferOffset = 0; // 当前buffer相对于原始buffer的偏移
};

class mBase {
public:
    //virtual ~mBase() = default;

    int type = 0;

    __attribute__((always_inline))
    myString *toMyString() {
        return reinterpret_cast<myString *>(this);
    }

    __attribute__((always_inline))
    myObj *toMyObj() {
        return reinterpret_cast<myObj *>(this);
    }
};

class myString : public mBase {
    size_t strOffst = 1; // 字符串长度,注意：已经包括终止符

public:
    myString() {
        type = 1;
    };

    myString(size_t strLength) {
        type = 1;
        strOffst = strLength;
    };

    std::string getValue() const {
        // 计算新的指针位置
        return std::string(static_cast<const char *>(reinterpret_cast<const void *>(this)) - strOffst);
    }
};

class myObj : public mBase {
public:
    // 使用 std::shared_ptr 管理内存

    std::vector<std::string> *Keys = nullptr;
    std::vector<size_t> *ValuesOffset = nullptr;

    std::vector<mBase *> *valuesPtr = nullptr;
    size_t writeBufferOffset = 0; //在原始buffer的偏移量

    ~myObj() = default;

    myObj() {
        auto keysValue = new std::vector<std::string>();
        auto ValuesOffsetValue = new std::vector<size_t>();
        type = 2;
        Keys = keysValue;
        ValuesOffset = ValuesOffsetValue;
    }

    void insertKeyValue(std::string key, size_t aValueOffset) {
        Keys->push_back(key);
        ValuesOffset->push_back(aValueOffset);
    }

    void writeToBuffer(ManualBuffer *bufferStore) {
        writeBufferOffset = bufferStore->offset;

        size_t totalSize = sizeof(size_t) + ValuesOffset->size() * sizeof(size_t);
        for (const auto &aKey: (*Keys)) {
            totalSize += sizeof(size_t) + aKey.size(); // 保存每个字符串的大小和内容
        }

        //allocate很可能改变原来buffer，导致指针失效。所以要在改变之前，计算偏移量；
        auto thisOffset = reinterpret_cast<const char *>(this) - bufferStore->GetData();
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

        for (const auto &key: *newKeys) {
            size_t keySize = key.size();
            memcpy(ptr, &keySize, sizeof(size_t));
            ptr += sizeof(size_t);
            memcpy(ptr, key.data(), keySize);
            ptr += keySize;
        }
    }

    void readFromBuffer(ManualBuffer *bufferStore) {
        const char *ptr = bufferStore->buffer.data() + (writeBufferOffset - bufferStore->bufferOffset);

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

        for (size_t i = 0; i < offsetSize; ++i) {
            size_t keySize;
            memcpy(&keySize, ptr, sizeof(size_t)); // 读取字符串大小
            ptr += sizeof(size_t);
            std::string key(ptr, keySize); // 读取字符串内容
            //std::cout << "read:" << key << std::endl;
            Keys->push_back(std::move(key));
            ptr += keySize;
        }
    }

    void sayHello() const {
        std::cout << Keys->size() << std::endl;
    }

    void getValueByName(std::string &key, mBase *value) const {
    }
};

class myFrame : public myObj {
public:
    size_t startBuffer = 0;
    size_t endBuffer = 0;

    //myFrame() = default;

    explicit myFrame(ManualBuffer *bufferStore) {
        markStart(bufferStore);
        type = 3;
    }

    inline void markStart(ManualBuffer *bufferStore) {
        startBuffer = bufferStore->offset;
    }

    inline void markEnd(ManualBuffer *bufferStore) {
        writeToBuffer(bufferStore);
        endBuffer = bufferStore->offset;
    }

    myFrame *clone(ManualBuffer *bufferStore) {
        // if (startOffset >= endOffset || endOffset > buffer.size())
        // {
        //     throw std::out_of_range("Invalid offset range");
        // }

        auto *newBuffer = new ManualBuffer();
        newBuffer->bufferOffset = startBuffer;
        newBuffer->buffer = std::vector<char>(bufferStore->buffer.begin() + startBuffer,
                                              bufferStore->buffer.begin() + endBuffer);
        newBuffer->offset = newBuffer->buffer.size() - 1;
        return reinterpret_cast<myFrame *>(newBuffer);
    };
};

size_t createAFrame(ManualBuffer *bufferStore) {
    auto aFrame = new(bufferStore->allocate(sizeof(myFrame))) myFrame(bufferStore);
    return reinterpret_cast<const char *>(aFrame) - bufferStore->GetData();
}

size_t createAStr(ManualBuffer *bufferStore, std::string &strValue) {
    auto cStr = strValue.c_str();
    // 计算字符串长度
    size_t len = strlen(cStr);

    // 在 ManualBuffer 中分配空间
    char *strBuffer = bufferStore->allocate(len + 1);
    std::strcpy(strBuffer, cStr);

    // 在 ManualBuffer 中分配空间给 myString 对象
    myString *aStr = new(bufferStore->allocate(sizeof(myString))) myString(len + 1);

    // 返回 aStr 相对于 bufferStore->buffer 的偏移量
    return reinterpret_cast<const char *>(aStr) - bufferStore->GetData();
}

size_t createAStr(ManualBuffer *bufferStore, const char *cStr) {
    // 计算字符串长度
    size_t len = strlen(cStr);
    std::strcpy(bufferStore->allocate(len + 1), cStr);
    auto aStr = new(bufferStore->allocate(sizeof(myString))) myString(len + 1);
    return reinterpret_cast<const char *>(aStr) - bufferStore->GetData();
}

size_t createAObj(ManualBuffer *bufferStore) {
    auto aObj = new(bufferStore->allocate(sizeof(myObj))) myObj();
    return reinterpret_cast<const char *>(aObj) - bufferStore->GetData();
}


int dev() {
    auto bufferStore = new ManualBuffer();
    std::vector<size_t> allOffset;
    std::vector<size_t> frameOffset;

    auto topFrameOffset = createAFrame(bufferStore);
    frameOffset.push_back(topFrameOffset);

    for (int i = 0; i < 1000; i++) {
        auto aframeOffset = createAFrame(bufferStore);
        frameOffset.push_back(aframeOffset);

        auto aString = generateRandomString(10).c_str();
        allOffset.push_back(createAStr(bufferStore, aString));

        auto createAObjResult = createAObj(bufferStore);
        allOffset.push_back(createAObjResult);

        auto createAStringResult_1 = createAStr(bufferStore, "123");
        auto createAStringResult_2 = createAStr(bufferStore, "456");

        auto thisPtr = bufferStore->GetData() + createAObjResult;
        auto thisObj = reinterpret_cast<myObj *>(thisPtr);
        //std::cout << thisObj->keyValueIndex << std::endl;
        thisObj->insertKeyValue("a", createAStringResult_1);
        thisObj->insertKeyValue("b", createAStringResult_2);
        thisObj->writeToBuffer(bufferStore);

        auto aFrame = getPtr<myFrame>(aframeOffset, bufferStore);
        aFrame->insertKeyValue(std::string("obj") + std::to_string(i), createAObjResult);
        aFrame->markEnd(bufferStore);
    }

    getPtr<myFrame>(topFrameOffset, bufferStore)->markEnd(bufferStore);

    for (auto &oneFrameOffset: frameOffset) {
        auto aFrame = getPtr<myFrame>(oneFrameOffset, bufferStore);
        aFrame->readFromBuffer(bufferStore);
        // std::cout << "keySize:" << aFrame->Keys->size() << std::endl;
        // for (auto &oneFrameKey: *(aFrame->Keys)) {
        //     std::cout << oneFrameKey << std::endl;
        // }
        // std::cout << "ValuesOffsetSize:" << aFrame->ValuesOffset->size() << std::endl;
    }

    auto newBufferStore1 = bufferStore->clone();
    auto newBufferStore = newBufferStore1->clone();
    delete bufferStore;
    delete newBufferStore1;

    for (int i = 0; i < 1000; i += 100) {
        // std::cout << "offset:" << allOffset[i] << std::endl;
        auto thisPtr = newBufferStore->GetData() + allOffset[i];
        if (reinterpret_cast<const mBase *>(thisPtr)->type == 1) {
            std::cout << "string value:" << reinterpret_cast<const myString *>(thisPtr)->getValue() << std::endl;
        } else if (reinterpret_cast<const mBase *>(thisPtr)->type == 2) {
            std::cout << "obj value:" << reinterpret_cast<const myObj *>(thisPtr)->Keys->size() << std::endl;
        }

        auto nextPtr = newBufferStore->GetData() + allOffset[i + 1];
        if (reinterpret_cast<const mBase *>(nextPtr)->type == 1) {
            std::cout << "string value:" << reinterpret_cast<const myString *>(nextPtr)->getValue() << std::endl;
        } else if (reinterpret_cast<const mBase *>(nextPtr)->type == 2) {
            std::cout << "obj value:" << reinterpret_cast<const myObj *>(nextPtr)->Keys->size() << std::endl;
        }
    }

    return 0;
}

__attribute__((optimize("O0")))
int testBuffer() {
    auto bufferStore = new ManualBuffer();


    std::vector<size_t> allOffset;

    for (int i = 0; i < 10000; i++) {
        //std::cout <<  std::string("abcdefg" + std::to_string(i)) << std::endl;
        //auto aString = std::string("abcdefg" + std::to_string(i)).c_str();
        auto aString = generateRandomString(10).c_str();
        allOffset.push_back(createAStr(bufferStore, aString));

        auto createAObjResult = createAObj(bufferStore);
        allOffset.push_back(createAObjResult);
    }


    START_TIMER("buffer");
    for (int k = 0; k < 10000; k++) {
        auto newBufferStore1 = bufferStore->clone();
        delete newBufferStore1;
    }
    STOP_TIMER("buffer");
    return 0;
}

__attribute__((optimize("O0")))
void testNew() {
    START_TIMER("new");
    for (int k = 0; k < 10000; k++) {
        for (int i = 0; i < 10000; i++) {
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

void goTest() {
    START_TIMER("new");
    dev();
    STOP_TIMER("new");
    showTimeMNap();
}

void performanceTest() {
    testNew();
    testBuffer();
    showTimeMNap();
}

class will2Buffer {
public:
    std::vector<std::string> data;
};
