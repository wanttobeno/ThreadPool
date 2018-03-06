#pragma once

#include <set>
#include <list>
#include "Thread.h"

class CThreadPoolExecutor
{
public:
	CThreadPoolExecutor(void);
	virtual ~CThreadPoolExecutor(void);

	/*
	  初始化线程池，创建minThreads个线程
	*/
	bool Init(unsigned int minThreads,unsigned int maxThreads,unsigned int maxPendingTaskse);

	/*
	  添加任务
	  执行任务，若当期任务列表没有满，将此任务插入到任务列表，返回true
	  若当前任务列表满了，但当前线程数量小于最大线程数，将创建新线程执行此任务，返回true
	  若当前任务列表满了，但单前线程数量等于最大线程数，将丢失此任务，返回false
	  返回值：成功true，失败false
	*/
	bool Execute(Runnable* pRunnable);
	
	/*
	  终止线程池，先制止添加任务，然后等待任务列表为空，
	  设置最小线程数量为0，等待直到线程数来英为空，
	  清空垃圾堆中的任务
	*/
	void Terminate();

	/*
	  返回线程池中单前的线程数量
	*/

	unsigned int GetThreadPoolSize();

private:
	/*
	   获取任务列表中的任务，若任务为空，返回NULL
	*/
	Runnable* GetTask();
	static unsigned int WINAPI StaticThreadFunc(void * arg);
private:
	class CWork:public CThread
	{
	public:
		CWork(CThreadPoolExecutor* pThreadPool,Runnable* pFirstTask = NULL);
		virtual ~CWork();
		void Run();
	private:
		CThreadPoolExecutor * m_pThreadPool;
		Runnable* m_pFirstTask;
		volatile bool m_bRun;
	};

	typedef std::set<CWork *>ThreadPool;
	typedef std::list<Runnable *> Tasks;
	typedef ThreadPool::iterator ThreadPoolItr;

	ThreadPool m_ThreadPool;
	ThreadPool m_TrashThread;
	Tasks m_Tasks;

	CRITICAL_SECTION m_csTasksLock;
	CRITICAL_SECTION m_csThreadPoolLock;

	volatile bool m_bRun;
	volatile bool m_bEnableInsertTask;
	volatile unsigned int m_minThreads;
	// 最多开启线程数
	volatile unsigned int m_maxThreads;
	// 最大待处理任务数量
	volatile unsigned int m_maxPendingTasks;
};
