//
// Created by 潘峰 on 2024/1/29.
//
#include <string>
#include <iostream>

using namespace std;

class base {
public:
    string name = "xxx";
};

class AType : public base {
public:
    string hello = "123";
};

class BType : public base {
public:
    string word = "456";
};


template<std::size_t Size, std::size_t Align>
struct AlignedBuffer {
    // alignas是用来控制数据的内存对齐的关键字，alignas(n) type variable
    alignas(Align) std::byte buffer[Size];
};

template<typename T>
T* createObjectInBuffer(std::byte* buffer, T&&value) {
    //std::forward 是和std::move相关的左值右值转发问题--
    return new(buffer) T(std::forward<T>(value)); //将T类型的值，转移到buffer中
}

template<typename T>
void destroyObjectInBuffer(T* obj) {
    obj->~T();
}

//创建一个内存buffer，并在其中创建对象
void testMemory() {
    //std::max_align_t 作为一个足够大的对齐要求的类型，通常能够满足大多数自定义类型的对齐需求
    AlignedBuffer<sizeof(base) + sizeof(AType) + sizeof(BType), alignof(std::max_align_t)> bufferStore;

    // 在缓冲区中创建 CustomType 对象
    base* typePtr1 = createObjectInBuffer(bufferStore.buffer, base());
    // 在缓冲区中创建 AnotherType 对象
    AType* typePtr2 = createObjectInBuffer(bufferStore.buffer + sizeof(base), AType());
    BType* typePtr3 = createObjectInBuffer(bufferStore.buffer + sizeof(base) + sizeof(AType), BType());

    // 使用对象
    std::cout << "base name: " << typePtr1->name << std::endl;
    std::cout << "AType hello: " << typePtr2->hello << std::endl;
    std::cout << "BType word: " << typePtr3->word << std::endl;

    // 销毁对象
    destroyObjectInBuffer(typePtr1);
    destroyObjectInBuffer(typePtr2);
    destroyObjectInBuffer(typePtr3);

    // 分配足够的内存以容纳所有对象
    // void* memory = operator new(sizeof(base) + sizeof(AType) + sizeof(BType));
}
