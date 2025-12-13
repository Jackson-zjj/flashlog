#include <memory>

namespace logger {

class VariadicLogHandle;

class LogFactory {
public:
    LogFactory(const LogFactory& other) = delete;

    LogFactory& operator=(const LogFactory& other) = delete;

    ~LogFactory() = default;
    
    static LogFactory& GetInstance() {
        static LogFactory inst;
        return inst;
    }

    void SetLogHandle(std::shared_ptr<VariadicLogHandle> log_handle) {
        log_handle_ = log_handle;
    }

    VariadicLogHandle* GetLogHandle() {
        return log_handle_.get();
    }

private:
    LogFactory() = default;

    std::shared_ptr<VariadicLogHandle> log_handle_;

};  // class LogFactory

}   // namespace logger