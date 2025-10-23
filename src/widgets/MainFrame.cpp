#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/srchctrl.h>
#include <wx/aboutdlg.h>
#include <atomic>
#include <chrono>
#include <thread>
#include "utils/OpenFile.hpp"
#include "LaunchR.hpp"
#include "ResultListCtrl.hpp"
#include "MainFrame.hpp"

using namespace LR;
typedef std::list<Searcher::IteratorPtr> IteratorList;

wxDEFINE_EVENT(LR_MAINFRAME_UPDATE_STATUSBAR_OBJECT_COUNT, wxCommandEvent);
wxDEFINE_EVENT(LR_MAINFRAME_UPDATE_STATUSBAR_SEARCHING_STATUS, wxCommandEvent);

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
    void OnExit(wxCommandEvent&);
    void OnAbout(wxCommandEvent&);
    void OnItemActivated(wxListEvent&);
    void OnSearchText(wxCommandEvent&);
    void OnSearchKeyDown(wxKeyEvent&);
    void OnUpdateStatusbarObjectCount(wxCommandEvent&);
    void OnUpdateStatusbarSearchingStatus(wxCommandEvent&);

    MainFrame*                 owner;
    wxSearchCtrl*              search_ctrl;
    ResultListCtrl*            result_list;
    std::shared_ptr<QueryTask> query_task;
};

static void UpdateStatusBarSearchingStatus(struct MainFrame* frame, const wxString& text)
{
    wxCommandEvent* e = new wxCommandEvent(LR_MAINFRAME_UPDATE_STATUSBAR_SEARCHING_STATUS);
    e->SetString(text);
    wxQueueEvent(frame, e);
}

static void UpdateStatusBarObjectCount(struct MainFrame* frame, int count)
{
    wxCommandEvent* e = new wxCommandEvent(LR_MAINFRAME_UPDATE_STATUSBAR_OBJECT_COUNT);
    e->SetInt(count);
    wxQueueEvent(frame, e);
}

static void QueryTaskThread(struct QueryTask* task)
{
    IteratorList iterators;
    for (auto& searcher : wxGetApp().searchers)
    {
        iterators.push_back(searcher->Query(task->query));
    }
    UpdateStatusBarSearchingStatus(task->frame->owner, "Searching...");

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
                    UpdateStatusBarObjectCount(task->frame->owner, task->frame->result_list->GetCount());
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

    if (iterators.empty())
    {
        UpdateStatusBarSearchingStatus(task->frame->owner, "");
    }

    task->frame->result_list->UpdateUI();
    UpdateStatusBarObjectCount(task->frame->owner, task->frame->result_list->GetCount());
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

static void CreateMenuBar(MainFrame::Data* data)
{
    wxMenuBar* menu_bar = new wxMenuBar;

    wxMenu* menu_file = new wxMenu;
    menu_file->Append(wxID_EXIT);
    menu_bar->Append(menu_file, L"&File");

    wxMenu* menu_help = new wxMenu;
    menu_help->Append(wxID_ABOUT);
    menu_bar->Append(menu_help, L"&Help");

    data->owner->SetMenuBar(menu_bar);
    data->owner->Bind(wxEVT_MENU, &MainFrame::Data::OnExit, data, wxID_EXIT);
    data->owner->Bind(wxEVT_MENU, &MainFrame::Data::OnAbout, data, wxID_ABOUT);
}

MainFrame::Data::Data(MainFrame* owner)
{
    this->owner = owner;

    /* Menubar. */
    CreateMenuBar(this);

    /* Basis UI */
    wxPanel*    panel = new wxPanel(owner);
    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);

    /* Search box */
    search_ctrl = new wxSearchCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    search_ctrl->ShowSearchButton(true);
    search_ctrl->ShowCancelButton(true);
    search_ctrl->Bind(wxEVT_TEXT, &Data::OnSearchText, this);
    search_ctrl->Bind(wxEVT_TEXT_ENTER, &Data::OnSearchText, this);
    search_ctrl->Bind(wxEVT_KEY_DOWN, &Data::OnSearchKeyDown, this);

    /* Result list */
    result_list = new ResultListCtrl(panel);
    result_list->Bind(wxEVT_LIST_ITEM_ACTIVATED, &Data::OnItemActivated, this);

    /* Layout */
    vbox->Add(search_ctrl, 0, wxEXPAND | wxALL, 8);
    vbox->Add(result_list, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);
    panel->SetSizerAndFit(vbox);
    owner->Centre();

    owner->CreateStatusBar(2);
    owner->Bind(LR_MAINFRAME_UPDATE_STATUSBAR_SEARCHING_STATUS, &Data::OnUpdateStatusbarSearchingStatus, this);
    owner->Bind(LR_MAINFRAME_UPDATE_STATUSBAR_OBJECT_COUNT, &Data::OnUpdateStatusbarObjectCount, this);

    UpdateResults(this, "");
    search_ctrl->SetFocus();
}

MainFrame::Data::~Data()
{
    /* manually stop to avoid multithreaded competition. */
    query_task.reset();
}

void MainFrame::Data::OnExit(wxCommandEvent&)
{
    owner->Close(true);
}

void MainFrame::Data::OnAbout(wxCommandEvent&)
{
    wxAboutDialogInfo info;
    info.SetName("LaunchR");
    info.SetVersion("0.1.1");
    info.SetDescription("Portable and cross-platform launcher and searcher.");
    info.SetCopyright("Copyright (c) 2025 qgymib");
    info.SetWebSite("https://github.com/qgymib/LaunchR");
    wxAboutBox(info);
}

void MainFrame::Data::OnItemActivated(wxListEvent& event)
{
    long index = event.GetIndex();
    if (index >= 0 && index < result_list->GetItemCount())
    {
        wxString path = result_list->GetItemText(index, 1);

        wxLogDebug("Opening file: " + path + "");
        OpenFile(path);
    }
}

void MainFrame::Data::OnSearchText(wxCommandEvent&)
{
    UpdateResults(this, search_ctrl->GetValue());
}

void MainFrame::Data::OnSearchKeyDown(wxKeyEvent& e)
{
    int key = e.GetKeyCode();
    if (key != WXK_DOWN && key != WXK_UP && key != WXK_PAGEUP && key != WXK_PAGEDOWN)
    {
        e.Skip();
        return;
    }

    long count = result_list->GetItemCount();
    if (count <= 0)
    {
        return;
    }

    long selected = result_list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    long newSel = selected;

    if (key == WXK_DOWN)
    {
        if (selected == -1)
            newSel = 0;
        else
        {
            newSel = selected + 1;
            if (newSel >= count)
                newSel = count - 1;
        }
    }
    else if (key == WXK_UP)
    {
        if (selected == -1)
            newSel = count - 1;
        else
        {
            newSel = selected - 1;
            if (newSel < 0)
                newSel = 0;
        }
    }
    else if (key == WXK_PAGEDOWN)
    {
        int page = result_list->GetCountPerPage();
        if (page <= 0)
            page = 10;
        if (selected == -1)
        {
            newSel = page - 1;
            if (newSel >= count)
                newSel = count - 1;
        }
        else
        {
            newSel = selected + page;
            if (newSel >= count)
                newSel = count - 1;
        }
    }
    else /* WXK_PAGEUP */
    {
        int page = result_list->GetCountPerPage();
        if (page <= 0)
            page = 10;
        if (selected == -1)
        {
            newSel = 0;
        }
        else
        {
            newSel = selected - page;
            if (newSel < 0)
                newSel = 0;
        }
    }

    if (selected != -1)
    {
        result_list->SetItemState(selected, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
    }
    result_list->SetItemState(newSel, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED,
                              wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
    result_list->EnsureVisible(newSel);
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

void MainFrame::Data::OnUpdateStatusbarObjectCount(wxCommandEvent& e)
{
    int      count = e.GetInt();
    wxString count_str = format_with_commas(count);
    wxString msg = count_str + " object";
    if (count > 1)
    {
        msg += "s";
    }
    msg += " found";

    owner->SetStatusText(msg, 0);
}

void MainFrame::Data::OnUpdateStatusbarSearchingStatus(wxCommandEvent& e)
{
    owner->SetStatusText(e.GetString(), 1);
}

MainFrame::MainFrame(wxWindow* parent) : wxFrame(parent, wxID_ANY, "LaunchR", wxDefaultPosition, wxSize(600, 420))
{
    m_data = new MainFrame::Data(this);
}

MainFrame::~MainFrame()
{
    delete m_data;
    m_data = nullptr;
}
