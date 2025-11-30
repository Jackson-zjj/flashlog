#pragma once

#include <functional>

namespace logger {

///@brief 离开作用域后执行指定函数或代码
class ScopeGuard {
public:
    ScopeGuard() = default;
    ~ScopeGuard();

    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    ScopeGuard(ScopeGuard&&) = delete;
    ScopeGuard& operator=(ScopeGuard&&) = delete;
    
    template<typename F, typename... Args>
    ScopeGuard(F&& func, Args&&... args) {
        func_ = std::bind(std::forward<F>(func), std::forward<Args>(args)...);
    }

private:
    std::function<void()> func_;
    
};  // class ScopeGuard

ScopeGuard::~ScopeGuard() {
    if (func_) {
        func_();
    }
}

// 利用宏定义+宏拼接+类的转换构造
#undef LOG_DELAY
///@brief 离开作用域后执行
#define LOG_DELAY logger::ScopeGuard guard##__LINE__ = [&]()

}   // namespace logger