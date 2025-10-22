#include <wx/wx.h>
#include <wx/regex.h>
#include <atomic>
#include <thread>
#include <list>
#include <mutex>
#include <filesystem>
#include "LaunchR.hpp"
#include "FileName.hpp"

using namespace LR;

struct FileNameSearcherIter : Searcher::Iterator
{
    FileNameSearcherIter(const wxString& query);
    ~FileNameSearcherIter() override;
    Searcher::ResultVariant Next() override;

    wxRegEx                 query_regex;         /* Regex for query. */
    std::atomic<bool>       flag_running = true; /* Looping flag. */
    std::thread*            search_thread;       /* Search threads. */
    std::list<std::wstring> pending_paths;       /* Paths to search. */

    bool                        flag_done = false; /* Search done flag. */
    std::list<Searcher::Result> results;           /* Storage for search results. */
    std::mutex                  result_mutex;      /* Mutex for results. */
};

static void SearchFileNameInPath(struct FileNameSearcherIter* searcher, const std::wstring& path)
{
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        if (!searcher->flag_running)
        {
            break;
        }

        if (entry.is_directory())
        {
            searcher->pending_paths.push_back(entry.path().wstring());
            continue;
        }

        if (!entry.is_regular_file())
        {
            continue;
        }

        wxString name(entry.path().filename().wstring());
        if (searcher->query_regex.Matches(name))
        {
            Searcher::Result ret;
            ret.title = name;
            ret.path = entry.path().wstring();

            std::lock_guard<std::mutex> lock(searcher->result_mutex);
            searcher->results.push_back(ret);
        }
    }
}

static void SearchFileNameThread(struct FileNameSearcherIter* searcher)
{
    while (searcher->flag_running && !searcher->pending_paths.empty())
    {
        std::wstring path = searcher->pending_paths.front();
        searcher->pending_paths.pop_front();
        SearchFileNameInPath(searcher, path);
    }

    {
        std::lock_guard<std::mutex> lock(searcher->result_mutex);
        searcher->flag_done = true;
    }
}

FileNameSearcherIter::FileNameSearcherIter(const wxString& query)
{
    query_regex.Compile(query);

    const wxString     search_path = LaunchRApp::GetWorkingDir();
    const std::wstring search_path_std = search_path.ToStdWstring();
    pending_paths.push_back(search_path_std);

    search_thread = new std::thread(SearchFileNameThread, this);
}

FileNameSearcherIter::~FileNameSearcherIter()
{
    flag_running = false;
    search_thread->join();
    delete search_thread;
}

Searcher::ResultVariant FileNameSearcherIter::Next()
{
    std::lock_guard<std::mutex> lock(result_mutex);
    if (results.empty())
    {
        return flag_done ? Searcher::ResultCode::End : Searcher::ResultCode::TryAgain;
    }
    Searcher::Result ret = results.front();
    results.pop_front();
    return ret;
}

Searcher::IteratorPtr FileNameSearcher::Query(const wxString& query)
{
    return std::make_shared<FileNameSearcherIter>(query);
}
