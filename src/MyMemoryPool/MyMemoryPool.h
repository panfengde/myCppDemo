//
// Created by 潘峰 on 2024/3/3.
//

#ifndef MYMEMORYPOOL_H
#define MYMEMORYPOOL_H
#include <vector>

struct Temp {
};

template<typename T>
class MyMemoryPool {
    // 使用 typedef 简化类型书写
    typedef T* pointer;
    // #define xxT(x) reinterpret_cast<pointer>(x);

private:
    static const int POOL_SIZE = 5;
    //std::vector<char *> memory = {};
    std::vector<bool *> available = {};

    int availAddrs[2] = {0, 0};

public:
    std::vector<pointer> memory = {};

    MyMemoryPool() {
        applyNewBufer();
    };

    ~MyMemoryPool() = default;

    void applyNewBufer() {
        alignas(std::max_align_t) char* sourceBuffer = new char[POOL_SIZE * sizeof(T)];
        memory.push_back(reinterpret_cast<pointer>(sourceBuffer));

        auto availableArray = new bool[POOL_SIZE];
        for (int i = 0; i < POOL_SIZE; ++i) {
            availableArray[i] = true;
        }
        available.push_back(availableArray);

        availAddrs[0] = memory.size() - 1;
        availAddrs[1] = 0;
    }

    pointer allocate() {
        auto thisMemory = memory[availAddrs[0]] + availAddrs[1] * sizeof(T);
        available[availAddrs[0]][availAddrs[1]] = false;

        for (int i = availAddrs[1] + 1; i < POOL_SIZE; ++i) {
            if (available[availAddrs[0]][i]) {
                availAddrs[1] = i;
                return thisMemory;
            }
        }
        for (int row = availAddrs[0] + 1; row < available.size(); row++) {
            for (int i = 0; i < POOL_SIZE; ++i) {
                if (available[row][i]) {
                    availAddrs[0] = row;
                    availAddrs[1] = i;
                    return thisMemory;
                }
            }
        }
        applyNewBufer();
        return thisMemory;
    }

    void deallocate(pointer ptr) {
        int dealRow = 0;
        int dealCol = 100;

        for (int row = 0; row < memory.size(); row++) {
            ptrdiff_t offset = ptr - memory[row];
            if (offset % sizeof(T) == 0 && offset >= 0 && offset <= POOL_SIZE * sizeof(T)) {
                dealRow = row;
                break;
            }
        }

        ptrdiff_t offset = ptr - memory[dealRow];
        for (int col = 0; col < POOL_SIZE; col++) {
            if (col * sizeof(T) == offset) {
                dealCol = col;
                break;
            }
        }
        //只是执行了析构函数，但实际并未释放内存；
        (memory[dealRow] + dealCol * sizeof(T))->~T();
        available[dealRow][dealCol] = true;
        // std::cout << "col offset:" << offset << std::endl;
        // std::cout << "dealRow:" << dealRow << "," << "dealCol:" << dealCol << std::endl;
        if (dealRow < availAddrs[0]) {
            availAddrs[0] = dealRow;
            availAddrs[1] = dealCol;
        }
        else if (availAddrs[0] == dealRow && dealCol < availAddrs[1]) {
            availAddrs[1] = dealCol;
        }
    }

    void showAvailable() {
        std::cout << "currentAddres row:" << availAddrs[0] << "Col:" << availAddrs[1] << std::endl;
        for (int row = 0; row < available.size(); row++) {
            std::cout << (row < 10 ? "row: " + std::to_string(row) : "row:" + std::to_string(row)) << "  ";
            for (int i = 0; i < POOL_SIZE; i++) {
                std::cout << (available[row][i] ? "canUse" : "isUsed") << "  ";
            }
            std::cout << std::endl;
        }
    }

    void freeIdleBuffer() {
        //为了保证，正确的erase，一定要保证cnaDeleteRow存储的地址是前面小，后面大。
        std::vector<size_t> cnaDeleteRow;

        for (int row = 0; row < available.size(); row++) {
            bool canDelete = true;
            for (int col = 0; col < POOL_SIZE; col++) {
                canDelete = available[row][col];
                if (!canDelete) {
                    break;
                }
            }
            if (canDelete && row != availAddrs[0]) {
                cnaDeleteRow.push_back(row);
            }
        }

        for (auto it = cnaDeleteRow.rbegin(); it != cnaDeleteRow.rend(); ++it) {
            auto aBufferIter = memory.begin() + *it;
            auto aBuffer = reinterpret_cast<char *>(*aBufferIter);
            delete[] aBuffer;
            memory.erase(aBufferIter);

            auto aVailableIter = available.begin() + *it;
            auto aVailable = *aVailableIter;
            delete[] aVailable;
            available.erase(aVailableIter);
        }
    }

    //
    // pointer deallocate(pointer ptr) {
    // };
};
#endif //MYMEMORYPOOL_H
