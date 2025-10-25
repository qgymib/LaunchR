#ifndef LAUNCHR_SEARCHER_TEXT_HPP
#define LAUNCHR_SEARCHER_TEXT_HPP

#include "Searcher.hpp"

namespace LR
{

struct TextSearcher : Searcher
{
    IteratorPtr Query(const wxString& query) override;
};

} // namespace LR

#endif
