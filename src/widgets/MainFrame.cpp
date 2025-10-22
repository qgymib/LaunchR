#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/srchctrl.h>
#include <wx/utils.h>
#include <atomic>
#include <algorithm>
#include <vector>
#include <list>
#include <iomanip>
#include <sstream>
#include <format>
#include <chrono>
#include "LaunchR.hpp"
#include "ResultListCtrl.hpp"
#include "MainFrame.hpp"

#include <thread>

using namespace LR;
typedef std::list<Searcher::IteratorPtr> IteratorList;

struct QueryTask
{
    QueryTask(MainFrame::Data* frame, const wxString& query);
    ~QueryTask();

    MainFrame::Data*  frame;
    wxString          query;
    std::atomic<bool> flag_running = true;
    std::thread       thread;
};

struct MainFrame::Data
{
    Data(MainFrame* owner);
    ~Data();
    void OnItemActivated(wxListEvent&);
    void OnSearchText(wxCommandEvent&);

    MainFrame*                 owner;
    wxPanel*                   panel;
    wxBoxSizer*                vbox;
    wxSearchCtrl*              search_ctrl;
    ResultListCtrl*            result_list;
    std::shared_ptr<QueryTask> query_task;
};

static void QueryTaskThread(struct QueryTask* task)
{
    IteratorList iterators;
    for (auto& searcher : wxGetApp().searchers)
    {
        iterators.push_back(searcher->Query(task->query));
    }

    auto start_time = std::chrono::steady_clock::now();
    while (task->flag_running && !iterators.empty())
    {
        size_t                 append_count = 0;
        IteratorList::iterator it = iterators.begin();
        while (it != iterators.end() && task->flag_running)
        {
            Searcher::ResultVariant ret_v;
            while (std::holds_alternative<Searcher::Result>(ret_v = (*it)->Next()))
            {
                Searcher::Result ret = std::get<Searcher::Result>(ret_v);
                task->frame->result_list->Append(ret);
                append_count++;

                auto now_time = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now_time - start_time);
                if (duration.count() > 100)
                {
                    task->frame->result_list->UpdateUI();
                    start_time = now_time;
                }
            }

            Searcher::ResultCode code = std::get<Searcher::ResultCode>(ret_v);
            if (code == Searcher::ResultCode::End)
            {
                IteratorList::iterator it_tmp = it;
                ++it;
                iterators.erase(it_tmp);
                continue;
            }

            ++it;
        }

        if (append_count == 0)
        {
            wxThread::Sleep(10);
        }
    }

    task->frame->result_list->UpdateUI();
}

QueryTask::QueryTask(MainFrame::Data* frame, const wxString& query)
{
    this->query = query;
    this->frame = frame;
    thread = std::thread(QueryTaskThread, this);
}

QueryTask::~QueryTask()
{
    flag_running = false;
    thread.join();
}

/**
 * @brief Update search result.
 * @param[in] data MainFrame.
 * @param[in] query Query string.
 */
static void UpdateResults(MainFrame::Data* data, const wxString& query)
{
    /* Stop the previous query. */
    data->query_task.reset();

    /* Clear results. */
    data->result_list->Clear();

    /* Start a new query. */
    data->query_task = std::make_shared<QueryTask>(data, query);
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
    search_ctrl->Bind(wxEVT_TEXT, &Data::OnSearchText, this);
    search_ctrl->Bind(wxEVT_TEXT_ENTER, &Data::OnSearchText, this);

    /* Result list */
    result_list = new ResultListCtrl(panel);
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

MainFrame::Data::~Data()
{
    /* manually stop to avoid multithreaded competition. */
    query_task.reset();
}

void MainFrame::Data::OnItemActivated(wxListEvent& event)
{
    long index = event.GetIndex();
    if (index >= 0 && index < result_list->GetItemCount())
    {
        wxString path = result_list->GetItemText(index, 1);

        wxLogDebug("Opening file: " + path + "");
        if (!wxLaunchDefaultApplication(path))
        {
            wxLogWarning("Cannot open file: " + path + "");
        }
    }
}

void MainFrame::Data::OnSearchText(wxCommandEvent&)
{
    UpdateResults(this, search_ctrl->GetValue());
}

#if 0
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
#endif

MainFrame::MainFrame(wxWindow* parent) : wxFrame(parent, wxID_ANY, "Launcher", wxDefaultPosition, wxSize(600, 420))
{
    m_data = new MainFrame::Data(this);
}

MainFrame::~MainFrame()
{
    delete m_data;
    m_data = nullptr;
}
