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
        a = aParam + 1000;
    }

    string name = "xxx";
    int a;

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

int objNumbers = 20000;
int batchNumbers = 10000;

void useSpecialNew() {
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
    vector<char *> bufferStore;
    alignas(std::max_align_t) char* aNewBuffer = new char[aBufferSize];
    std::memcpy(aNewBuffer, sourceBuffer, aBufferSize);

    base* clone_typePtr1 = reinterpret_cast<base *>(aNewBuffer);
    AType* clone_typePtr2 = reinterpret_cast<AType *>(aNewBuffer + sizeof(base));
    BType* clone_typePtr3 = reinterpret_cast<BType *>(aNewBuffer + sizeof(base) + sizeof(AType));

    std::cout << "base name: " << clone_typePtr1->name << std::endl;
    std::cout << "AType hello: " << clone_typePtr2->hello << std::endl;
    std::cout << "BType word: " << clone_typePtr3->word << std::endl;

    delete[] aNewBuffer;
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

