#include <wx/wx.h>
#include <wx/filefn.h>
#include <atomic>
#include <thread>
#include <mutex>
#include <semaphore>
#include <list>
#include "Utils/BoyerMoore.hpp"
#include "utils/FileSystem.hpp"
#include "LaunchR.hpp"
#include "Text.hpp"

using namespace LR;

typedef std::list<std::thread*>                  ThreadList;
typedef std::list<FileSystemTraversal::FileInfo> PathList;
typedef std::list<Searcher::Result>              ResultList;

struct TextSearcherIter : Searcher::Iterator
{
    explicit TextSearcherIter(const wxString& query);
    ~TextSearcherIter() override;
    Searcher::ResultVariant Next() override;

    std::atomic_bool looping; /* Looping flag. */

    wxString            query;                          /* Query string. */
    std::thread*        fs_traversal_thread;            /* Filesystem traversal thread. */
    std::atomic_bool    fs_traversal_finished;          /* Filesystem traversal finished. */
    ThreadList          content_query_threads;          /* Content search threads. */
    std::atomic<size_t> const_query_threads_exit_count; /* The number of query threads exited. */

    PathList                   query_files;       /* File list to query. */
    std::mutex                 query_files_mutex; /* Mutex for query_files. */
    std::counting_semaphore<>* query_files_sem;   /* Semaphore for file list. */

    std::mutex result_mutex;
    ResultList result_list;
};

static void TextSearchFileSystem(TextSearcherIter* searcher)
{
    wxString cwd = wxGetCwd();

    FileSystemTraversal::Traversal(cwd, SIZE_MAX, [searcher](const FileSystemTraversal::FileInfo& info) {
        if (info.isfile)
        {
            std::lock_guard<std::mutex> guard(searcher->query_files_mutex);
            searcher->query_files.push_back(info);
        }
        searcher->query_files_sem->release();

        return static_cast<bool>(searcher->looping);
    });

    searcher->fs_traversal_finished = true;
}

static void TextSearchFileContent(TextSearcherIter* searcher, const FileSystemTraversal::FileInfo& info, void* data,
                                  size_t size)
{
    wxScopedCharBuffer buf = searcher->query.ToUTF8();
    size_t             bufSz = buf.length();
    if (bufSz > size)
    {
        return;
    }

    BoyerMoore bm(buf.data(), bufSz);
    auto       ret = bm.Search(data, size);
    if (!ret.has_value())
    {
        return;
    }

    Searcher::Result result;
    result.title = info.name;
    result.path = info.path;

    {
        std::lock_guard<std::mutex> guard(searcher->result_mutex);
        searcher->result_list.push_back(result);
    }
}

static void TextSearchFileWithPath(TextSearcherIter* searcher, const FileSystemTraversal::FileInfo& info)
{
    FileMemoryMap view(info.path);
    void*         addr = view.GetAddr();
    if (addr == nullptr)
    {
        return;
    }
    size_t size = view.GetSize();
    size_t cfgTextMaxSize = wxGetApp().settings->Get().TextMaxSize;
    if (cfgTextMaxSize != 0 && size > cfgTextMaxSize)
    {
        size = cfgTextMaxSize;
    }

    TextSearchFileContent(searcher, info, addr, size);
}

static void TextSearchFile(TextSearcherIter* searcher)
{
    while (searcher->looping && !searcher->fs_traversal_finished)
    {
        (void)searcher->query_files_sem->try_acquire_for(std::chrono::milliseconds(100));

        FileSystemTraversal::FileInfo fileInfo;
        {
            std::lock_guard<std::mutex> guard(searcher->query_files_mutex);
            if (searcher->query_files.empty())
            {
                continue;
            }
            fileInfo = searcher->query_files.front();
            searcher->query_files.pop_front();
        }
        TextSearchFileWithPath(searcher, fileInfo);
    }

    ++searcher->const_query_threads_exit_count;
}

TextSearcherIter::TextSearcherIter(const wxString& query)
{
    /* Use max 12 threads to query text. */
    unsigned cpus = std::thread::hardware_concurrency();
    if (cpus == 0 || cpus > 12)
    {
        cpus = 12;
    }

    this->query = query;
    this->fs_traversal_finished = false;
    this->const_query_threads_exit_count = 0;
    this->query_files_sem = new std::counting_semaphore<>(0);

    if (!query.empty())
    {
        looping = true;
        fs_traversal_thread = new std::thread(TextSearchFileSystem, this);
        for (unsigned i = 0; i < cpus; i++)
        {
            content_query_threads.push_back(new std::thread(TextSearchFile, this));
        }
    }
    else
    {
        looping = false;
        fs_traversal_thread = nullptr;
    }
}

TextSearcherIter::~TextSearcherIter()
{
    looping = false;

    if (fs_traversal_thread != nullptr)
    {
        fs_traversal_thread->join();
        delete fs_traversal_thread;
    }

    for (auto t : content_query_threads)
    {
        t->join();
        delete t;
    }

    delete query_files_sem;
}

Searcher::ResultVariant TextSearcherIter::Next()
{
    if (!looping)
    {
        return Searcher::ResultCode::End;
    }

    {
        std::lock_guard<std::mutex> guard(result_mutex);
        if (!result_list.empty())
        {
            Searcher::Result result = result_list.front();
            result_list.pop_front();
            return result;
        }
    }

    if (const_query_threads_exit_count != content_query_threads.size())
    {
        return Searcher::ResultCode::TryAgain;
    }
    return Searcher::ResultCode::End;
}

Searcher::IteratorPtr TextSearcher::Query(const wxString& query)
{
    return std::make_shared<TextSearcherIter>(query);
}
