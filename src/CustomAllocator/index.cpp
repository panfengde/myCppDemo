//
// Created by 潘峰 on 2024/9/18.
//
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <cstddef>
#include <cstring>
#include "./myString.cpp"

class MemoryPool
{
public:
    MemoryPool(size_t poolSize)
        : poolSize_(poolSize), used_(0)
    {
        pool_ = new char[poolSize_];
    }

    ~MemoryPool()
    {
        delete[] pool_;
    }

    void* allocate(size_t size)
    {
        if (used_ + size > poolSize_)
        {
            throw std::bad_alloc();
        }
        void* ptr = pool_ + used_;
        used_ += size;
        return ptr;
    }

    void deallocate(void* ptr, size_t size)
    {
        // 简单实现不做任何处理，实际可以实现空闲列表
    }

private:
    char* pool_;
    size_t poolSize_;
    size_t used_;
};

template <typename T>
class CustomAllocator
{
public:
    using value_type = T;

    CustomAllocator(MemoryPool& pool) : pool_(pool)
    {
    }

    template <typename U>
    CustomAllocator(const CustomAllocator<U>& other) : pool_(other.pool_)
    {
    }

    T* allocate(size_t n)
    {
        return static_cast<T*>(pool_.allocate(n * sizeof(T)));
    }

    void deallocate(T* p, size_t n)
    {
        pool_.deallocate(p, n * sizeof(T));
    }

    template <typename U>
    bool operator==(const CustomAllocator<U>& other) const
    {
        return &pool_ == &other.pool_;
    }

    template <typename U>
    bool operator!=(const CustomAllocator<U>& other) const
    {
        return &pool_ != &other.pool_;
    }

private:
    MemoryPool& pool_;

    template <typename U>
    friend class CustomAllocator;
};

class aTest
{
public:
    using Allocator = CustomAllocator<std::string>;
    std::vector<std::string, Allocator> keys;

    aTest(MemoryPool& pool)
        : keys(Allocator(pool))
    {
    }

    // 复制构造函数
    aTest(const aTest& other, MemoryPool& pool)
        : keys(other.keys.begin(), other.keys.end(), Allocator(pool))
    {
    }
};

int test()
{
    // 分配足够大的内存池
    size_t poolSize = 1024 * 1024; // 1MB
    MemoryPool pool(poolSize);

    // 创建对象
    aTest original(pool);
    original.keys.push_back("Hello");
    original.keys.push_back("World");

    // 克隆对象
    aTest clone(original, pool);

    // 打印克隆对象中的值
    for (const auto& key : clone.keys)
    {
        std::cout << key << std::endl;
    }

    return 0;
}

class myString
{
    std::string value = "123";
};
