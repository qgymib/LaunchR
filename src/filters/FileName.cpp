#include <wx/wx.h>
#include <wx/filefn.h>
#include "FileName.hpp"

using namespace LR;

struct FileNameFilter::Data
{
    Data(const wxString& query, Callback cb);
    wxString query;
    wxString query_lower;
    Callback cb;
    bool     isWildcard;
};

FileNameFilter::Data::Data(const wxString& query, Callback cb)
{
    this->query = query;
    this->cb = cb;
    isWildcard = wxIsWild(query);
    query_lower = query.Lower();
}

FileNameFilter::FileNameFilter(const wxString& query, Callback cb) : Filter(query, cb)
{
    m_data = new Data(query, cb);
}

FileNameFilter::~FileNameFilter()
{
    delete m_data;
}

void FileNameFilter::Handle(MsgPtr msg)
{
    if (std::holds_alternative<Filter::Cmd>(*msg))
    {
        m_data->cb(msg);
        return;
    }

    const Filter::Record& record = std::get<Filter::Record>(*msg);
    if (m_data->isWildcard)
    {
        if (!wxMatchWild(m_data->query, record.Name, false))
        {
            return;
        }
    }
    else
    {
        const wxString lowerName = record.Name.Lower();
        if (!lowerName.Contains(m_data->query_lower))
        {
            return;
        }
    }

    /* Bypass. */
    m_data->cb(msg);
}
