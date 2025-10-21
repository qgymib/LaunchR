#ifndef LAUNCHR_UTILS_SETTINGS_HPP
#define LAUNCHR_UTILS_SETTINGS_HPP

namespace LR
{

struct Settings
{
    bool use_ripgrep = true; /* Enable ripgrep support. */
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
