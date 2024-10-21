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
//#include "CustomAllocator/myString.cpp"
//#include "CustomAllocator/test.cpp"
#include "stringTest/test.cpp"

int main()
{
    testStackBufferString();
    std::cout << "end" << std::endl;
    // goTest();
    return 0;
}
