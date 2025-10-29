#ifndef LAUNCHR_FILTERS_FILENAME_HPP
#define LAUNCHR_FILTERS_FILENAME_HPP

#include "Filter.hpp"

namespace LR
{

struct FileNameFilter : Filter
{
    FileNameFilter(const wxString& query, Callback cb);
    ~FileNameFilter() override;
    void Handle(MsgPtr msg) override;

    struct Data;
    struct Data* m_data;
};

}

#endif
