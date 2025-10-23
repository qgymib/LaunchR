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

    /**
     * @brief Clear all contents and update UI.
     */
    void Clear();

    /**
     * @brief Append result into table. The UI is not update until UpdateUI() called.
     * @param[in] result Information.
     */
    void Append(const LR::Searcher::Result& result);

    /**
     * @brief Refresh UI.
     */
    void UpdateUI();

    /**
     * @brief Get the number of contents.
     * @return Item number.
     */
    size_t GetCount() const;

    wxString OnGetItemText(long item, long column) const override;
    int      OnGetItemColumnImage(long item, long column) const override;

    struct Data;
    struct Data* m_data;
};

} // namespace LR

#endif
