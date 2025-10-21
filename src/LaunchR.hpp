#ifndef LAUNCHR_HPP
#define LAUNCHR_HPP

#include <wx/app.h>
#include "utils/Settings.hpp"

class LaunchRApp final : public wxApp
{
public:
    bool OnInit() override;
    int  OnExit() override;

public:
    LR::SettingsManager* settings = nullptr;
};

wxDECLARE_APP(LaunchRApp);

#endif
