#ifndef LAUNCHR_UTILS_FILE_SYSTEM_HPP
#define LAUNCHR_UTILS_FILE_SYSTEM_HPP

#include <wx/wx.h>
#include <functional>

namespace LR
{

struct FileSystemTraversal
{
    struct FileInfo
    {
        wxString name;   /* File name. */
        wxString path;   /* File path. */
        bool     isfile; /* True if file, false if directory. */
    };

    /**
     * @brief Traversal callback function.
     * @return true to continue traverse, false to stop.
     */
    typedef std::function<bool(const FileInfo& info)> Callback;

    /**
     * @brief FileSystem traversal.
     * @param[in] path Filesystem path.
     * @param[in] level Directory level. 0 is the first level.
     * @param[in] cb Result callback.
     */
    static void Traversal(const wxString& path, size_t level, Callback cb);
};

struct FileMemoryMap
{
    explicit FileMemoryMap(const wxString& path);
    ~FileMemoryMap();

    void* GetAddr();
    size_t GetSize();

    struct Data;
    struct Data* m_data;
};

} // namespace LR

#endif
