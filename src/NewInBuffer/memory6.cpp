//
// Created by 潘峰 on 2024/1/29.
//
#include <string>
#include <iostream>
#include <ctime>
#include <vector>

using namespace std;

class base {
public:
    base() {
    }

    base(vector<string> _data) {
        data = _data;
    }

    string name = "xxx";
    vector<string> data;
};

class AType : public base {
public:
    AType() {
        data = vector<string>{"aA", "aA", "aA", "aA", "aA", "aA", "aA",};
    }

    AType(AType* other) {
        data = other->data;
    }

    void aprint1() {
        cout << "print" << endl;
    }

    void aprint2() {
        cout << "print" << endl;
    }

    void aprint3() {
        cout << "print" << endl;
    }

    string hello = "123";
};

class BType : public base {
public:
    BType() {
        data = vector<string>{"aB", "aB", "aB", "aB", "aB", "aB",};
    }

    BType(BType* other) {
        data = other->data;
    }

    void baprint1() {
        cout << "print b" << endl;
    }

    void baprint2() {
        cout << "print b" << endl;
    }

    void baprint3() {
        cout << "print b" << endl;
    }

    string word = "456";
};


int objNumbers = 30000;
int batchNumbers = 1000;

void normalNewTest() {
    clock_t start, end;
    start = clock();
    vector<base *> storeBase;
    vector<AType *> storeAtyoe;
    vector<BType *> storeBtype;
    for (int i = 0; i < batchNumbers; i++) {
        for (int k = 0; k < objNumbers; k++) {
            storeBase.push_back(new base(vector<string>{"a1", "a1", "a1", "a1", "a1",}));
            storeAtyoe.push_back(new AType());
            storeBtype.push_back(new BType());
        }
        for (auto one: storeBase) {
            delete one;
        }
        for (auto one: storeAtyoe) {
            delete one;
        }
        for (auto one: storeBtype) {
            delete one;
        }
        storeBase.clear();
        storeAtyoe.clear();
        storeBtype.clear();
    }


    end = clock();
    cout << (double)(end - start) / CLOCKS_PER_SEC * 1000 << "ms" << endl;
}

void specialNewTest() {
    std::size_t aBufferSize = (sizeof(base) + sizeof(AType) + sizeof(BType)) * objNumbers;
    alignas(std::max_align_t) char* sourceBuffer = new char[aBufferSize];

    auto begin = sourceBuffer;
    for (int i = 0; i < objNumbers; i++) {
        new(begin)base(vector<string>{"a1", "a1", "a1", "a1", "a1",});
        new(begin + sizeof(base))AType();
        new(begin + sizeof(base) + sizeof(AType))BType();
        begin += sizeof(base) + sizeof(AType) + sizeof(BType);
    }

    clock_t start, end;
    start = clock();

    for (int patchIndex = 0; patchIndex < batchNumbers; patchIndex++) {
        alignas(std::max_align_t) char* aNewBuffer = new char[aBufferSize];
        std::memcpy(aNewBuffer, sourceBuffer, aBufferSize);
        auto batchBegin = aNewBuffer;
        for (int i = 0; i < objNumbers; i++) {
            auto one = reinterpret_cast<base *>(batchBegin);
            auto two = reinterpret_cast<AType *>(batchBegin + sizeof(base));
            auto three = reinterpret_cast<BType *>(batchBegin + sizeof(base) + sizeof(AType));

            new(one)base(one->data);
            new(two)AType(two);
            new(three)BType(three);
            batchBegin += sizeof(base) + sizeof(AType) + sizeof(BType);
        }

        for (int i = 0; i < objNumbers; i++) {
            auto one = reinterpret_cast<base *>(batchBegin);
            auto two = reinterpret_cast<AType *>(batchBegin + sizeof(base));
            auto three = reinterpret_cast<BType *>(batchBegin + sizeof(base) + sizeof(AType));
            one->~base();
            two->~AType();
            three->~BType();
            batchBegin += sizeof(base) + sizeof(AType) + sizeof(BType);
        }
        // bufferStore.push_back(aNewBuffer);
        delete[] aNewBuffer;
    }
    // for (auto one: bufferStore) {
    //     // 释放动态分配的内存
    //     delete[] one;
    // }
    end = clock();
    cout << (double)(end - start) / CLOCKS_PER_SEC * 1000 << "ms" << endl;
    /*// 销毁对象
    destroyObjectInBuffer(typePtr1);
    destroyObjectInBuffer(typePtr2);
    destroyObjectInBuffer(typePtr3);*/
}


/*
int objNumbers = 20000;
int batchNumbers = 10000;
63623.1ms
4146ms
Hello, World!
*/
