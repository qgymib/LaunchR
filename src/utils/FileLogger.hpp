#ifndef LAUNCHR_UTILS_FILELOGGER_HPP
#define LAUNCHR_UTILS_FILELOGGER_HPP

namespace LR
{

class FileLogger
{
public:
    FileLogger();
    ~FileLogger();

public:
    struct Data;
    struct Data* m_data;
};

} // namespace LR

#endif
