#include "LaunchR.hpp"
#include "widgets/MainFrame.hpp"

wxIMPLEMENT_APP(LaunchRApp); // NOLINT

bool LaunchRApp::OnInit()
{


    auto frame = new LR::MainFrame(nullptr);
    frame->SetIcon(wxIcon("IDI_ICON1"));
    frame->Show(true);

    settings = new LR::SettingsManager();

    return true;
}

int LaunchRApp::OnExit()
{
    delete settings;
    return 0;
}
