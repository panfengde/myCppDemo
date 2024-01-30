//
// Created by 潘峰 on 2024/1/29.
//
#include <string>
#include <iostream>

using namespace std;

class base {
public:
    base(int aParam) {
        a = aParam + 1000;
        b = aParam + 5000;
        c = aParam * 2;
    }

    string name = "xxx";
    int a;
    int b;
    int c;
};

class AType : public base {
public:
    AType(int aParam): base(aParam) {
    }
    string hello = "123";
};

class BType : public base {
public:
    BType(int aParam): base(aParam) {
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

    alignas(std::max_align_t) std::byte buffer2[aBufferSize];

    // 使用 std::memcpy 进行字节级别的拷贝
    std::memcpy(buffer2, buffer, aBufferSize);
    base* clone_typePtr1 = reinterpret_cast<base *>(buffer2);
    AType* clone_typePtr2 = reinterpret_cast<AType *>(buffer2 + sizeof(base));
    BType* clone_typePtr3 = reinterpret_cast<BType *>(buffer2 + sizeof(base) + sizeof(BType));


    std::cout << "base name: " << clone_typePtr1->name << std::endl;
    std::cout << "AType hello: " << clone_typePtr2->hello << std::endl;
    std::cout << "BType word: " << clone_typePtr3->word << std::endl;

    /*// 销毁对象
    destroyObjectInBuffer(typePtr1);
    destroyObjectInBuffer(typePtr2);
    destroyObjectInBuffer(typePtr3);*/
}
