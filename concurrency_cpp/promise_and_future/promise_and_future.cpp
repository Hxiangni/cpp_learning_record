// promise_and_future.cpp: 定义应用程序的入口点。
//
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN //WIN32_LEAN_AND_MEAN 含义：精简 windows.h 头文件。
#include <windows.h>//windows.h 是 Windows 超级巨大的总头文件，里面会包含巨量很少用的组件（MFC、COM、网络、GDI 等）。
#endif

#include "promise_and_future.h"  
#include "async_and_packaged_task.h"
int main()
{
#ifdef _WIN32
    // 设置控制台输出代码页为UTF‑8
    SetConsoleOutputCP(CP_UTF8);
    // 如果需要控制台输入也支持UTF‑8，再加这一句
    // SetConsoleCP(CP_UTF8);
#endif
     
    //test_cmp();
    test_packaged_task();
	return 0;
}
