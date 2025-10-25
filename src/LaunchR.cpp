#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include "searchers/FileName.hpp"
#include "searchers/PortableApps.hpp"
#include "searchers/Text.hpp"
#include "widgets/MainFrame.hpp"
#include "LaunchR.hpp"

wxIMPLEMENT_APP(LaunchRApp); // NOLINT

using namespace LR;

static void RegisterSearcher(LaunchRApp* app)
{
    if (app->settings->Get().PortableAppSupport)
    {
        app->searchers.push_back(new PortableAppSearcher);
    }
    if (app->settings->Get().FileNameSupport)
    {
        app->searchers.push_back(new FileNameSearcher);
    }
    if (app->settings->Get().TextSupport)
    {
        app->searchers.push_back(new TextSearcher);
    }
}

bool LaunchRApp::OnInit()
{
    wxLog::SetLogLevel(wxLOG_Debug);

    settings = new LR::SettingsManager();
    logger = new LR::FileLogger();
    RegisterSearcher(this);

    auto frame = new LR::MainFrame(nullptr);
    frame->SetIcon(wxIcon("IDI_ICON1"));
    frame->Show(true);

    return true;
}

int LaunchRApp::OnExit()
{
    for (auto searcher : searchers)
    {
        delete searcher;
    }
    delete logger;
    delete settings;
    return 0;
}

wxString LaunchRApp::GetWorkingDir()
{
    const wxString exePath = wxStandardPaths::Get().GetExecutablePath();

    wxString dataDir;
    wxFileName::SplitPath(exePath, &dataDir, nullptr, nullptr, wxPATH_NATIVE);

    return dataDir;
}

wxString LaunchRApp::GenDataPath(const char* name)
{
    const wxUniChar sep = wxFileName::GetPathSeparator();
    const wxString  dataDir = GetWorkingDir();

    wxString ret = dataDir + sep + ".LaunchR";
    if (name != nullptr)
    {
        ret += sep + wxString::FromUTF8(name);
    }

    return ret;
}
