#include "thread_pool.h"

namespace logger {

ThreadPool::ThreadInfo::~ThreadInfo() {
    if (t_Ptr != nullptr && t_Ptr->joinable()) {
        t_Ptr->join();
    }
}

void ThreadPool::Start() {
    if (!is_running_.load()) {
        is_running_.store(true);
        for (int i = 0; i < thread_count_.load(); i++) {
            AddThread();
        }
    }
}

void ThreadPool::Stop() {
    if (is_running_.load()) {
        is_running_.store(false);
        cv_.notify_all();
    }
    thread_vector_.clear();
}

void ThreadPool::AddThread() {
    auto func = [this](){
        while(true) {
            // 提取task并执行
            Task task;
            {
                std::unique_lock<std::mutex> lock(this->mtx_);
                // 休眠
                this->cv_.wait(lock, [this]() {
                    return !this->is_running_.load() || !this->task_queue_.empty();
                });
                
                if (!this->is_running_.load()) {
                    break;
                }

                task = this->task_queue_.front();
                this->task_queue_.pop();
            }
            task();
        }
    };
    ThreadInfoPtr ptr = std::make_shared<ThreadInfo>();
    ptr->t_Ptr = std::make_shared<std::thread>(std::move(func));
    thread_vector_.emplace_back(ptr);
}

}   // namespace logger