//
//	Copyright (c)1998-2014,Xunmei Technology
//	All Rights Reserved
//
//Description:
// Semaphore.h  2013-02-05 jiaogwei

/************************************************************************/
/* 
	http://processors.wiki.ti.com/images/4/4f/Semaphores.pdf
*/
/************************************************************************/

#pragma once

// 多平台信号量类
class Semaphore
{
	Semaphore(Semaphore const &);
	Semaphore& operator=(Semaphore const &);
public:
	// 构造函数，会创建系统信号量
	// 这里创建LONG_MAX个最大并发数量
	explicit Semaphore(int initialCount =0);
	// 析构函数，会销毁系统互斥量
	~Semaphore(void);

	// Pending on a semaphore decrements count
	// 要求锁定一个semaphore
	// 减少信号量计数,如果已经减少到0，会阻塞调用的线程
	// 返回值： >=0 成功， <0 出错
	int pend();

	// Pending on a semaphore decrements count
	// 减少信号量计数，如果是从0累加，会唤醒其等待队列的第一个线程
	// 参数：超时时间，单位毫秒
	// 返回值： >=0 成功， <0 出错或超时
	int pend(unsigned long timeout);

	// Posting a semaphore increments count
	// 增加信号量计数，如果是从0累加，会唤醒其等待队列的第一个线程
	// 返回值：当前信号量计数
	int post();
private:
	struct SemaphoreInternal;
	SemaphoreInternal* m_internal;
};
