// promise_and_future.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。

#pragma once

#include <iostream>
#include <future>
#include <thread>
//积分求pi
void compute_pi(const long num_steps, std::promise<double>& promise) {
	double step = 1.0 / num_steps;//小矩形的宽度
	double sum = 0.0;
	for (long  i = 0; i < num_steps; i++) {
		double x = (i + 0.5) * step;
		double y = 4.0 / (1.0 + x * x);
		sum =sum + y;
	}
	promise.set_value(sum * step);
}

void display(std::future<double>& receiver) {
	/**
	 * future::get()
	 * 当promise还没set_value是阻塞挂起不消耗cpu
	 * 当promise已经set_value 立即返回
	 * - - -
	 * 线程 2 在 get() 处进入等待队列，操作系统把它从 CPU 调度里拿掉。
	 * 等线程 1 调用 set_value 时，OS 通知 future，future 
	 * 再把线程 2 放回就绪队列。整个过程 T2 不占任何 CPU 周期。
	 * - - -
	 * get() 只能用一次,get() 会"取走"共享状态（消费 future）
	 * 如果想"看看好了没"但不取值，用 
	 * wait() //无限等
	 * wait_for(1s) //等1秒
	 * wait_until()
	 */
	double pi = receiver.get();
	std::cout << "返回的pi=" << pi << std::endl;
}

void test_compute_pi(){
	const int step = 1000000;//步数
	std::promise<double> promise;
	/* promise.get_future() 做的事：
	 * 1.在 promise 内部创建一块共享状态（如果还没有）
	 * 2.返回一个 future，指向同一块共享状态
	 * 3.从此 promise 和 future 就"接头"成功
	 */
	auto receiver = promise.get_future();
	/**
	 *std::thread 会把所有参数按值拷贝/移动到线程内部的存储里
	 *std::promise是move-only不能拷贝的，
	 * */
	std::thread th1(compute_pi, step, std::ref(promise));
	std::thread th2(display, std::ref(receiver));
	th1.join();
	th2.join();
}


//更安全的改进版,防止引用进入别的线程后提前被销毁而造成出错误
//上面函数参数也改为右值引用
//void test_sage_compute_pi(){
//	const int step = 1000000;//步数
//	std::thread th1;
//	std::thread th2;
//	{
//		std::promise<double> promise;
//		auto receiver = promise.get_future();
//		th1=std::thread(compute_pi, step, std::move(promise));
//		th2=std::thread(display,std::move(receiver));
//	}
//	th1.join();
//	th2.join();
//}
// TODO: 在此处引用程序需要的其他标头。
