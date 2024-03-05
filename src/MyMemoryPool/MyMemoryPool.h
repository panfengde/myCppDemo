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
    typedef T *pointer;
    // #define xxT(x) reinterpret_cast<pointer>(x);

private:

    //std::vector<char *> memory = {};
    //二维表，存储的bool数组
    std::vector<bool *> available = {};

    int availAddress[2] = {0, 0};

public:
    static const int POOL_SIZE = 5;
    //二维表，存储的指针数组
    std::vector<pointer *> memory = {};

    MyMemoryPool() {
        applyNewBuffer();
    };

    ~MyMemoryPool() = default;

    void applyNewBuffer() {
        auto availableArray = new bool[POOL_SIZE];
        auto pieceStore = new pointer[POOL_SIZE];
        try {
            char *charBlock = new char[POOL_SIZE * sizeof(T)];
            auto block = reinterpret_cast<pointer>(charBlock);

            for (int i = 0; i < POOL_SIZE; ++i) {
                pieceStore[i] = block + i;
                availableArray[i] = true;
            }
            // 使用内存
        } catch (const std::bad_alloc &e) {
            std::cerr << "内存分配失败:" << e.what() << std::endl;
            // 处理内存分配失败的情况
        }
        memory.push_back(pieceStore);
        available.push_back(availableArray);
        availAddress[0] = memory.size() - 1;
        availAddress[1] = 0;
    }

    pointer allocate() {
        auto thisMemory = memory[availAddress[0]][availAddress[1]];
        available[availAddress[0]][availAddress[1]] = false;

        for (int i = availAddress[1] + 1; i < POOL_SIZE; ++i) {
            if (available[availAddress[0]][i]) {
                availAddress[1] = i;
                return thisMemory;
            }
        }
        for (int row = availAddress[0] + 1; row < available.size(); row++) {
            for (int i = 0; i < POOL_SIZE; ++i) {
                if (available[row][i]) {
                    availAddress[0] = row;
                    availAddress[1] = i;
                    return thisMemory;
                }
            }
        }
        applyNewBuffer();
        return thisMemory;
    }

    void deallocate(pointer ptr) {
        int dealRow = -1;
        int dealCol = -1;

        for (int row = 0; row < memory.size(); row++) {
            ptrdiff_t offset = ptr - memory[row][0];
            if (offset >= 0 && offset < POOL_SIZE) {
                dealRow = row;
                break;
            }
        }
        if (dealRow == -1) {
            std::cout << "deallocate error:" << std::endl;
            return;
        }

        ptrdiff_t offset = ptr - memory[dealRow][0];
        for (int col = 0; col < POOL_SIZE; col++) {
            if (col == offset) {
                dealCol = col;
                break;
            }
        }
        if (dealCol == -1) {
            std::cout << "deallocate error:" << std::endl;
            return;
        }

//        std::cout << "col offset:" << offset << std::endl;
//        std::cout << "dealRow:" << dealRow << "," << "dealCol:" << dealCol << std::endl;
        //只是执行了析构函数，但实际并未释放内存；
        (memory[dealRow][dealCol])->~T();
        available[dealRow][dealCol] = true;

        if (dealRow < availAddress[0]) {
            availAddress[0] = dealRow;
            availAddress[1] = dealCol;
        } else if (availAddress[0] == dealRow && dealCol < availAddress[1]) {
            availAddress[1] = dealCol;
        }
    }

    void showAvailable() {
        std::cout << "currentAddres row:" << availAddress[0] << "Col:" << availAddress[1] << std::endl;
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
            if (canDelete && row != availAddress[0]) {
                cnaDeleteRow.push_back(row);
            }
        }

        for (auto it = cnaDeleteRow.rbegin(); it != cnaDeleteRow.rend(); ++it) {
            auto thisMemoryRowIter = (memory.begin() + *it);
            pointer *rowFirstPiecePtr = *thisMemoryRowIter;
            delete[] rowFirstPiecePtr[0];
            delete[] rowFirstPiecePtr;
            memory.erase(thisMemoryRowIter);

            auto thisAvailableRow = (available.begin() + *it);
            delete[] *thisAvailableRow;
            available.erase(thisAvailableRow);
        }
    }

    //
    // pointer deallocate(pointer ptr) {
    // };
};

#endif //MYMEMORYPOOL_H
