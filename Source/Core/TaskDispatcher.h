#ifndef IU_CORE_TASKDISPACHER_H
#define IU_CORE_TASKDISPACHER_H

#pragma once
#include <functional>
#include <boost/bind.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>

typedef std::function<void()> TaskRunnerTask;
/*
What if posted message will be never processed by receiver, e.g. it will be closed for some reason? 
In this case Func object will be leaked wasting your memory.
What if sender instance that sent the message gets destroyed before receiver processes the message? 

*/
class ITaskRunner {
public:
    virtual ~ITaskRunner(){}
    virtual void runInGuiThread(TaskRunnerTask&& task, bool async = false) = 0;
};

class Task {
public:
    virtual void run() = 0;
    virtual ~Task() {}
};

class CancellableTask: public Task {
public:
    virtual void cancel() = 0;
    virtual bool isCanceled() = 0;
    virtual bool isInProgress() = 0;
};

class TaskDispatcher {
public:

    TaskDispatcher(size_t numThreads): pool_(numThreads) {
        
    }

    ~TaskDispatcher() {
        pool_.stop();
        pool_.join();
    }

    template<typename Func> void post(Func&& func) {
        boost::asio::post(pool_, std::forward<Func>(func));
    }

    void postTask(std::shared_ptr<Task> task) {
        boost::asio::post(pool_, boost::bind(&Task::run, task));
    }

private:
    boost::asio::thread_pool pool_;
};

#endif