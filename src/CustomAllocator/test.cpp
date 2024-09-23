//
// Created by 潘峰 on 2024/9/23.
//
#include <iostream>
#include <string>
#include <cstring>
#include <new>    // For placement new
#include <vector>
#include <random>

void writeToBuffer(const std::vector<std::string>& keys,
                   const std::vector<size_t>& valuesOffset,
                   std::vector<char>& buffer)
{
    size_t totalSize = sizeof(size_t) + valuesOffset.size() * sizeof(size_t);
    for (const auto& key : keys)
    {
        totalSize += sizeof(size_t) + key.size(); // 保存每个字符串的大小和内容
    }
    size_t bufferOffset = 0;
    buffer.resize(buffer.size() + totalSize);
    char* ptr = buffer.data() + bufferOffset;

    auto offsetSize = valuesOffset.size();
    memcpy(ptr, &offsetSize, sizeof(size_t));
    ptr += sizeof(size_t);
    memcpy(ptr, valuesOffset.data(), valuesOffset.size() * sizeof(size_t));
    ptr += valuesOffset.size() * sizeof(size_t);

    for (const auto& key : keys)
    {
        size_t keySize = key.size();
        memcpy(ptr, &keySize, sizeof(size_t));
        ptr += sizeof(size_t);
        memcpy(ptr, key.data(), keySize);
        ptr += keySize;
    }
}

void readFromBuffer(const std::vector<char>& buffer, std::vector<std::string>& keys, std::vector<size_t>& valuesOffset)
{
    const char* ptr = buffer.data();

    // 读取ValuesOffset
    size_t offsetSize;
    memcpy(&offsetSize, ptr, sizeof(size_t));
    ptr += sizeof(size_t);
    valuesOffset.resize(offsetSize);
    memcpy(valuesOffset.data(), ptr, offsetSize * sizeof(size_t));
    ptr += offsetSize * sizeof(size_t);

    // 读取keys
    keys.clear();
    for (size_t i = 0; i < offsetSize; ++i)
    {
        size_t keySize;
        memcpy(&keySize, ptr, sizeof(size_t)); // 读取字符串大小
        ptr += sizeof(size_t);
        std::string key(ptr, keySize); // 读取字符串内容
        keys.push_back(std::move(key));
        ptr += keySize;
    }
}

void aTest()
{
    std::vector<std::string> keys = {"key1", "key2", "key3"};
    std::vector<size_t> valuesOffset = {0, 1, 2};

    std::vector<char> buffer;
    writeToBuffer(keys, valuesOffset, buffer);

    std::vector<std::string> readKeys;
    std::vector<size_t> readValuesOffset;
    readFromBuffer(buffer, readKeys, readValuesOffset);

    // 输出结果
    for (const auto& key : readKeys)
    {
        std::cout << key << std::endl;
    }
    for (const auto& offset : readValuesOffset)
    {
        std::cout << offset << std::endl;
    }
}
