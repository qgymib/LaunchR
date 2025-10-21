#ifndef LAUNCHR_SEARCHERS_PORTABLEAPPS_HPP
#define LAUNCHR_SEARCHERS_PORTABLEAPPS_HPP

#include "Searcher.hpp"

namespace LR
{

struct PortableAppSearcher : Searcher
{
    PortableAppSearcher();
    ~PortableAppSearcher() override;

    IteratorPtr Query(const wxString& query) override;

    struct Data;
    struct Data* m_data;
};

} // namespace LR

#endif
