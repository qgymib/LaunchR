#ifndef LAUNCHR_FILTERS_FILTER_HPP
#define LAUNCHR_FILTERS_FILTER_HPP

#include <wx/wx.h>
#include <wx/string.h>
#include <optional>
#include <vector>
#include <memory>
#include <functional>
#include <variant>

namespace LR
{

struct Filter
{
    enum class Cmd
    {
        Start,
        End,
    };

    struct Record
    {
        typedef std::vector<uint8_t> Binary;

        wxString                Name;             /* Item name in. */
        wxString                Path;             /* Item path in filesystem. */
        uint64_t                Size;             /* File size. */
        std::optional<uint64_t> CreationTime;     /* Creation time in epoch. */
        std::optional<uint64_t> ModificationTime; /* Modification time in epoch. */
        std::optional<Binary>   Content;          /* File content. */
    };

    typedef std::variant<Cmd, Record>   Msg;
    typedef std::shared_ptr<Msg>        MsgPtr;
    typedef std::function<void(MsgPtr)> Callback;

    Filter(const wxString& query, Callback cb);
    virtual ~Filter();
    virtual void Handle(MsgPtr msg);
};

} // namespace LR

#endif
