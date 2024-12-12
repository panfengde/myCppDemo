#include <FFI/ffi.h>
#include <stdio.h>
#include <iostream>
/*void callback(ffi_cif *cif, void *ret, void **args, void *userdata) {
    printf("Callback invoked with arg: %d\n", *(int *) args[0]);
    *(int *) ret = *(int *) args[0] * 2;
}*/
// 定义 xxxFN 函数
void xxxFN() {
    std::cout << "runFN:" << std::endl;
}
int main() {
    ffi_cif cif;
    ffi_closure *closure; // 回调闭包
    //int (*func)(int);     // 函数指针
    void *func;
    ffi_type *args[1]; // 参数类型
    int x = 5, result;

    // 设置参数类型
    args[0] = &ffi_type_sint;

    // 准备调用接口
    if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 1, &ffi_type_sint, args) != FFI_OK) {
        fprintf(stderr, "Failed to prepare cif\n");
        return 1;
    }

    // 分配闭包并绑定到函数指针
    closure = (ffi_closure *) ffi_closure_alloc(sizeof(ffi_closure), (void **) &func);
    if (closure == NULL) {
        fprintf(stderr, "Failed to allocate closure\n");
        return 1;
    }
    auto xxxFN = []()-> void {
        std::cout << "runFN------------:" << std::endl;
    };
    auto aCallBack = [](ffi_cif *cif, void *ret, void **args, void *userdata)-> void {
        printf("Callback invoked with arg: %d\n", *(int *) args[0]);
        auto fn = reinterpret_cast<void(*)()>(userdata);
        fn();  // 调用传入的函数

        *(int *) ret = *(int *) args[0] * 2;
    };

    // 设置闭包
    if (ffi_prep_closure_loc(closure, &cif, aCallBack, (void*) &xxxFN, (void *) func) != FFI_OK) {
        fprintf(stderr, "Failed to prepare closure\n");
        ffi_closure_free(closure);
        return 1;
    }

    // 调用函数指针
    result = reinterpret_cast<int(*)(int)>(func)(x);
    printf("Result: %d\n", result);

    // 释放闭包
    ffi_closure_free(closure);

    return 0;
}
