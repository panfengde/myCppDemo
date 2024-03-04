#include <iostream>
#include <cassert>    // assert()
#include <ctime>      // clock()

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
    clock_t start;
    MyMemoryPool<TestObj> test;
    start = clock();
    for (int i = 0; i < 33; i++) {
        new(test.allocate())TestObj({"name:" + std::to_string(i), "xxxxxx"});
    }
    test.showAvailable();

    for (int i = 0; i < 5; i++) {
        test.deallocate(test.memory[1] + i * sizeof(TestObj));
        test.deallocate(test.memory[2] + i * sizeof(TestObj));
        test.deallocate(test.memory[3] + i * sizeof(TestObj));
    }


    test.showAvailable();
    test.freeIdleBuffer();
    test.showAvailable();

    std::cout << "use Time: ";
    std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";
    std::cout << "Hello, World!" << std::endl;
    return 0;
}

int main_3() {
    clock_t start;
    MyMemoryPool<TestObj> test;
    start = clock();
    for (int i = 0; i < 20; i++) {
        new(test.allocate())TestObj({"name:" + std::to_string(i), "xxxxxx"});
    }
    test.showAvailable();
    test.deallocate(test.memory[0]);
    test.showAvailable();
    test.deallocate(test.memory[1]);

    // //std::cout << (reinterpret_cast<TestObj *>(test.memory[0]))->name << std::endl;
    // for (int i = 0; i < 5; i++) {
    //     std::cout << (test.memory[0] + i * sizeof(TestObj))->name << std::endl;
    // }
    test.showAvailable();
    new(test.allocate())TestObj({"xxx", "xxxxxx"});
    test.showAvailable();
    new(test.allocate())TestObj({"pppp", "xxxxxx"});
    test.showAvailable();
    std::cout << "use Time: ";
    std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";

    std::cout << "Hello, World!" << std::endl;
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
