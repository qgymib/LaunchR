#ifndef LAUNCHR_SEARCHERS_FILENAME_HPP
#define LAUNCHR_SEARCHERS_FILENAME_HPP

#include "Searcher.hpp"

namespace LR
{

struct FileNameSearcher : Searcher
{
    IteratorPtr Query(const wxString& query) override;
};

} // namespace LR

#endif
