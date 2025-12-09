#include <thread>
#include <memory>
#include <functional>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <future>

namespace logger {

/// @brief 线程池
class ThreadPool {
public:
    // Construct
    explicit ThreadPool(int count) : thread_count_(count), is_running_(false) {}

    ThreadPool(const ThreadPool& other) = delete;

    ThreadPool& operator=(const ThreadPool& other) = delete;

    ~ThreadPool() {Stop();}

    // public Interface
    void Start();

    void Stop();

    template <typename F, typename... Args>
    void addTask(F&& func, Args&&... args);
    
    template <typename F, typename... Args>
    std::shared_future<std::invoke_result_t<F, Args...>> addReturnTask(F&& func, Args&&... args);

private:
    // private interface
    void addThread();

    // private typedef
    struct ThreadInfo;
    using Task = std::function<void()>;
    using ThreadPtr = std::shared_ptr<std::thread>;
    using ThreadInfoPtr = std::shared_ptr<ThreadInfo>;

    struct ThreadInfo {
        ThreadInfo() = default;
        ~ThreadInfo();

        ThreadPtr t_Ptr;
    };

    // private member
    std::atomic<int> thread_count_;
    std::atomic<bool> is_running_;
    std::mutex mtx_;  
    std::condition_variable cv_;
    std::queue<Task> task_queue_;
    std::vector<ThreadInfoPtr> thread_vector_;

};// class ThreadPool

template <typename F, typename... Args>
void ThreadPool::addTask(F&& func, Args&&... args) {
    Task task = std::bind(std::forward<F>(func), std::forward<Args>(args)...);
    {
        std::lock_guard<std::mutex> lock(this->mtx_);
        task_queue_.emplace(task);
    }
    cv_.notify_all();
}

template <typename F, typename... Args>
auto ThreadPool::addReturnTask(F&& func, Args&&... args) -> std::shared_future<std::invoke_result_t<F, Args...>> {
   using r_type = std::invoke_result_t<F, Args...>;
   auto task = std::bind(std::forward<F>(func), std::forward<Args>(args)...);
   auto r_task = std::make_shared<std::packaged_task<r_type()>>(task);
   auto result = r_task->get_future();
   auto shared_result = result.share();
   {
        std::lock_guard<std::mutex> lock(this->mtx_);
        task_queue_.emplace([r_task]() {
            (*r_task)();
        });
   }
   cv_.notify_all();
   return shared_result;
}

}// namespace logger