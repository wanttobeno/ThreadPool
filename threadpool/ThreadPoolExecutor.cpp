#include "ThreadPoolExecutor.h"


/////////////////////////////////////////////
// 控制日志输出

//#define PRING_LOG

#ifdef PRING_LOG
	#define ZY_PRINT printf
#else
void Print(...)
{
}
	#define ZY_PRINT Print
#endif // _DEBUG
/////////////////////////////////////////////

CThreadPoolExecutor::CWork::CWork(CThreadPoolExecutor* pThreadPool,Runnable* pFirstTask /* = NULL */):
m_pThreadPool(pThreadPool),
m_pFirstTask(pFirstTask),
m_bRun(true)
{

}

CThreadPoolExecutor::CWork::~CWork()
{

}

/*
执行任务的工作线程。
当前没有任务时，如果当前线程数量大于最小线程数量，减少线程，
否则，执行清理程序，将线程类给释放掉
*/

void CThreadPoolExecutor::CWork::Run()
{
	Runnable * pTask = NULL;
	while(m_bRun)
	{
		if (NULL == m_pFirstTask)
		{
			// 获取任务
			pTask = m_pThreadPool->GetTask();
			ZY_PRINT("CThreadPoolExecutor CWorker::Run() pTask is 0x%x\n",pTask);
		}
		else
		{
			pTask = m_pFirstTask;
			m_pFirstTask = NULL;
			ZY_PRINT("CThreadPoolExecutor CWork::Run() m_pFirstTask  is not nullpTask is 0x%x\n",pTask);
		}

		if (NULL == pTask)
		{
			// 线程池无任务，空闲状态，执行清理工作
			ZY_PRINT("CThreadPoolExecutor CWork::Run() pTask is NUll \n");
			EnterCriticalSection(&(m_pThreadPool->m_csThreadPoolLock));
			if (m_pThreadPool->GetThreadPoolSize() > m_pThreadPool->m_minThreads)
			{
				// 空闲状态，池线程移除多余的线程
				ZY_PRINT("CThreadPoolExecutor CWork::Run() GetThreadPoolSize >>>>>>>\n");
				ThreadPoolItr it = m_pThreadPool->m_ThreadPool.find(this);
				if (it != m_pThreadPool->m_ThreadPool.end())
				{
					m_pThreadPool->m_ThreadPool.erase(it);
					m_pThreadPool->m_TrashThread.insert(this);
				}
				m_bRun = false;
			}
			else
			{
				// 空闲状态，删除多余的线程池线程内存
				ZY_PRINT("CThreadPoolExecutor CWorker::Run() delete m_TrashThread ------- \n");
				ThreadPoolItr it = m_pThreadPool->m_TrashThread.begin();
				while(it!=m_pThreadPool->m_TrashThread.end())
				{
					(*it)->Join();
					delete (*it);
					m_pThreadPool->m_TrashThread.erase(it);
					it = m_pThreadPool->m_TrashThread.begin();
				}
			}
			LeaveCriticalSection(&(m_pThreadPool->m_csThreadPoolLock));
			continue;
		}
		else
		{
			// 获取到任务，直接执行
			pTask->Run();
			delete pTask;
			pTask = NULL;
		}
	}
}


CThreadPoolExecutor::CThreadPoolExecutor(void):
m_bRun(false),
m_bEnableInsertTask(false)
{
	InitializeCriticalSection(&m_csTasksLock);
	InitializeCriticalSection(&m_csThreadPoolLock);
}

CThreadPoolExecutor::~CThreadPoolExecutor(void)
{
	Terminate();
	DeleteCriticalSection(&m_csTasksLock);
	DeleteCriticalSection(&m_csThreadPoolLock);
}

bool CThreadPoolExecutor::Init(unsigned int minThreads,unsigned int  maxThreads,unsigned int maxPendingTasks)
{
	if (minThreads == 0)
		return false;
	if (maxThreads < minThreads)
		return false;
	m_minThreads  = minThreads;
	m_maxThreads = maxThreads;
	m_maxPendingTasks = maxPendingTasks;
	unsigned int i = m_ThreadPool.size();
	for (;i<minThreads;i++)
	{
		// 创建线程
		CWork * pWorker = new CWork(this);
		if (NULL == pWorker)
			return false;
		EnterCriticalSection(&m_csThreadPoolLock);
		m_ThreadPool.insert(pWorker);
		LeaveCriticalSection(&m_csThreadPoolLock);
		pWorker->Start();
	}
	m_bRun = true;
	m_bEnableInsertTask = true;
	return true;
}

bool CThreadPoolExecutor::Execute(Runnable* pRunnable)
{
	if (!m_bEnableInsertTask)
		return false;
	if (NULL == pRunnable)
	{
		ZY_PRINT("CThreadPoolExecutor pRunnable is NULL\n");
		return false;
	}
	if (m_Tasks.size() >= m_maxPendingTasks)  // 任务列表满了没有
	{
		if (m_ThreadPool.size() < m_maxThreads)  // 任务列表满了，判断线程池线程到达上限了没
		{
			CWork* pWorker  = new CWork(this,pRunnable);
			if (NULL == pWorker)
			{
				ZY_PRINT("CThreadPoolExecutor new CWorke is NULL \n");
				return false;
			}
			EnterCriticalSection(&m_csThreadPoolLock);
			m_ThreadPool.insert(pWorker);
			LeaveCriticalSection(&m_csThreadPoolLock);
			ZY_PRINT("CThreadPoolExecutor CWork Start is 0x%d\n",pWorker);
			pWorker->Start();
		} 
		else	// 线程池线程到达上限，放弃处理新任务
		{
			ZY_PRINT("CThreadPoolExecutor m_ThreadPool > m_maxThreads\n");
			return false;
		}
	}
	else	// 将任务添加到任务列表中去
	{
		EnterCriticalSection(&m_csTasksLock);
		m_Tasks.push_back(pRunnable);
		LeaveCriticalSection(&m_csTasksLock);
	}
	return true;
}

Runnable* CThreadPoolExecutor::GetTask()
{
	Runnable* pTask =NULL;
	EnterCriticalSection(&m_csTasksLock);
	if (!m_Tasks.empty())
	{
		pTask = m_Tasks.front();
		m_Tasks.pop_front();
	}
	else
	{
		ZY_PRINT("CThreadPoolexecutor Runnable m_Task is NULL \n");
	}
	LeaveCriticalSection(&m_csTasksLock);
	return pTask;
}

void CThreadPoolExecutor::Terminate(void)
{
	ZY_PRINT("CThreadPoolExetor Terminate\n");
	m_bEnableInsertTask = false;
	while(m_Tasks.size() > 0)
	{
		Sleep(1);
	}
	m_bRun = false;
	m_minThreads =0;
	m_maxThreads =0;
	m_maxPendingTasks =0;
	while(m_ThreadPool.size() >0)
	{
		Sleep(1);
	}
	EnterCriticalSection(&m_csThreadPoolLock);
	ThreadPoolItr it = m_TrashThread.begin();
	while(it != m_TrashThread.end())
	{
		(*it)->Join();
		delete (*it);
		m_TrashThread.erase(it);
		it =m_TrashThread.begin();
	}
	LeaveCriticalSection(&m_csThreadPoolLock);
}



unsigned int CThreadPoolExecutor::GetThreadPoolSize()
{
	return m_ThreadPool.size();
}











