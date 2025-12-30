#include "scheduler/scheduler.h"

#include "internal_log.h"

namespace logger {

void Scheduler::NewTaskExecutor(ExecutorTag tag, int thread_count) {
    std::unique_lock<std::mutex> lock(mtx_);
    if (executor_dirc_.find(tag) != executor_dirc_.end()) {
        return;
    }
    executor_dirc_[tag] = std::make_unique<Executor>(thread_count);
    executor_dirc_[tag]->Start();
}

void Scheduler::CancelRepeatedTask(uint64_t repeated_id) {
    timer_.CancelRepeatedTask(repeated_id);
}

Executor* Scheduler::GetOrAddExecutor(ExecutorTag tag) {
    std::unique_lock<std::mutex> lock(mtx_);
    if (executor_dirc_.find(tag) == executor_dirc_.end()) {
        lock.unlock();
        NewTaskExecutor(tag, 1);
    }
    return executor_dirc_[tag].get();
}

// --------- Scheduler Timer Function ---------

void Scheduler::Timer::Start() {
    if (!is_running_.load()) {
        is_running_.store(true);
        timer_executor_->Start();
        timer_executor_->AddTask(&Scheduler::Timer::Run_, this);
    }
}

void Scheduler::Timer::Stop() {
    if (is_running_.load()) {
        is_running_.store(false);
        timer_executor_->Stop();
        cv_.notify_all();
    }
}

void Scheduler::Timer::Run_() {
    while(true) {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this]() {
            return !timer_queue_.empty() || !is_running_.load();
        });
        if (!is_running_.load()) {
            break;
        }
        auto s = timer_queue_.top();
        auto diff = s.time_point - std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::microseconds>(diff).count() > 0) {
            cv_.wait_for(lock, diff);
        } else {
            timer_queue_.pop();
            lock.unlock();
            try {
                s.task();
            }
            catch(const std::exception& e) {
                ERROR("task run error[{}] \n", e.what());
            }
            
        }
    }
}

void Scheduler::Timer::PostDelayedTask(Task&& task, const std::chrono::microseconds& delta) {
    InternalS s;
    s.task = std::move(task);
    s.time_point = std::chrono::high_resolution_clock::now() + delta;
    {
        std::unique_lock<std::mutex> lock(mtx_);
        timer_queue_.push(std::move(s));
        cv_.notify_all();
    }
}

uint64_t Scheduler::Timer::PostRepeatedTask(Task&& task, const std::chrono::microseconds& delta, uint64_t repeat_num) {
    RepeatedTaskId id = GetNextRepeatedTaskId_();
    {
        std::unique_lock<std::mutex> lock(mtx_);
        repeated_set_.insert(id);
    }
    PostRepeatedTask_(std::forward<Task>(task), delta, id, repeat_num);
    return id;
}

void Scheduler::Timer::PostRepeatedTask_(Task&& task, const std::chrono::microseconds& delta, RepeatedTaskId id, uint64_t repeat_num) {
    // repeat_num初始值小于-1无限循环
    if (repeat_num == 0) {
        CancelRepeatedTask(id);
        return;
    }
    {
        std::unique_lock<std::mutex> lock(mtx_);
        if (repeated_set_.find(id) == repeated_set_.end()) {
            return;
        }
    }
    task();

    Task timer_task = std::bind(&Scheduler::Timer::PostRepeatedTaskForward_, this, std::move(task), delta, id, repeat_num - 1);

    InternalS s;
    s.task = std::move(timer_task);
    s.time_point = std::chrono::high_resolution_clock::now() + delta;
    s.repeated_id = id;
    {
        std::unique_lock<std::mutex> lock(mtx_);
        timer_queue_.push(std::move(s));
        cv_.notify_all();
    }

}

// 转发函数，帮助实现递归
void Scheduler::Timer::PostRepeatedTaskForward_(Task& task, const std::chrono::microseconds& delta, RepeatedTaskId id, uint64_t repeat_num) {
    PostRepeatedTask_(std::move(task), delta, id, repeat_num);
}


void Scheduler::Timer::CancelRepeatedTask(RepeatedTaskId id) {
    std::unique_lock<std::mutex> lock(mtx_);
    repeated_set_.erase(id);
}



}   // namespace logger