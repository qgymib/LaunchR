#include <wx/wx.h>
#include <wx/dir.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/regex.h>
#include <thread>
#include <mutex>
#include "LaunchR.hpp"
#include "PortableApps.hpp"

using namespace LR;
typedef std::list<Searcher::Result> ResultList;

struct PortableAppSearcher::Data
{
    Data();
    ~Data();

    ResultList results;
    bool       search_finished;
    std::mutex result_mutex;

    wxRegEx      launcher_regex;
    std::thread* scan_thread;
};

struct PortableAppSearcherIterator : Searcher::Iterator
{
    PortableAppSearcherIterator(struct PortableAppSearcher::Data* searcher, const wxString& query);
    Searcher::ResultVariant Next() override;

    struct PortableAppSearcher::Data* searcher;
    wxString                          query;
    ResultList                        query_results;
    ResultList::iterator              current;
    bool                              flag_query = false;
};

static wxArrayString GetFirstLevelFolder(const wxString& path)
{
    wxArrayString folders;

    wxDir d(path);
    if (!d.IsOpened())
    {
        return folders;
    }

    wxString name;
    bool     cont = d.GetFirst(&name, wxEmptyString, wxDIR_DIRS);
    while (cont)
    {
        folders.Add(name);
        cont = d.GetNext(&name);
    }

    return folders;
}

static wxArrayString GetFirstLevelFile(const wxString& path)
{
    wxArrayString files;

    wxDir d(path);
    if (!d.IsOpened())
    {
        return files;
    }

    wxString name;
    bool     cont = d.GetFirst(&name, wxEmptyString, wxDIR_FILES);
    while (cont)
    {
        files.Add(name);
        cont = d.GetNext(&name);
    }

    return files;
}

static void SearchPortableLauncher(PortableAppSearcher::Data* data, const wxString& path, const wxArrayString& files)
{
    for (const wxString& name : files)
    {
        if (data->launcher_regex.Matches(name))
        {
            Searcher::Result ret;
            ret.title = data->launcher_regex.GetMatch(name, 1);
            ret.path = path + wxFileName::GetPathSeparator() + name;

            {
                std::lock_guard<std::mutex> lock(data->result_mutex);
                data->results.push_back(ret);
            }
        }
    }
}

static void SearchPortableApps(PortableAppSearcher::Data* data)
{
    const wxUniChar sep = wxFileName::GetPathSeparator();
    const wxString  cwd = LaunchRApp::GetWorkingDir();

    wxArrayString dirs = GetFirstLevelFolder(cwd);
    for (const wxString& name : dirs)
    {
        const wxString path = cwd + sep + name;
        wxArrayString  files = GetFirstLevelFile(path);
        SearchPortableLauncher(data, path, files);
    }

    {
        std::lock_guard<std::mutex> lock(data->result_mutex);
        data->search_finished = true;
    }
}

PortableAppSearcher::Data::Data()
{
    search_finished = false;
    launcher_regex.Compile("(.*Portable)\\.exe");
    scan_thread = new std::thread(SearchPortableApps, this);
}

PortableAppSearcher::Data::~Data()
{
    scan_thread->join();
    delete scan_thread;
}

PortableAppSearcher::PortableAppSearcher()
{
    m_data = new Data;
}

PortableAppSearcher::~PortableAppSearcher()
{
    delete m_data;
}

PortableAppSearcherIterator::PortableAppSearcherIterator(PortableAppSearcher::Data* searcher, const wxString& query)
{
    this->searcher = searcher;
    this->query = query; /* Since wxWidgets 3.3, all string copies are deep. */
}

static void PerformPortableAppsQuery(PortableAppSearcherIterator* iter)
{
    const wxString q_lower = iter->query.Lower();
    for (const auto& it : iter->searcher->results)
    {
        const wxString t_lower = it.title.Lower();
        const bool     matched = q_lower.empty() || t_lower.Contains(q_lower);

        if (!matched)
        {
            continue;
        }

        iter->query_results.push_back(it);
    }
    iter->current = iter->query_results.begin();
}

Searcher::ResultVariant PortableAppSearcherIterator::Next()
{
    bool search_finished = false;
    {
        std::lock_guard<std::mutex> lock(searcher->result_mutex);
        search_finished = searcher->search_finished;
    }

    if (!search_finished)
    {
        return Searcher::ResultCode::TryAgain;
    }

    if (!flag_query)
    {
        flag_query = true;
        PerformPortableAppsQuery(this);
    }

    if (current == query_results.end())
    {
        return Searcher::ResultCode::End;
    }

    Searcher::Result ret = *current;
    ++current;

    return ret;
}

Searcher::IteratorPtr PortableAppSearcher::Query(const wxString& query)
{
    return std::make_shared<PortableAppSearcherIterator>(m_data, query);
}
