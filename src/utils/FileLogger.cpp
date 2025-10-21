#include <wx/wx.h>
#include <wx/log.h>
#include <wx/filename.h>
#include <wx/strconv.h>
#include "LaunchR.hpp"
#include "FileLogger.hpp"

using LR::FileLogger;

struct FileLogger::Data
{
    Data();
    ~Data();

    wxString     path;
    FILE*        file;
    wxLogStderr* logger;
};

static wxString GetLogPath()
{
    const std::string& path = wxGetApp().settings->Get().log.path;
    if (!path.empty())
    {
        return wxString::FromUTF8(path);
    }

    return LaunchRApp::GenDataPath("log.txt");
}

static void CreateLogDirIfNotExist(const wxString& path)
{
    wxFileName f(path);
    if (f.DirExists())
    {
        return;
    }

    if (!f.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL))
    {
        wxLogError("Failed to create directory: %s", path);
    }
}

FileLogger::Data::Data()
{
    path = GetLogPath();

    if (wxGetApp().settings->Get().log.enable)
    {
        CreateLogDirIfNotExist(path);

        const char* path_c = path.c_str();
        file = fopen(path_c, "wb");
    }
    else
    {
        file = nullptr;
    }

    logger = new wxLogStderr(file, wxConvUTF8);
}

FileLogger::Data::~Data()
{
    delete logger;
    if (file != nullptr)
    {
        fclose(file);
    }
}

FileLogger::FileLogger()
{
    m_data = new Data;
}

FileLogger::~FileLogger()
{
    delete m_data;
}
