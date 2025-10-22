#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/srchctrl.h>
#include <algorithm>
#include <vector>
#include <list>
#include <locale>
#include <iomanip>
#include <sstream>
#include <format>
#include "LaunchR.hpp"
#include "MainFrame.hpp"

using namespace LR;
typedef std::list<Searcher::IteratorPtr> IteratorList;

struct MainFrame::Data : wxTimer
{
    Data(MainFrame* owner);
    void OnItemActivated(wxListEvent&);
    void OnSearchText(wxCommandEvent&);
    void Notify();

    MainFrame*    owner;
    wxPanel*      panel;
    wxBoxSizer*   vbox;
    wxSearchCtrl* search_ctrl;
    wxListCtrl*   result_list;

    wxString     query;
    bool         new_query;
    bool         default_selected;
    IteratorList query_iterators;
};

/**
 * @brief Update search result.
 * @param[in] data MainFrame.
 * @param[in] query Query string.
 */
static void UpdateResults(MainFrame::Data* data, const wxString& query)
{
    /*
     * Stat timer.
     * User query cannot start here, it may block the UI thread.
     *
     * Only start the timer when no pending query.
     */
    if (!data->new_query)
    {
        data->wxTimer::Start(10, false);
    }

    data->query = query;
    data->new_query = true;
}

MainFrame::Data::Data(MainFrame* owner)
{
    this->owner = owner;
    new_query = false;
    default_selected = false;

    /* Basis UI */
    panel = new wxPanel(owner);
    vbox = new wxBoxSizer(wxVERTICAL);

    /* Search box */
    search_ctrl = new wxSearchCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    search_ctrl->ShowSearchButton(true);
    search_ctrl->ShowCancelButton(true);
    search_ctrl->Bind(wxEVT_TEXT, &Data::OnSearchText, this);
    search_ctrl->Bind(wxEVT_TEXT_ENTER, &Data::OnSearchText, this);

    /* Result list */
    result_list = new wxListCtrl(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                 wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_NONE);
    result_list->InsertColumn(0, "Name", wxLIST_FORMAT_LEFT, 200);
    result_list->InsertColumn(1, "Path", wxLIST_FORMAT_LEFT, 300);
    result_list->InsertColumn(2, "Shortcut", wxLIST_FORMAT_LEFT, 80);
    result_list->Bind(wxEVT_LIST_ITEM_ACTIVATED, &Data::OnItemActivated, this);

    /* Layout */
    vbox->Add(search_ctrl, 0, wxEXPAND | wxALL, 8);
    vbox->Add(result_list, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);
    panel->SetSizerAndFit(vbox);
    owner->Centre();

    owner->CreateStatusBar(2);

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

/**
 * @brief Formats the given number with commas as thousand separators.
 * @param[in] number The number to format.
 * @return A string representation of the formatted number with commas.
 */
static std::string format_with_commas(long number)
{
    std::string num_str = std::to_string(number);
    int         insert_position = num_str.length() - 3;
    while (insert_position > 0)
    {
        num_str.insert(insert_position, ",");
        insert_position -= 3;
    }
    return num_str;
}

static void FetchNewResults(MainFrame::Data* data)
{
    data->result_list->Freeze();

    IteratorList::iterator it = data->query_iterators.begin();
    while (it != data->query_iterators.end())
    {
        Searcher::ResultVariant ret_v;
        while (std::holds_alternative<Searcher::Result>(ret_v = (*it)->Next()))
        {
            Searcher::Result ret = std::get<Searcher::Result>(ret_v);
            long             row = data->result_list->InsertItem(data->result_list->GetItemCount(), ret.title);
            if (ret.path)
            {
                data->result_list->SetItem(row, 1, ret.path.value());
            }
        }

        Searcher::ResultCode code = std::get<Searcher::ResultCode>(ret_v);
        if (code == Searcher::ResultCode::End)
        {
            IteratorList::iterator it_tmp = it;
            ++it;
            data->query_iterators.erase(it_tmp);
            continue;
        }

        ++it;
    }

    /* Select the first entry by default. */
    if (!data->default_selected && data->result_list->GetItemCount() > 0)
    {
        data->result_list->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        data->result_list->EnsureVisible(0);
        data->default_selected = true;
    }

    data->result_list->Thaw();

    wxString num = format_with_commas(data->result_list->GetItemCount());
    wxString status_msg = num + " Objects";
    data->owner->SetStatusText(status_msg, 0);
}

void MainFrame::Data::Notify()
{
    /* If there is a new query, restart search. */
    if (new_query)
    {
        query_iterators.clear();
        for (auto& searcher : wxGetApp().searchers)
        {
            query_iterators.push_back(searcher->Query(query));
        }

        new_query = false;
        default_selected = false;

        result_list->Freeze();
        result_list->DeleteAllItems();
        result_list->Thaw();

        owner->SetStatusText("Searching...", 1);
    }

    FetchNewResults(this);

    if (query_iterators.empty())
    {
        owner->SetStatusText("", 1);
        wxTimer::Stop();
    }
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
