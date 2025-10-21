#ifndef LAUNCHR_HPP
#define LAUNCHR_HPP

#include <wx/wx.h>
#include <wx/app.h>
#include "utils/FileLogger.hpp"
#include "utils/Settings.hpp"

class LaunchRApp final : public wxApp
{
public:
    bool OnInit() override;
    int  OnExit() override;

public:
    static wxString GenDataPath(const char* name);

public:
    LR::SettingsManager* settings = nullptr; /* Settings manager. */
    LR::FileLogger*      logger = nullptr;   /* File logger. */
};

wxDECLARE_APP(LaunchRApp);

#endif
