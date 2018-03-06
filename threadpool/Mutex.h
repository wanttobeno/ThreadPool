/************************************************************************/
/* 
作用：保护共享资源。

1．互斥量是内核对象，它与关键段都有“线程所有权”所以不能用于线程的同步。

2．互斥量能够用于多个进程之间线程互斥问题，并且能完美的解决某进程意外终止所造成的“遗弃”问题。
*/
/************************************************************************/
#pragma once

class Mutex
{	// 构造函数，拷贝构造函数
	Mutex(Mutex const &);
	// this 已经调用构造函数，赋值运算。
	//未重载这个函数，默认浅拷贝，即地址赋值。
	//重载函数做深拷贝处理
	Mutex &operator=(Mutex const&);
public:
	Mutex(void);
	// 析构会销毁系统互斥量
	virtual ~Mutex(void);

	// 进入临界区
	bool enter();

	// 离开临界区
	bool leave();
private:
	struct MutexInternal;
	MutexInternal *m_pInternal;
};
