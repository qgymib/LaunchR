#ifndef LAUNCHR_SEARCHER_HPP
#define LAUNCHR_SEARCHER_HPP

#include <wx/string.h>
#include <variant>
#include <optional>
#include <memory>

namespace LR
{

struct Searcher
{
    struct Result
    {
        wxString                title; /* Item title */
        std::optional<wxString> path;  /* Item path. */
    };
    enum class ResultCode : int
    {
        TryAgain, /* Try again. */
        End,      /* No data. */
    };
    using ResultVariant = std::variant<Result, ResultCode>;

    struct Iterator
    {
        Iterator() = default;
        Iterator(const Iterator& it) = delete;
        virtual ~Iterator() = default;

        virtual ResultVariant Next();
    };
    using IteratorPtr = std::shared_ptr<Iterator>;

    virtual ~Searcher() = default;
    virtual IteratorPtr Query(const wxString& query);
};

} // namespace LR

#endif
