//
// Created by 潘峰 on 2024/9/19.
//
#include <iostream>
#include <string>
#include <cstring>
#include <new>    // For placement new

#include <random>

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
class ManualBuffer;

std::vector<std::vector<std::string> > allKeys;
std::vector<std::vector<size_t> > allValues;

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
    size_t keyValueIndex = 0;

    myObj() {
        type = 2;
        allKeys.emplace_back();
        allValues.emplace_back();
        keyValueIndex = allKeys.size() - 1;
    }

    void insertKeyValue(std::string key, size_t valueOffset) const {
        allKeys[keyValueIndex].push_back(key);
        allValues[keyValueIndex].push_back(valueOffset);
    }

    void sayHello() const {
        std::cout << keyValueIndex << std::endl;
    }

    void getValueByName(std::string &key, mBase *value) const {
    }

};

class ManualBuffer {
public:
    ManualBuffer() : buffer(nullptr), capacity(0), offset(0) {
        // 使用 realloc 分配 1 MB (1 MB = 1024 * 1024 字节)
        buffer = reinterpret_cast<char *>(std::realloc(buffer, 1024 * 1024));
    }

    ~ManualBuffer() {
        if (buffer) {
            std::free(buffer);
        }
    }

    char *allocate(size_t needDataSize) {
        if (capacity < offset + needDataSize) {
            // 增加缓冲区容量
            capacity = capacity + 1 * 1024;
            std::cout << "add---" << std::endl;
            buffer = static_cast<char *>(std::realloc(buffer, capacity));
        }
        auto currentPtr = buffer + offset;
        offset += needDataSize;
        return currentPtr;
    };

    // 获取buffer数据指针
    const void *GetData() const {
        return buffer;
    }

    // 获取buffer大小
    size_t GetSize() const {
        return offset;
    }

    // 克隆方法
    [[nodiscard]] ManualBuffer *clone() const {
        auto *newBuffer = new ManualBuffer();
        newBuffer->capacity = capacity;
        newBuffer->offset = offset;

        // 复制数据
        if (buffer && offset > 0) {
            newBuffer->buffer = static_cast<char *>(std::malloc(capacity));
            std::memcpy(newBuffer->buffer, buffer, offset);
        }

        return newBuffer;
    }


    char *buffer;

private:
    size_t capacity; //容量
    size_t offset; //当前位置
};

size_t createAStr(ManualBuffer *bufferStore, std::string &strValue) {
    auto cStr = strValue.c_str();
    // 计算字符串长度
    size_t len = strlen(cStr);
    auto thisBuffer = bufferStore->buffer;
    std::strcpy(bufferStore->allocate(len + 1), cStr);
    auto aStr = new(bufferStore->allocate(sizeof(myString))) myString(len + 1);

    return reinterpret_cast<const char *>(aStr) - thisBuffer;
}

size_t createAStr(ManualBuffer *bufferStore, const char *cStr) {
    auto thisBuffer = bufferStore->buffer;
    // 计算字符串长度
    size_t len = strlen(cStr);

    std::strcpy(bufferStore->allocate(len + 1), cStr);
    auto aStr = new(bufferStore->allocate(sizeof(myString))) myString(len + 1);

    return reinterpret_cast<const char *>(aStr) - thisBuffer;
}

size_t createAObj(ManualBuffer *bufferStore) {
    auto thisBuffer = bufferStore->buffer;
    auto aObj = new(bufferStore->allocate(sizeof(myObj))) myObj();
    return reinterpret_cast<const char *>(aObj) - thisBuffer;
}

void *createAStr(std::string &strValue) {
    auto cStr = strValue.c_str();
}

int testMystring() {
    auto bufferStore = new ManualBuffer();


    std::vector<size_t> allOffset;

    for (int i = 0; i < 2000; i++) {
        //std::cout <<  std::string("abcdefg" + std::to_string(i)) << std::endl;
        //auto aString = std::string("abcdefg" + std::to_string(i)).c_str();
        auto aString = generateRandomString(10).c_str();
        allOffset.push_back(createAStr(bufferStore, aString));

        auto createAObjResult = createAObj(bufferStore);
        allOffset.push_back(createAObjResult);

        auto createAStringResult_1 = createAStr(bufferStore, "123");
        auto createAStringResult_2 = createAStr(bufferStore, "456");

        auto thisPtr = bufferStore->buffer + createAObjResult;
        auto thisObj = reinterpret_cast<myObj *>(thisPtr);


        std::cout << thisObj->keyValueIndex << std::endl;
        // thisObj->insertKeyValue("a", createAStringResult_1);
        // thisObj->insertKeyValue("b", createAStringResult_2);
    }

    std::cout << "ok---" << std::endl;

    auto newBufferStore = bufferStore->clone();

    for (int i = 0; i < 2000; i += 100) {
        std::cout << "offset:" << allOffset[i] << std::endl;
        auto thisPtr = newBufferStore->buffer + allOffset[i];
        if (reinterpret_cast<mBase *>(thisPtr)->type == 1) {
            std::cout << "string value:" << reinterpret_cast<myString *>(thisPtr)->getValue() << std::endl;
        } else if (reinterpret_cast<mBase *>(thisPtr)->type == 2) {
            std::cout << "obj value:" << reinterpret_cast<myObj *>(thisPtr)->keyValueIndex << std::endl;
        }

        auto nextPtr = newBufferStore->buffer + allOffset[i + 1];
        if (reinterpret_cast<mBase *>(nextPtr)->type == 1) {
            std::cout << "string value:" << reinterpret_cast<myString *>(nextPtr)->getValue() << std::endl;
        } else if (reinterpret_cast<mBase *>(nextPtr)->type == 2) {
            std::cout << "obj value:" << reinterpret_cast<myObj *>(nextPtr)->keyValueIndex << std::endl;
        }
    }

    return 0;
}
