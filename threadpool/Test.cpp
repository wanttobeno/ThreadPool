
#define  WIN32_LEAN_AND_MEAN

#include "Mutex.h"
#include "Semaphore.h"
#include "Thread.h"
#include "ThreadPoolExecutor.h"


// 测试线程数
#define  TEST_TREAD_COUNT 20
// 每个测试线程数添加的任务数
#define  TASK_COUNT 100

static int g_totalcount = 0;
static int g_seq = 0;

// 保存g_seq
CRITICAL_SECTION  g_csThreadCode;

// 保护共享资源
Mutex g_runmutex; 

// 这里的作用：退出循环，相当于以个全局布尔变量
Semaphore g_semFinishFlag;


class R :public Runnable
{
public:
	~R()
	{

	}
	void Run()
	{
		EnterCriticalSection(&g_csThreadCode);
		g_seq ++;
		Sleep(0);
		printf("Hello World is: %d\n",g_seq);
		LeaveCriticalSection(&g_csThreadCode);
	}
protected:
private:
};

DWORD __stdcall WapperFun1(void* arg)
{
	CThreadPoolExecutor* poolmanager = (CThreadPoolExecutor*)arg;
	if (poolmanager == NULL)
		return 0;
	for (int i=0;i<TASK_COUNT;++i)
	{
		R* r= new R;
		g_runmutex.enter();
		while(!poolmanager->Execute(r)) 
		{
			Sleep(100); // 任务添加失败，给线程运行时间，稍后继续添加
		}
		g_totalcount++;
		g_runmutex.leave();
	}

	if (g_totalcount >= TEST_TREAD_COUNT*TASK_COUNT)
	{
		g_semFinishFlag.post();
	}
	return 0;
}

int main()
{
	// 创建线程池，并初始化10个工作线程
	CThreadPoolExecutor* pExecutor = new CThreadPoolExecutor();
	pExecutor->Init(10,64,100);

	InitializeCriticalSection(&g_csThreadCode);

	// 添加20个线程任务
	HANDLE m_threadId[TEST_TREAD_COUNT] = {0};
	for (int cow =0;cow < TEST_TREAD_COUNT;cow ++)
	{
		m_threadId[cow] = CreateThread(NULL,0,WapperFun1,pExecutor,0,NULL);
	}

	// 每100毫秒判断下任务是否执行完毕了
	while(1)
	{
		if (g_semFinishFlag.pend(100) <0)
		{
			Sleep(10);
			continue; 
		}
		else
			break;
	}
	pExecutor->Terminate();
	delete pExecutor;
	DeleteCriticalSection(&g_csThreadCode);
	getchar();
	return 0;
}
