#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/srchctrl.h>
#include <algorithm>
#include <vector>
#include "LaunchR.hpp"
#include "MainFrame.hpp"

using namespace LR;

// Data model
struct Item
{
    wxString name;
    wxString desc;
    wxString hotkey;
};

struct MainFrame::Data
{
    Data(MainFrame* owner);
    void OnItemActivated(wxListEvent& event);
    void OnSearchText(wxCommandEvent& event);

    MainFrame* owner;

    wxPanel*      panel;
    wxBoxSizer*   vbox;
    wxSearchCtrl* search_ctrl;
    wxListCtrl*   result_list;
};

/**
 * @brief Update search result.
 * @param[in] data MainFrame.
 * @param[in] query Query string.
 */
static void UpdateResults(MainFrame::Data* data, const wxString& query)
{
    data->result_list->Freeze();
    data->result_list->DeleteAllItems();

    auto it = wxGetApp().searcher->Query(query);
    while (true)
    {
        auto retv = it->Next();
        if (std::holds_alternative<Searcher::Result>(retv))
        {
            auto ret = std::get<Searcher::Result>(retv);
            long row = data->result_list->InsertItem(data->result_list->GetItemCount(), ret.title);
            if (ret.path)
            {
                data->result_list->SetItem(row, 1, ret.path.value());
            }

            continue;
        }

        auto ret = std::get<Searcher::ResultCode>(retv);
        if (ret == Searcher::ResultCode::End)
        {
            break;
        }
    }

    /* Select the first entry by default. */
    if (data->result_list->GetItemCount() > 0)
    {
        data->result_list->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        data->result_list->EnsureVisible(0);
    }

    data->result_list->Thaw();
}

MainFrame::Data::Data(MainFrame* owner)
{
    this->owner = owner;

    /* Basis UI */
    panel = new wxPanel(owner);
    vbox = new wxBoxSizer(wxVERTICAL);

    /* Search box */
    search_ctrl = new wxSearchCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    search_ctrl->ShowSearchButton(true);
    search_ctrl->ShowCancelButton(true);

    /* Result list */
    result_list = new wxListCtrl(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                 wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_NONE);
    result_list->InsertColumn(0, "Name", wxLIST_FORMAT_LEFT, 200);
    result_list->InsertColumn(1, "Path", wxLIST_FORMAT_LEFT, 300);
    result_list->InsertColumn(2, "Shortcut", wxLIST_FORMAT_LEFT, 80);

    /* Layout */
    vbox->Add(search_ctrl, 0, wxEXPAND | wxALL, 8);
    vbox->Add(result_list, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);
    panel->SetSizerAndFit(vbox);
    owner->Centre();

    search_ctrl->Bind(wxEVT_TEXT, &Data::OnSearchText, this);
    search_ctrl->Bind(wxEVT_TEXT_ENTER, &Data::OnSearchText, this);
    result_list->Bind(wxEVT_LIST_ITEM_ACTIVATED, &Data::OnItemActivated, this);

    UpdateResults(this, "");
    search_ctrl->SetFocus();
}

void MainFrame::Data::OnItemActivated(wxListEvent& event)
{
    long index = event.GetIndex();
    if (index >= 0 && index < result_list->GetItemCount())
    {
        wxString name = result_list->GetItemText(index);
        wxMessageBox("Activated: " + name, "Open", wxOK | wxICON_INFORMATION, owner);
    }
}

void MainFrame::Data::OnSearchText(wxCommandEvent&)
{
    UpdateResults(this, search_ctrl->GetValue());
}

MainFrame::MainFrame(wxWindow* parent) : wxFrame(parent, wxID_ANY, "Launcher", wxDefaultPosition, wxSize(600, 420))
{
    m_data = new MainFrame::Data(this);
}

MainFrame::~MainFrame()
{
    delete m_data;
    m_data = nullptr;
}
