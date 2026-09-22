#pragma once
#include <iostream>
#include <vector>
#include <future>
#include <algorithm>
#include <numeric>     // ← 新增：accumulate
#include <random>
#include <thread>
#include <syncstream>
#include <map>
class Random {
public:
    Random(int min, int max)
        /**
        * std::random_device：标准库随机设备类
        * std::random_device{}直接构造一个临时的random_device实例
        * ()：调用这个临时对象的函数调用运算符 operator()，返回一个 unsigned int随机值
        * 这个 unsigned int 作种子初始化 engine_
        * 因为 mt19937 是"确定性"引擎：给它同一个种子，它必然吐出同一串数所以要在这里伪随机一一下
        */
        :engine_(std::random_device{}())
        , dist_(min, max)
    {}
    int operator()()
    {
        return dist_(engine_);
    }

private:
    /**
     * 因为 mt19937 是"确定性"引擎：给它同一个种子，它必然吐出同一串数
     * engine_(42);就是用42做种子,参数一样输出来的也一样
     */
    std::mt19937 engine_;
    //创建指定的分布区域
    std::uniform_int_distribution<int> dist_;

};


float Compute(std::vector<float> &v)
{
    //所在线程休眠2秒
    std::this_thread::sleep_for(std::chrono::seconds(2)); 
    //累加vector所有元素，0.0f是累加初始值
    auto r = std::accumulate(v.begin(), v.end(), 0.0f); 
    /**
     * std::osyncstream(std::cout) 创建一个临时对象，绑定 std::cout
     * 后面所有 << 内容先写进它自己的内部缓冲区，不直接落到 cout
     * 整条语句结束 → 临时对象析构 → 缓冲区一次性、原子地刷进 cout
     * 内部有锁保护，其他线程的 osyncstream 插不进来
     */
    std::osyncstream(std::cout) << "执行任务的线程Id:" << std::this_thread::get_id() << std::endl;
    return r;
}

//主副线程都设置了2秒的延迟
//最后测试发现运行时间也就两秒多一点
//可以推测肯定有两个线程在同时进行
void test_Compute()
{
    const int RN_MAX = 10000;
    Random rgen(0, RN_MAX);
    std::vector<float> numbers(100, 0.0f);
    // 填充100个0~1之间的随机浮点数
    std::generate(numbers.begin(), numbers.end(), [&]
        { return float(rgen()) / RN_MAX; });

    auto begin = std::chrono::steady_clock::now(); // 记录开始时间
    std::cout << "创建任务的线程Id:" << std::this_thread::get_id() << std::endl;

    // 重点！创建异步任务，启动独立线程运行Compute
    std::future<float> result;
    result = std::async(std::launch::async, Compute, std::ref(numbers));

    std::this_thread::sleep_for(std::chrono::seconds(2)); // 主线程也休眠2秒

    auto r = result.get(); // 阻塞！等待子线程执行完毕，拿到返回值
    std::cout << "result = " << r << std::endl;

    auto end = std::chrono::steady_clock::now();
    // 计算总共耗时
    std::cout << "任务共耗时 = "
        << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
        << "[ms]" << std::endl;
};


/* 执行完该对比可以发现
 * async是立刻开启子线程来执行，立即开跑
 * deferred不创建线程，推迟到 get()/wait() 时执行
 * 当deferred时if判断任务根本还没有执行呢
 */
void test_cmp() {
    const int RN_MAX = 10000;
    Random rgen(0, RN_MAX);
    std::vector<float> numbers(100, 0.0f);
    // 填充100个0~1之间的随机浮点数
    std::generate(numbers.begin(), numbers.end(), [&]
        { return float(rgen()) / RN_MAX; });

    std::map<std::string, std::launch> m = {
        //把整数 0 强制转换成枚举类型std::launch
        //进入源码可以看到两个策略的枚举类型
        //async    = 0x1,
        //deferred = 0x2
        //代表没有传入任何 launch 标记，对应 std::async 的默认调用形式（不带策略参数）。
        //默认策略的话就只能看编译器是怎么安排的了我用MSVC和异步效果一样
        {"默认",(std::launch)0},
        {"异步",std::launch::async},
        {"推迟",std::launch::deferred },
    };

    for (auto& p : m) {
        std::cout << "\n*********************\n策略:" << p.first << std::endl;
        // 记录当前时间点
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        // 打印【创建async任务】所在的线程ID，也就是主线程
        std::cout << "创建任务的线程Id:" << std::this_thread::get_id() << std::endl;

        std::future<float> result;
        /*
        * p.second的类型是std::launch，这是位掩码枚举 (enum class)。
        */
        if (int(p.second) != 0)
            //调用掩码的策略
            result = std::async(p.second, Compute, std::ref(numbers));
        else
            //调用默认策略
            result = std::async(Compute, std::ref(numbers));
       
        /** 
         * result是std::future对象
         * future::wait_for(时长)
         * wait_for：等待任务完成，可以设置最长等待时间；会返回一个枚举std::future_status。
         * std::chrono::milliseconds(0),查询一下立刻返回，等待0秒无等待
         * 下面是返回值
         *  1.std::future_status::ready：任务已经执行完毕，结果就绪，可以直接get ()取值
         *  2.std::future_status::timeout：任务还在跑，还没完成（超时，因为我们只给 0ms 等待）
         *  3.std::future_status::deferred：这个 future 是 deferred 推迟模式，任务根本还没有开始执行！
         */
        if (result.wait_for(std::chrono::milliseconds(0)) == std::future_status::deferred) {
            std::cout << "任务推迟" << std::endl;
        }

        /**
         * 调用get()会阻塞等待任务完成，并且拿到返回值
         */
        auto r = result.get();

        std::cout << "result = " << r << std::endl;
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << "任务共耗时 = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
    }
}