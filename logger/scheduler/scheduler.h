#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <chrono>

#include "scheduler/thread_pool.h"

namespace logger {

using Executor = logger::ThreadPool;
using ExecutorPtr = std::unique_ptr<Executor>;
using ExecutorTag = std::string_view;

/// @brief 任务调度器（管理任务执行器）
class Scheduler {
public:
    Scheduler(const Scheduler& other) = delete;

    Scheduler& operator=(const Scheduler& other) = delete;

    ~Scheduler() = default;

    // 单例
    static Scheduler& GetInstance() {
        static Scheduler inst;
        return inst;
    }

    /// @brief 创建执行器
    void NewTaskExecutor(ExecutorTag tag, int thread_count = 1);

    /// @brief 提交任务
    template <typename F, typename... Args>
    inline void PostTask(ExecutorTag tag, F&& func, Args&&... args);

    /// @brief 提交任务并获取结果
    template <typename F, typename... Args>
    inline auto PostTaskAndGetResult(ExecutorTag tag, F&& func, Args&&... args)
        -> std::shared_future<std::invoke_result_t<F, Args...>>;

    /// @brief 提交延迟任务
    template <typename Rep, typename Period, typename F, typename... Args>
    inline void PostDelayTask(ExecutorTag tag, const std::chrono::duration<Rep, Period>& delta, F&& func, Args&&... args);

    /// @brief 提交重复执行的任务
    template <typename Rep, typename Period, typename F, typename... Args>
    inline void PostRepeatedTask(ExecutorTag tag, const std::chrono::duration<Rep, Period>& delta, uint64_t repeat_num, F&& func, Args&&... args);

    /// @brief 取消任务
    inline void CancelRepeatedTask(uint64_t repeated_id);

private:
    class Timer {
    public:
        using Task = std::function<void()>;
        using RepeatedTaskId = uint64_t;
        
        Timer() : is_running_(false), timer_executor_(std::make_unique<logger::ThreadPool>(1)), repeated_task_id_(0) {}

        Timer(const Timer& other) = delete;

        Timer& operator=(const Timer& other) = delete;

        ~Timer() {Stop();}

        void Start();

        void Stop();

        void PostDelayedTask(Task&& task, const std::chrono::microseconds& delta);

        RepeatedTaskId PostRepeatedTask(Task&& task, const std::chrono::microseconds& delta, uint64_t repeat_num);

        void CancelRepeatedTask(RepeatedTaskId repeated_id);
        
    private:
        void Run_();

        void PostRepeatedTask_(Task&& task, const std::chrono::microseconds& delta, RepeatedTaskId repeated_id, uint64_t repeat_num);

        void PostRepeatedTaskForward_(Task& task, const std::chrono::microseconds& delta, RepeatedTaskId repeated_id, uint64_t repeat_num);

        RepeatedTaskId GetNextRepeatedTaskId_() {return repeated_task_id_++;}

        // Timer private typedef
        struct InternalS {
            RepeatedTaskId repeated_id;
            Task task;
            std::chrono::time_point<std::chrono::high_resolution_clock> time_point;

            InternalS() = default;
            InternalS(const InternalS& other) = default;
            InternalS& operator=(const InternalS& other) = default;
            InternalS(const InternalS&& other) : repeated_id(other.repeated_id), task(std::move(other.task)), time_point(other.time_point) {}
            bool operator<(const InternalS& other) const {return time_point > other.time_point;}
        };

        // Timer private member
        std::mutex mtx_;
        std::condition_variable cv_;
        std::atomic<bool> is_running_;
        ExecutorPtr timer_executor_;
        std::priority_queue<InternalS> timer_queue_;
        std::atomic<RepeatedTaskId> repeated_task_id_;
        std::unordered_set<RepeatedTaskId> repeated_set_;     // 正在循环执行的任务
    };

    // Scheduler private
    Scheduler() = default;
    Executor* GetOrAddExecutor(ExecutorTag);

    std::mutex mtx_;
    std::unordered_map<ExecutorTag, ExecutorPtr> executor_dirc_;
    Timer timer_;
};

template <typename F, typename... Args>
void Scheduler::PostTask(ExecutorTag tag, F&& func, Args&&... args) {
    Executor* executor = GetOrAddExecutor(tag);
    executor->AddTask(std::forward<F>(func), std::forward<Args>(args)...);
}

template <typename F, typename... Args>
auto Scheduler::PostTaskAndGetResult(ExecutorTag tag, F&& func, Args&&... args) -> std::shared_future<std::invoke_result_t<F, Args...>> {
    Executor* executor = GetOrAddExecutor(tag);
    return executor->AddReturnTask(std::forward<F>(func), std::forward<Args>(args)...);
}

template <typename Rep, typename Period, typename F, typename... Args>
void Scheduler::PostDelayTask(ExecutorTag tag, const std::chrono::duration<Rep, Period>& delta, F&& func, Args&&... args) {
    timer_.Start();
    auto task = std::bind(&Scheduler::PostTask, this, std::move(tag), std::forward<F>(func), std::forward<Args>(args)...);
    timer_.PostDelayedTask(std::move(task), std::chrono::duration_cast<std::chrono::microseconds>(delta));
}

template <typename Rep, typename Period, typename F, typename... Args>
void Scheduler::PostRepeatedTask(ExecutorTag tag, const std::chrono::duration<Rep, Period>& delta, uint64_t repeat_num, F&& func, Args&&... args) {
    timer_.Start();
    auto task = std::bind(&Scheduler::PostTask, this, std::move(tag), std::forward<F>(func), std::forward<Args>(args)...);
    timer_.PostRepeatedTask(std::move(task), std::chrono::duration_cast<std::chrono::microseconds>(delta), repeat_num);
}

}   // namespace logger

#define SCHEDULER logger::Scheduler::GetInstance()
