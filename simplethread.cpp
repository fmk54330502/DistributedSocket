#include "simplethread.h"

int simplePool::init()
{
    //初始化锁和条件变量，以及任务类
    for (int i = 0; i < NUM_OF_THREADS; i++)
    {
        pthread_mutex_init(cond_lock + i, NULL);
        pthread_cond_init(cond + i, NULL);
        tasks[i].args = NULL;
        tasks[i].condi = cond + i;
        tasks[i].function = NULL;
        tasks[i].isTaskActivated = false;
        tasks[i].lock = cond_lock + i;
    }

    //启动线程
    for (int i = 0; i < NUM_OF_THREADS; i++)
    {
        if (pthread_create(tid + i, NULL, taskFunc, (void *)(tasks + i)) == 0)
            cout << "*****Thread " << i << " successfully created..." << endl;
        else
        {
            return THREAD_FAILURE;
        }
    }
    return 0;
}

void *simplePool::taskFunc(void *_task)
{
    pthreadTask *task = (pthreadTask *)_task;
    while (1)
    {
        pthread_mutex_lock(task->lock);
        while (!task->isTaskActivated) //如果未激活则等待
        {
            pthread_cond_wait(task->condi, task->lock);
        }
        //cout << "task begin" << endl;
        pthread_mutex_unlock(task->lock);

        task->function(task->args);

        //初始化task，等待接收下一次指令
        task->isTaskActivated = false;
        task->function = NULL;
        task->args = NULL;
    }
    pthread_mutex_unlock(task->lock);
    return (NULL);
}

int simplePool::setThreadTask(int index, int funcIndex, void (*function)(void *), void *_args)
{
    if (index >= NUM_OF_THREADS)
    {
        return INDEX_OUT_OF_RANGE;
    }
    //将任务分配到args中
    fmkSocketApiArgs *args = (fmkSocketApiArgs *)_args;
    args->funcIndex = funcIndex;
    if (tasks[index].isTaskActivated == true)
        return TASK_BUSY;
    tasks[index].args = (void *)args;
    tasks[index].function = function;
    tasks[index].isTaskActivated = true;
    //这句话结束线程的挂起
    pthread_cond_signal(tasks[index].condi);
    return 0;
}

int simplePool::resetThread(int index)
{
    tasks[index].args = NULL;
    tasks[index].function = NULL;
    tasks[index].isTaskActivated = false;

    return 0;
}

int simplePool::waitThread(int index)
{
    while (tasks[index].isTaskActivated)
    {
        _sleep(100);
    }
    return 0;
}

void simplePool::fmkSockAPI(void *_args)
{
    fmkSocketApiArgs *args = (fmkSocketApiArgs *)_args;
    const char *sendData = const_cast<const char *>(args->sendData.c_str());
    switch (args->funcIndex)
    {
    case CONFIG:
    {
        args->mySock.config(args->port, args->ipaddr, CLIENT);
        break;
    }
    case CLIENT_INIT:
    {
        args->mySock.clientInit();
        break;
    }
    case CLIENT_SEND:
    {
        args->mySock.clientSend((char *)sendData);
        break;
    }
    case CLIENT_DESTORY:
    {
        args->mySock.clientDestroy();
        break;
    }
    case CLIENT_CONFIG_INIT_AND_SEND:
    {
        args->mySock.config(args->port, args->ipaddr, CLIENT);
        args->mySock.clientInit();
        args->mySock.clientSend((char *)sendData);
        break;
    }
    case CLIENT_RECV:
    {
        args->mySock.clientRecv();
    }
    }
}

int main()
{
    simplePool pool;
    fmkSocketApiArgs socketArgs[NUM_OF_THREADS];

    Reader reader;
    Value value;

    value["sendMessage"] = "Json send message.";
    string str = value.toStyledString();


    socketArgs[0].ipaddr = "192.168.1.61";
    socketArgs[0].port = 5099;
    socketArgs[0].sendData = str;

    // socketArgs[1].ipaddr = "192.168.3.235";
    // socketArgs[1].port = 5099;
    // socketArgs[1].sendData = "This is sent from thread 2.";

    pool.init();
    for (int i = 0; i < NUM_OF_THREADS; i++)
    {
        pool.setThreadTask(i, pool.CLIENT_CONFIG_INIT_AND_SEND, pool.fmkSockAPI, (void *)(socketArgs + i));
    }
    for (int i = 0; i < NUM_OF_THREADS; i++)
    {
        pool.waitThread(i);
    }
    _sleep(2000);
    cout << "Now begin to receive message..." << endl;
    for (int i = 0; i < NUM_OF_THREADS; i++)
    {
        pool.setThreadTask(i, pool.CLIENT_RECV, pool.fmkSockAPI, (void *)(socketArgs + i));
    }
    for (int i = 0; i < NUM_OF_THREADS; i++)
    {
        pool.waitThread(i);
    }
    system("Pause");
}