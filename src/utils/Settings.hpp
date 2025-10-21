#ifndef LAUNCHR_UTILS_SETTINGS_HPP
#define LAUNCHR_UTILS_SETTINGS_HPP

namespace LR
{

struct SettingLog
{
    bool        enable = false; /* Enable logging file. */
    std::string path;           /* File path. */
};

struct Settings
{
    SettingLog log; /* Log configuration. */
};

class SettingsManager
{
public:
    SettingsManager();
    ~SettingsManager();

public:
    const Settings& Get() const;
    void            Set(const Settings& config);

private:
    struct Data;
    struct Data* m_data;
};

} // namespace LR

#endif
