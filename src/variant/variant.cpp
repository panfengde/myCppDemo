//
// Created by 潘峰 on 2024/1/29.
//
#include <variant>
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
    string word = "456";;
};

void testVariant() {
    constexpr size_t numObjects = 3;
    auto memoryBlock = new std::variant<AType, BType, base>[numObjects];

    memoryBlock[0] = base();
    memoryBlock[1] = AType();
    memoryBlock[2] = BType();

    auto&obj1 = std::get<base>(memoryBlock[0]);
    auto&obj2 = std::get<AType>(memoryBlock[1]);
    auto&obj3 = std::get<BType>(memoryBlock[2]);
    cout << "obj1:" << obj1.name << endl;
    cout << "obj2:" << obj2.hello << endl;
    cout << "obj3:" << obj3.word << endl;
    delete[] memoryBlock;
}
