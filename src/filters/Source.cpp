#include <wx/wx.h>
#include <wx/filename.h>
#include <atomic>
#include <thread>
#include <semaphore>
#include <list>
#include "Source.hpp"

using namespace LR;
typedef std::list<Filter::MsgPtr> MsgList;
typedef std::list<wxString>       PathQueue;

struct Source::Data
{
    Data(const wxString& query, Callback cb);
    ~Data();

    Callback cb;
    wxString query;

    std::atomic_bool       looping;
    std::binary_semaphore* fs_sem;
    std::thread*           fs_thread;
};

static std::optional<uint64_t> FiletimeToEpoch(const FILETIME* ft)
{
    if (ft->dwHighDateTime == 0 && ft->dwLowDateTime == 0)
    {
        return std::nullopt;
    }

    ULARGE_INTEGER ull;
    ull.LowPart = ft->dwLowDateTime;
    ull.HighPart = ft->dwHighDateTime;

    const int64_t EPOCH_DIFFERENCE = 116444736000000000LL;
    return (ull.QuadPart - EPOCH_DIFFERENCE) / 10000000LL;
}

static void SourceThreadFsTraversal(Source::Data* data, const wxString& path, PathQueue& pathQueue, MsgList& msgQueue)
{
    const wxUniChar sep = wxFileName::GetPathSeparator();
    const wxString  searchPattern = path + sep + "*";

    WIN32_FIND_DATAW findData;
    HANDLE           hFile = FindFirstFileW(searchPattern.wc_str(), &findData);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return;
    }

    do
    {
        if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0)
        {
            continue;
        }
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            const wxString subdir = path + sep + findData.cFileName;
            pathQueue.push_back(subdir);
            continue;
        }

        Filter::Record record;
        record.Name = findData.cFileName;
        record.Path = path + sep + record.Name;
        record.Size = (findData.nFileSizeHigh * (MAXDWORD + 1)) + findData.nFileSizeLow;
        record.CreationTime = FiletimeToEpoch(&findData.ftCreationTime);
        record.ModificationTime = FiletimeToEpoch(&findData.ftLastWriteTime);

        Filter::MsgPtr msg = std::make_shared<Filter::Msg>(std::move(record));
        msgQueue.push_back(msg);
    } while (data->looping && FindNextFileW(hFile, &findData));

    FindClose(hFile);
}

static void SourceThread(Source::Data* data)
{
    MsgList   msgQueue;
    PathQueue pathQueue;
    pathQueue.push_back(data->query);

    /* Wait for start command. */
    data->fs_sem->acquire();

    /* Notify start of stream. */
    data->cb(std::make_shared<Filter::Msg>(Filter::Cmd::Start));

    while (data->looping)
    {
        if (!msgQueue.empty())
        {
            Filter::MsgPtr msg = msgQueue.front();
            msgQueue.pop_front();
            data->cb(msg);
            continue;
        }
        if (pathQueue.empty())
        {
            break;
        }

        wxString path = pathQueue.front();
        pathQueue.pop_front();

        SourceThreadFsTraversal(data, path, pathQueue, msgQueue);
    }

    /* Notify end of stream. */
    data->cb(std::make_shared<Filter::Msg>(Filter::Cmd::End));
}

Source::Data::Data(const wxString& query, Callback cb)
{
    this->query = query;
    this->cb = cb;

    looping = true;
    fs_sem = new std::binary_semaphore(0);

    fs_thread = new std::thread(SourceThread, this);
}

Source::Data::~Data()
{
    looping = false;
    fs_sem->release();

    fs_thread->join();
    delete fs_thread;

    delete fs_sem;
}

Source::Source(const wxString& query, Callback cb) : Filter(query, cb)
{
    m_data = new Data(query, cb);
}

Source::~Source()
{
    delete m_data;
}

void Source::Handle(MsgPtr msg)
{
    /* Not accept records. */
    if (!std::holds_alternative<Filter::Cmd>(*msg))
    {
        return;
    }

    const auto cmd = std::get<Filter::Cmd>(*msg);
    if (cmd == Filter::Cmd::End)
    {
        m_data->looping = false;
    }
    m_data->fs_sem->release();
}
