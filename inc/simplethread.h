#define HAVE_STRUCT_TIMESPEC //不加这一句会出现timespec重定义的错误,而且需要放到phread.h之前进行引用
#include "pthread.h"
#include "socket.h"
#define NUM_OF_THREADS 1
class fmkSocketApiArgs
{
public:
    fmkSocket mySock;
    string ipaddr;
    int port;
    string sendData;
    int funcIndex;
};

class pthreadTask
{
public:
    void (*function)(void *args); //这句话定义了一个函数指针(*function)，该函数的类型是void，参数是void*args
    //void *function(void *args);//注意这种写法和上面一种写法含义是不一样的。这句话定义了一个函数function，它的类型是void*，参数是void*args.
    void *args;
    pthread_mutex_t *lock;
    pthread_cond_t *condi;
    bool isTaskActivated;

    pthreadTask()
    {
        isTaskActivated = false;
    }
};

class simplePool
{
public:
    //phtread的锁和条件设计的本意是避免线程间的冲突，用于构造高效的线程池
    //但我现在的需求中，不需要线程池
    //但是需要线程挂起，满足条件后才行动
    //线程之间的行为无耦合
    //所以每个线程都给一把锁和一个条件变量
    pthread_mutex_t cond_lock[NUM_OF_THREADS];
    pthread_cond_t cond[NUM_OF_THREADS];

    pthread_t tid[NUM_OF_THREADS];
    pthreadTask tasks[NUM_OF_THREADS]; //每个线程对应一个任务，任务是明确且一对一的

    enum poolError
    {
        THREAD_FAILURE = -1,
        TASK_BUSY = -2,
        INDEX_OUT_OF_RANGE = -3

    };

    int setThreadTask(int index, int funcIndex, void (*function)(void *), void *args);
    int resetThread(int index);
    int init();
    static void *taskFunc(void *task);
    //等待线程执行完任务延时
    int waitThread(int index);

    int funcIndex; //将函数进行编号，根据这个编号调用不同的函数
    enum funcIndexList
    {
        CONFIG = 0,
        SERVER_INIT = 1,
        SERVER_RECV = 2,
        SERVER_SEND = 3,
        CLIENT_INIT = 4,
        CLIENT_RECV = 5,
        CLIENT_SEND = 6,
        CLIENT_DESTORY = 7,

        //复合操作
        CLIENT_CONFIG_INIT_AND_SEND = 8
    };
    static void fmkSockAPI(void *_args);
};
