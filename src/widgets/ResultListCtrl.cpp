#include <wx/wx.h>
#include <mutex>
#include <vector>
#include "ResultListCtrl.hpp"

using namespace LR;

wxDEFINE_EVENT(LR_RESULT_LIST_UPDATE, wxCommandEvent);

struct ResultListCtrl::Data
{
    std::mutex result_mutex;
    ResultVec  results;
};

ResultListCtrl::ResultListCtrl(wxWindow* parent)
    : wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                 wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_NONE | wxLC_VIRTUAL)
{
    m_data = new Data;
    Bind(LR_RESULT_LIST_UPDATE, &ResultListCtrl::OnUpdateUI, this);
}

ResultListCtrl::~ResultListCtrl()
{
    delete m_data;
}

void ResultListCtrl::Clear()
{
    {
        std::lock_guard<std::mutex> lock(m_data->result_mutex);
        m_data->results.clear();
    }

    UpdateUI();
}

void ResultListCtrl::Append(const LR::Searcher::Result& result)
{
    std::lock_guard<std::mutex> lock(m_data->result_mutex);
    m_data->results.push_back(result);
}

void ResultListCtrl::UpdateUI()
{
    wxCommandEvent* e = new wxCommandEvent(LR_RESULT_LIST_UPDATE);
    wxQueueEvent(this, e);
}

wxString ResultListCtrl::OnGetItemText(long item, long column) const
{
    Searcher::Result ret;
    {
        std::lock_guard<std::mutex> lock(m_data->result_mutex);
        if (item >= static_cast<long>(m_data->results.size()))
        {
            return "";
        }
        ret = m_data->results[item];
    }

    switch (column)
    {
    case 0:
        return ret.title;
    case 1:
        return ret.path ? ret.path.value() : "";
    default:
        break;
    }
    return "";
}

void ResultListCtrl::OnUpdateUI(wxCommandEvent&)
{
    long count = 0;
    {
        std::lock_guard<std::mutex> lock(m_data->result_mutex);
        count = m_data->results.size();
    }

    wxListCtrl::SetItemCount(count);
}
