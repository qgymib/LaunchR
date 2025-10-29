#ifndef LAUNCHR_FILTERS_SOURCE_HPP
#define LAUNCHR_FILTERS_SOURCE_HPP

#include "Filter.hpp"

namespace LR
{

struct Source : Filter
{
    /**
     * @brief Recursive traverse all files and folders in directory.
     * @param[in] query Top level directory.
     * @param[in] cb Data callback.
     */
    Source(const wxString& query, Callback cb);
    ~Source() override;
    void Handle(MsgPtr msg) override;

    struct Data;
    struct Data* m_data;
};

} // namespace LR

#endif
