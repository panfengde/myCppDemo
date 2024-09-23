//
// Created by 潘峰 on 2024/9/20.
//
class ManualBuffer
{
public:
    ManualBuffer() : offset(0), buffer(std::make_shared<std::vector<char>>())
    {
        // 初始化缓冲区，预分配 1 MB
        buffer->reserve(1024 * 1024);
    }

    char* allocate(size_t needDataSize)
    {
        ensureUniqueBuffer(); // 在修改前确保 buffer 是唯一的（写时拷贝）

        if (offset + needDataSize > buffer->size())
        {
            // 扩容策略，几何扩展
            size_t newCapacity = buffer->size() + std::max(buffer->size() / 2, needDataSize);
            buffer->resize(newCapacity);
        }

        char* currentPtr = buffer->data() + offset;
        offset += needDataSize;
        return currentPtr;
    }

    // 获取buffer数据指针
    char* GetData()
    {
        return buffer->data();
    }

    // 获取buffer大小
    size_t GetSize() const
    {
        return offset;
    }

    // 克隆方法
    [[nodiscard]] ManualBuffer* clone() const
    {
        auto* newBuffer = new ManualBuffer();
        newBuffer->offset = offset;
        newBuffer->buffer = buffer; // 共享buffer
        return newBuffer;
    }

    size_t offset; // 当前偏移
private:
    // 确保 buffer 是唯一的（写时拷贝）
    void ensureUniqueBuffer()
    {
        if (!buffer.unique())
        {
            // 如果 buffer 被多个对象共享，进行深拷贝
            buffer = std::make_shared<std::vector<char>>(*buffer);
        }
    }

    std::shared_ptr<std::vector<char>> buffer; // 使用 shared_ptr 来管理共享内存
};