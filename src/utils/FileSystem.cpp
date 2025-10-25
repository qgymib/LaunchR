#include <wx/wx.h>
#include <wx/log.h>
#include <filesystem>
#include <list>
#include "FileSystem.hpp"

using namespace LR;

struct PathRecord
{
    typedef std::list<PathRecord> Queue;
    PathRecord(const std::wstring& path, size_t level);
    std::wstring path;
    size_t       level;
};

PathRecord::PathRecord(const std::wstring& path, size_t level)
{
    this->path = path;
    this->level = level;
}

void FileSystemTraversal::Traversal(const wxString& path, size_t level, Callback cb)
{
    const std::wstring wpath = path.ToStdWstring();

    PathRecord::Queue pathQueue;
    pathQueue.push_back(PathRecord(wpath, 0));

    bool looping = true;
    while (looping && !pathQueue.empty())
    {
        PathRecord record = pathQueue.front();
        pathQueue.pop_front();

        if (record.level > level)
        {
            break;
        }

        try
        {
            for (const auto& entry : std::filesystem::directory_iterator(record.path))
            {
                const bool is_directory = entry.is_directory();
                const bool is_regular_file = entry.is_regular_file();
                if (!is_regular_file && !is_directory)
                {
                    continue;
                }

                std::filesystem::path filePath = entry.path();
                FileInfo              info;
                info.name = filePath.filename().wstring();
                info.path = filePath.wstring();
                info.isfile = is_regular_file;
                if (!cb(info))
                {
                    looping = false;
                    break;
                }

                if (is_directory)
                {
                    pathQueue.push_back(PathRecord(filePath.wstring(), record.level + 1));
                }
            }
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            wxLogVerbose("Access fs failed: %s", e.what());
        }
    }
}

struct FileMemoryMap::Data
{
    Data(const wxString& path);
    ~Data();
    wxString path;
    HANDLE   hFile = INVALID_HANDLE_VALUE;
    HANDLE   hMapFile = nullptr;
    LPVOID   pMappedView = nullptr;
};

FileMemoryMap::Data::Data(const wxString& path)
{
    this->path = path;

    hFile = CreateFileW(path.wc_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                        nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        wxLogWarning("Cannot open file `" + path + "`");
        return;
    }

    hMapFile = CreateFileMappingW(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (hMapFile == nullptr)
    {
        wxLogWarning("Cannot create file mapping for `" + path + "`");
        return;
    }

    pMappedView = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
}

FileMemoryMap::Data::~Data()
{
    if (pMappedView != nullptr)
    {
        UnmapViewOfFile(pMappedView);
        pMappedView = nullptr;
    }
    if (hMapFile != nullptr)
    {
        CloseHandle(hMapFile);
        hMapFile = nullptr;
    }
    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }
}

FileMemoryMap::FileMemoryMap(const wxString& path)
{
    m_data = new Data(path);
}

FileMemoryMap::~FileMemoryMap()
{
    delete m_data;
}

void* FileMemoryMap::GetAddr()
{
    return m_data->pMappedView;
}

size_t FileMemoryMap::GetSize()
{
    DWORD fileSizeHigh = 0;
    DWORD fileSizeLow = GetFileSize(m_data->hFile, &fileSizeHigh);

    LARGE_INTEGER fileSize;
    fileSize.LowPart = fileSizeLow;
    fileSize.HighPart = fileSizeHigh;
    return fileSize.QuadPart;
}
