#include "decode_formatter.h"

#include <vector>

/*
 * @brief EffectiveMsg custom pattern
 *
 * %l log_level
 * %D timestamp date
 * %S timestamp seconds
 * %M timestamp milliseconds
 * %p process_id
 * %t thread_id
 * %# line
 * %F file_name
 * %f func_name
 * %v log_info
 *
 * such as: [%l][%D:%S][%p:%t][%F:%f:%#]%v
*/

namespace {

class FlagFormatter {
public:
    virtual ~FlagFormatter() = default;

    virtual void Format(const EffectiveMsg& msg, std::string& dest) = 0;
};

std::vector<std::unique_ptr<FlagFormatter>> flag_formatters;

void ParsePattern(const std::string& pattern);

std::string CombineLogMsg(const EffectiveMsg& msg);

} // namespace

void DecodeFormatter::SetPattern(const std::string& pattern) {
    ParsePattern(pattern);
}

void DecodeFormatter::Format(const EffectiveMsg& msg, std::string& dest) {
    if (!flag_formatters.empty()) {
        for (const auto& formatter : flag_formatters) {
            formatter->Format(msg, dest);
        }
    } else {
        dest.append(CombineLogMsg(msg));
    }
}

namespace  {

const char* GetLevelStr(int32_t level) {
    switch (level) {
        case 0:
            return "V";
        case 1:
            return "D";
        case 2:
            return "I";
        case 3:
            return "W";
        case 4:
            return "E";
        case 5:
            return "F";
        default:
            return "U";
    }
}

std::string MillisecondsToDateString(long long milliseconds) {
    std::chrono::system_clock::time_point tp = 
        std::chrono::system_clock::time_point(std::chrono::milliseconds(milliseconds));
    std::time_t time_tt = std::chrono::system_clock::to_time_t(tp);
    std::tm* timeinfo = std::localtime(&time_tt);
    std::ostringstream oss;
    oss << std::put_time(timeinfo, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

class AggregateFormatter final : public FlagFormatter {
public:
    ~AggregateFormatter() override = default;

    void AddChild(char ch) {str_ += ch;}

    void Format(const EffectiveMsg& msg, std::string& dest) override {
        dest.append(str_); 
    }
private:
    std::string str_;
};

class LogLevelFormatter final : public FlagFormatter {
public:
    ~LogLevelFormatter() override = default;

    void Format(const EffectiveMsg& msg, std::string& dest) override { 
        dest.append(GetLevelStr(msg.level())); 
    }
};

class TimestampDateFormatter final : public FlagFormatter {
public:
    ~TimestampDateFormatter() override = default;

    void Format(const EffectiveMsg& msg, std::string& dest) override { 
        dest.append(MillisecondsToDateString(msg.timestamp())); 
    }
};

class TimestampSecondsFormatter final : public FlagFormatter {
public:
    ~TimestampSecondsFormatter() override = default;

    void Format(const EffectiveMsg& msg, std::string& dest) override {
        dest.append(std::to_string(msg.timestamp() / 1000)); 
    }
};

class TimestampMillisecondsFormatter final : public FlagFormatter {
public:
    ~TimestampMillisecondsFormatter() override = default;

    void Format(const EffectiveMsg& msg, std::string& dest) override { 
        dest.append(std::to_string(msg.timestamp())); 
    }
};

class ProcessIdFormatter final : public FlagFormatter {
public:
    ~ProcessIdFormatter() override = default;

    void Format(const EffectiveMsg& msg, std::string& dest) override { 
        dest.append(std::to_string(msg.pid())); 
    }
};

class ThreadIdFormatter final : public FlagFormatter {
public:
    ~ThreadIdFormatter() override = default;

    void Format(const EffectiveMsg& msg, std::string& dest) override { 
        dest.append(std::to_string(msg.tid())); 
    }
};

class LineFormatter final : public FlagFormatter {
public:
    ~LineFormatter() override = default;

    void Format(const EffectiveMsg& msg, std::string& dest) override { 
        dest.append(std::to_string(msg.line())); 
    }
};

class FileNameFormatter final : public FlagFormatter {
public:
    ~FileNameFormatter() override = default;

    void Format(const EffectiveMsg& msg, std::string& dest) override { 
        dest.append(msg.file_name()); 
    }
};

class FuncNameFormatter final : public FlagFormatter {
public:
    ~FuncNameFormatter() override = default;

    void Format(const EffectiveMsg& msg, std::string& dest) override { 
        dest.append(msg.func_name()); 
    }
};

class LogInfoFormatter final : public FlagFormatter {
public:
    ~LogInfoFormatter() override = default;

    void Format(const EffectiveMsg& msg, std::string& dest) override { 
        dest.append(msg.log_info()); 
    }
};

std::unique_ptr<FlagFormatter> GetFormatterByFlag(char flag) {
    switch (flag) {
        case 'l':
            return std::make_unique<LogLevelFormatter>();
        case 'D':
            return std::make_unique<TimestampDateFormatter>();
        case 'S':
            return std::make_unique<TimestampSecondsFormatter>();
        case 'M':
            return std::make_unique<TimestampMillisecondsFormatter>();
        case 'p':
            return std::make_unique<ProcessIdFormatter>();
        case 't':
            return std::make_unique<ThreadIdFormatter>();
        case '#':
            return std::make_unique<LineFormatter>();
        case 'F':
            return std::make_unique<FileNameFormatter>();
        case 'f':
            return std::make_unique<FuncNameFormatter>();
        case 'v':
            return std::make_unique<LogInfoFormatter>();
        default:
            auto formatter = std::make_unique<AggregateFormatter>();
            formatter->AddChild('%');
            formatter->AddChild(flag);
            return formatter;
    }
}

void ParsePattern(const std::string& pattern) {
    std::unique_ptr<AggregateFormatter> user_chars;
    flag_formatters.clear();

    for (auto it = pattern.begin(); it != pattern.end(); ++it) {
        if (*it == '%') {
            if (user_chars) {
                flag_formatters.push_back(std::move(user_chars));
            }
            ++it;
            if (it == pattern.end()) {
                break;
            }
            flag_formatters.push_back(GetFormatterByFlag(*it));
        } else {
            if (!user_chars) {
                user_chars = std::make_unique<AggregateFormatter>();
            }
            user_chars->AddChild(*it);
        }
    }

    if (user_chars) {
        flag_formatters.push_back(std::move(user_chars));
    }
}

std::string CombineLogMsg(const EffectiveMsg& msg) {
    std::string output;
    char buffer[1024] = {0};
    snprintf(buffer, sizeof(buffer), "[%d][%lld][%d:%d][%s:%s:%d]", 
        msg.level(), msg.timestamp(), msg.pid(), msg.tid(), msg.file_name().c_str(), msg.func_name().c_str(), msg.line());
    output.append(buffer);
    output.append(msg.log_info());
    return output;
}
    
} // namespace 



