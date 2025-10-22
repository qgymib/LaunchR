#ifndef LAUNCHR_WIDGETS_RESULT_LIST_CTRL_HPP
#define LAUNCHR_WIDGETS_RESULT_LIST_CTRL_HPP

#include <wx/wx.h>
#include <wx/listctrl.h>

#include "searchers/Searcher.hpp"

namespace LR
{

struct ResultListCtrl : wxListCtrl
{
    typedef std::vector<LR::Searcher::Result> ResultVec;

    ResultListCtrl(wxWindow* parent);
    ~ResultListCtrl() override;
    void Clear();
    void Append(const LR::Searcher::Result& result);
    void UpdateUI();
    void OnUpdateUI(wxCommandEvent&);

    wxString OnGetItemText(long item, long column) const override;

    struct Data;
    struct Data* m_data;
};

} // namespace LR

#endif
