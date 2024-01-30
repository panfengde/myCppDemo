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
    base(int aParam) {
        a = aParam + 1000 + 1 + 34 + aParam * 2 + aParam * 2 + aParam * 2 + aParam * 2 + aParam * 2 + aParam * 2 +
            aParam * 2 + aParam * 2 + aParam * 2;
        b = aParam + a;
        c = aParam * b;
        d = aParam * b;
        e = aParam * b;
    }

    string name = "xxx";
    int a;
    int b;
    int c;
    int d;
    int e;
    int f;
    int g;
    int hh;
    int ii;
    int jj;
    int kk;
    int ll;
    int mm;
    int pp;
    int oo;
};

class AType : public base {
public:
    AType(int aParam): base(aParam + 2) {
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
    BType(int aParam): base(aParam + 4) {
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


void testMemory2() {
    std::size_t aBufferSize = sizeof(base) + sizeof(AType) + sizeof(BType);
    alignas(std::max_align_t) char buffer[aBufferSize];

    // 在缓冲区中创建 CustomType 对象
    base* typePtr1 = new(buffer)base(100);
    // 在缓冲区中创建 AnotherType 对象
    AType* typePtr2 = new(buffer + sizeof(base))AType(200);
    BType* typePtr3 = new(buffer + sizeof(base) + sizeof(AType))BType(300);

    // 使用对象
    std::cout << "base name: " << typePtr1->name << std::endl;
    std::cout << "AType hello: " << typePtr2->hello << std::endl;
    std::cout << "BType word: " << typePtr3->word << std::endl;


    clock_t start, end;
    start = clock();
    alignas(std::max_align_t) std::byte buffer2[aBufferSize];

    // 使用 std::memcpy 进行字节级别的拷贝
    std::memcpy(buffer2, buffer, aBufferSize);
    base* clone_typePtr1 = reinterpret_cast<base *>(buffer2);
    AType* clone_typePtr2 = reinterpret_cast<AType *>(buffer2 + sizeof(base));
    BType* clone_typePtr3 = reinterpret_cast<BType *>(buffer2 + sizeof(base) + sizeof(BType));


    std::cout << "base name: " << clone_typePtr1->name << std::endl;
    std::cout << "AType hello: " << clone_typePtr2->hello << std::endl;
    std::cout << "BType word: " << clone_typePtr3->word << std::endl;
    end = clock();
    cout << (double)(end - start) / CLOCKS_PER_SEC * 1000 << "ms" << endl;
    /*// 销毁对象
    destroyObjectInBuffer(typePtr1);
    destroyObjectInBuffer(typePtr2);
    destroyObjectInBuffer(typePtr3);*/
}

int objNumbers = 20000;
int batchNumbers = 10000;

void normalNewTest() {
    clock_t start, end;
    start = clock();
    vector<base *> storeBase;
    vector<AType *> storeAtyoe;
    vector<BType *> storeBtype;
    for (int i = 0; i < batchNumbers; i++) {
        for (int k = 0; k < objNumbers; k++) {
            storeBase.push_back(new base(100));
            storeAtyoe.push_back(new AType(200));
            storeBtype.push_back(new BType(300));
        }
        for (auto one: storeBase) {
            delete one;
            storeBase.clear();
        }
        for (auto one: storeAtyoe) {
            delete one;
            storeAtyoe.clear();
        }
        for (auto one: storeBtype) {
            delete one;
            storeBtype.clear();
        }
    }


    end = clock();
    cout << (double)(end - start) / CLOCKS_PER_SEC * 1000 << "ms" << endl;
}

void specialNewTest() {
    std::size_t aBufferSize = (sizeof(base) + sizeof(AType) + sizeof(BType)) * objNumbers;
    alignas(std::max_align_t) char* sourceBuffer = new char[aBufferSize];

    auto begin = sourceBuffer;
    for (int i = 0; i < objNumbers; i++) {
        new(begin)base(100);
        new(begin + sizeof(base))AType(200);
        new(begin + sizeof(base) + sizeof(AType))BType(300);
        begin += sizeof(base) + sizeof(AType) + sizeof(BType);
    }

    clock_t start, end;
    start = clock();
    alignas(std::max_align_t) std::byte buffer2[aBufferSize];
    vector<char *> bufferStore;

    for (int i = 0; i < batchNumbers; i++) {
        alignas(std::max_align_t) char* aNewBuffer = new char[aBufferSize];
        std::memcpy(aNewBuffer, sourceBuffer, aBufferSize);
        //bufferStore.push_back(aNewBuffer);
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