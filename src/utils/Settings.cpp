#include <wx/wx.h>
#include <wx/filename.h>
#include <nlohmann/json.hpp>
#include "LaunchR.hpp"
#include "Settings.hpp"

using LR::SettingsManager;

namespace LR
{
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SettingLog, enable, path)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Settings, log, PortableAppSupport, FileNameSupport, TextSupport,
                                                TextMaxSize)
} // namespace LR

struct SettingsManager::Data
{
    Data();

    LR::Settings settings;   /* Application configurations. */
    wxString     configPath; /* Configuration file path. */
};

/**
 * Retrieves the configuration file path for the application.
 *
 * This function constructs the path to the configuration file named
 * "config.json" located within a hidden ".LaunchR" directory
 * at the same directory level as the application's executable.
 *
 * The path includes the directory separator specific to the current
 * platform, ensuring proper handling of file paths in a cross-platform
 * manner.
 *
 * @return The full path to the configuration file as a wxString.
 */
static wxString GetConfigPath()
{
    return LaunchRApp::GenDataPath("config.json");
}

bool LoadConfig(const wxString& path, nlohmann::json* data)
{
    wxLogNull logNo; /* Temporary closure of error messages */
    wxFile    file;
    if (!file.Open(path, wxFile::read))
    {
        return false;
    }

    wxString content;
    file.ReadAll(&content);
    file.Close();

    *data = nlohmann::json::parse(content);
    return true;
}

static void SaveConfig(const wxString& path, const LR::Settings& config)
{
    wxFileName f(path);
    if (!f.DirExists())
    {
        if (!f.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL))
        {
            wxLogError("Failed to create directory: %s", path);
            return;
        }
    }

    wxFile file;
    if (!file.Open(path, wxFile::write))
    {
        wxLogError("Failed to open or create the file: %s", path);
        return;
    }

    nlohmann::json j = config;
    file.Write(j.dump(4));
    file.Close();
}

SettingsManager::Data::Data()
{
    configPath = GetConfigPath();

    nlohmann::json json;
    LoadConfig(configPath, &json);
    if (!json.empty())
    {
        settings = json.get<LR::Settings>();
    }
}

SettingsManager::SettingsManager()
{
    m_data = new Data();
}

SettingsManager::~SettingsManager()
{
    delete m_data;
}

const LR::Settings& SettingsManager::Get() const
{
    return m_data->settings;
}

void SettingsManager::Set(const Settings& config)
{
    m_data->settings = config;
    SaveConfig(m_data->configPath, config);
}
