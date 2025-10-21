#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include "LaunchR.hpp"
#include "widgets/MainFrame.hpp"

wxIMPLEMENT_APP(LaunchRApp); // NOLINT

bool LaunchRApp::OnInit()
{
    settings = new LR::SettingsManager();
    logger = new LR::FileLogger();

    auto frame = new LR::MainFrame(nullptr);
    frame->SetIcon(wxIcon("IDI_ICON1"));
    frame->Show(true);

    return true;
}

int LaunchRApp::OnExit()
{
    delete logger;
    delete settings;
    return 0;
}

wxString LaunchRApp::GenDataPath(const char* name)
{
    const wxUniChar sep = wxFileName::GetPathSeparator();
    const wxString  exePath = wxStandardPaths::Get().GetExecutablePath();

    wxString dataDir;
    wxFileName::SplitPath(exePath, &dataDir, nullptr, nullptr, wxPATH_NATIVE);

    wxString ret = dataDir + sep + ".LaunchR";
    if (name != nullptr)
    {
        ret += sep + wxString::FromUTF8(name);
    }

    return ret;
}
