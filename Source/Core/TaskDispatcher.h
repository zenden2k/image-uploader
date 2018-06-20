#ifndef IU_CORE_TASKDISPACHER_H
#define IU_CORE_TASKDISPACHER_H

#pragma once
#include <functional>

typedef std::function<void()> TaskDispatcherTask;
/*
What if posted message will be never processed by receiver, e.g. it will be closed for some reason? 
In this case Func object will be leaked wasting your memory.
What if sender instance that sent the message gets destroyed before receiver processes the message? 

*/
class ITaskDispatcher {
public:
    virtual ~ITaskDispatcher(){}
    virtual void runInGuiThread(TaskDispatcherTask&& task, bool async = false) = 0;
};

#endif