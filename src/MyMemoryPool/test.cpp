#include "MyMemoryPool/MyMemoryPool.h"
#include <vector>
#include <string>
#include <map>

using namespace std;

class other {
public:
    int xx = 123;
};


class aBase {
public:
    std::string name;
    std::vector<std::string *> data;
    std::map<string, other *> mapData;

    virtual void hello() {
        std::cout << "Base--" << std::endl;
    };

    ~aBase() {};
};

class aOne : public aBase {
public:
    aOne(std::string _name) {
        name = _name;
        data.push_back(new std::string(_name));
        data.push_back(new std::string(_name));
    }

    ~aOne() {};

    void hello() override {
        std::cout << "one--" << std::endl;
    };

    void xx() {
        std::cout << "xx--" << std::endl;
    };
};

void test() {
    clock_t start;
    MyMemoryPool<aOne> test;
    start = clock();
    for (int i = 0; i < 33; i++) {
        new(test.allocate())aOne({"name:" + std::to_string(i)});
    }
    test.showAvailable();

    for (int i = 0; i < MyMemoryPool<aOne>::POOL_SIZE; i++) {
        test.deallocate(test.memory[1][i]);
        test.deallocate(test.memory[2][i]);
        test.deallocate(test.memory[3][i]);
    }


    test.showAvailable();
    test.freeIdleBuffer();
    test.showAvailable();

    std::cout << "use Time: ";
    std::cout << (((double) clock() - start) / CLOCKS_PER_SEC) << "\n\n";
    std::cout << "Hello, World!" << std::endl;
}