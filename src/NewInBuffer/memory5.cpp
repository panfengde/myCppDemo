//
// Created by 潘峰 on 2024/1/31.
//
#include <string>
#include <iostream>
#include <ctime>
#include <vector>

using namespace std;

class AType {
public:
    AType() {
        vec=*(new std::vector<char>{});
    }

    string name = "";
    std::vector<char> vec;
};


void isBufferEnough() {
    std::size_t aBufferSize = sizeof(AType);
    alignas(std::max_align_t) char* sourceBuffer = new char[aBufferSize];

    // 在预先分配的内存中构造对象
    AType* obj = new(sourceBuffer) AType;
    // 修改 std::string 和 std::vector 内容
    obj->name = "name";
    for(int i=0;i<100;i++) {
        obj->vec.push_back('p');
    }
    // 使用 obj 对象
    std::cout << obj->name << std::endl;
    std::cout << sizeof(AType) << std::endl;
    std::cout << sizeof(obj) << std::endl;
    std::cout << obj->vec[50] << std::endl;
}

void isBufferEnough2() {
    std::size_t aBufferSize = sizeof(AType);
    alignas(std::max_align_t) char* sourceBuffer = new char[aBufferSize];

    // 在预先分配的内存中构造对象
    AType* obj = new(sourceBuffer) AType;
    // 修改 std::string 和 std::vector 内容
    obj->name = "name";
    for(int i=0;i<100;i++) {
        obj->vec.push_back('p');
    }
    // 使用 obj 对象
    std::cout << sizeof(AType) << std::endl;
    std::cout << sizeof(obj) << std::endl;
    std::cout << obj->name << std::endl;
    std::cout << obj->vec[1] << std::endl;


    alignas(std::max_align_t) char* aNewBuffer = new char[aBufferSize];
    std::memcpy(aNewBuffer, sourceBuffer, aBufferSize);

    delete obj;


    AType* clone_typePtr1 = reinterpret_cast<AType *>(aNewBuffer);
    // clone_typePtr1->vec={'x','c'};
    // clone_typePtr1->name="newxx";
    // 注意：不要使用 delete，而是显式调用对象的析构函数


    std::cout << "--------clone-------" << std::endl;
    std::cout << sizeof(clone_typePtr1) << std::endl;
    std::cout << clone_typePtr1->name << std::endl;
    std::cout << clone_typePtr1->vec[1] << std::endl;

}