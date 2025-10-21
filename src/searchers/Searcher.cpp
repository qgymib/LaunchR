#include "Searcher.hpp"

using namespace LR;

Searcher::IteratorPtr Searcher::Query(const wxString&)
{
    return std::make_shared<Searcher::Iterator>();
}

Searcher::ResultVariant Searcher::Iterator::Next()
{
    return Searcher::ResultCode::End;
}
