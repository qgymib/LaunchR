#ifndef LAUNCHR_FILTERS_FILTER_HPP
#define LAUNCHR_FILTERS_FILTER_HPP

#include <string>
#include <optional>
#include <vector>
#include <memory>
#include <functional>

namespace LR
{

struct Filter
{
    struct Item
    {
        typedef std::vector<uint8_t>  Binary;
        typedef std::shared_ptr<Item> Ptr;

        bool                  is_eof;   /* End of items. If true, all other fields should be ignored. */
        std::string           u8_title; /* Item title in UTF-8. */
        std::string           u8_path;  /* Item path in filesystem, encoding in UTF-8. */
        std::optional<Binary> content;  /* File content. */
    };
    typedef std::function<void(Item::Ptr item)> Callback;

    Filter(const std::string& u8_query, Callback cb);
    virtual ~Filter();
    virtual void Append(Item::Ptr item);
};

} // namespace LR

#endif
