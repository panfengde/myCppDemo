//
// Created by 59546 on 2024/10/16.
//
//
// Created by 潘峰 on 2024/9/25.
//

#include <vector>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stack>
#include <memory>
#include <cstring>

//extern BufferCache GlobalBufferCache;s

using namespace std;


class StackBuffer {
public:
    StackBuffer() : currentOffset(0) {
        // buffer.resize( 1024); // 初始化缓冲区，预分配 1 MB
        buffer.resize(8); // 初始化缓冲区，预分配 1 B
    }

    ~StackBuffer() {
        vector<char>().swap(buffer);
        // if (!GlobalBufferCache.exists(cloneStart)) {
        //     GlobalBufferCache.insert(cloneStart, std::move(buffer));
        // }

        //
        //cout << "delte StackBuffer" << endl;
    };

    char *allocate(size_t needDataSize) {
        if (currentOffset + needDataSize > buffer.size()) {
            // 增加缓冲区容量
            size_t newCapacity = 2 * (needDataSize + buffer.size()); // 扩容策略
            //size_t newCapacity = needDataSize + buffer.size(); // 扩容策略
            buffer.resize(newCapacity);
        }
        char *currentPtr = buffer.data() + currentOffset;
        currentOffset += needDataSize;
        return currentPtr;
    }

    // 获取buffer数据指针
    char *GetData() { return buffer.data(); }

    // 获取buffer大小
    size_t GetSize() const { return currentOffset; }

    void shrink_to_fit() {
        if (currentOffset < buffer.size()) {
            buffer.resize(currentOffset);
            buffer.shrink_to_fit();
        }
    }

    // 克隆方法
    [[nodiscard]] StackBuffer *clone() const {
        auto *newBuffer = new StackBuffer();
        newBuffer->currentOffset = currentOffset;
        newBuffer->buffer = buffer; // 直接复制 vector，避免手动复制
        newBuffer->bufferOffsetComparedOriginBuffer = bufferOffsetComparedOriginBuffer;
        return newBuffer;
    }

    [[nodiscard]] StackBuffer *clone(size_t buffersStart, size_t bufferEnd) const {
        auto *newBuffer = new StackBuffer();
        size_t cloneSize = bufferEnd - buffersStart;
        if (bufferEnd < buffersStart) {
            cout << "clone--error" << endl;
        }
        newBuffer->buffer.resize(cloneSize); // 预分配目标buffer大小
        memcpy(newBuffer->buffer.data(), buffer.data() + buffersStart, cloneSize); // 直接复制内存块

        newBuffer->currentOffset = currentOffset;
        newBuffer->bufferOffsetComparedOriginBuffer = buffersStart;
        return newBuffer;
    }

    [[nodiscard]] __attribute__((always_inline)) bool isInStackBuffer(void *atom) {
        auto begin = buffer.data();
        return atom >= begin && atom <= begin + buffer.size();
    }

    size_t cloneStart = 0;
    std::vector<char> buffer; // 使用 std::vector 代替裸指针
    size_t currentOffset; // 当前偏移
    size_t bufferOffsetComparedOriginBuffer = 0; // 当前buffer相对于原始buffer的偏移
};

string aStringInfo = "qwertyuiop";

class go {
public:
    const char* name=nullptr;
    int age = 0;
    size_t xx;
    void sayHello() { std::cout << "" << name<< endl; }

    void get() {

        name = reinterpret_cast<const char *>(this) + sizeof(go);
    }
};

void testStackBufferString() {
    auto aStackBuffer = new StackBuffer();

    vector<go *> all={};
    for (int i = 0; i < 200; i++) {
        auto one = new(aStackBuffer->allocate(sizeof(go) + aStringInfo.size()))go();
        auto oneOffset = reinterpret_cast<char *>(one) - aStackBuffer->GetData();
        memcpy(reinterpret_cast<char *>(one) + sizeof(go), aStringInfo.c_str(), aStringInfo.size());

        // if (i == 10) {
        //     onePoint = reinterpret_cast<go *>(aStackBuffer->GetData() + oneOffset);
        //
        // }
        all.push_back(reinterpret_cast<go *>(aStackBuffer->GetData() + oneOffset));
    }



    int k=0;
    for(auto aPoint : all) {
        cout<<k++<<endl;
        aPoint->get();
    }


}
