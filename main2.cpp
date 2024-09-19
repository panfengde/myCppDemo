#include <iostream>
#include <cassert>    // assert()
#include <ctime>      // clock()
#include <vector>

// #include "variant/variant.cpp"
// #include "NewInBuffer/memory1.cpp"
// #include "NewInBuffer/memory2.cpp"
// #include "NewInBuffer/memory3.cpp"
// #include "NewInBuffer/memory4.cpp"
//#include "NewInBuffer/memory5.cpp"
// #include "NewInBuffer/memory6.cpp"

#include "MemoryPool/MemoryPool.h"
#include "MemoryPool/StackAlloc.h"
#include "MyMemoryPool/MyMemoryPool.h"
#include "MyMemoryPool/test.cpp"

// 插入元素个数
#define ELEMS 100
// 重复次数
#define REPS 2



struct TestObj {
    std::string name;
    std::string des;

    ~TestObj() {
        name = "00000000";
        std::cout << "~TestObj" << std::endl;
    };
};


int main() {
    test();
    return 0;
}

int main_2() {
    clock_t start;
    // 使用内存池
    MemoryPool<TestObj> TestObjPool;
    start = clock();

    auto point1 = TestObjPool.allocate(1);
    TestObjPool.construct(point1, TestObj{"11111", "111111"});

    auto point2 = TestObjPool.allocate(1);
    TestObjPool.construct(point2, TestObj{"22222", "222222"});

    std::cout << point1->name << std::endl;
    std::cout << "---------" << std::endl;
    std::cout << point2->name << std::endl;

    TestObjPool.deallocate(point1);
    auto point3 = TestObjPool.allocate(1);
    TestObjPool.construct(point3, TestObj{"333", "333"});
    std::cout << "---------" << std::endl;
    std::cout << point1->name << std::endl;
    std::cout << point3->name << std::endl;

    std::cout << "Hello, World!" << std::endl;
    return 0;
}

int main_1() {
    // testVariant();
    // testMemory();
    // testMemory2();
    // normalNewTest();
    // specialNewTest();
    // useSpecialNew();
    // isBufferEnough();
    // isBufferEnough2();
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
