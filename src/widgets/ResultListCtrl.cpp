#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/mimetype.h>
#include <mutex>
#include <vector>
#include <variant>
#include <map>
#include <thread>
#include <semaphore>
#include "ResultListCtrl.hpp"

using namespace LR;

typedef std::map<std::wstring, int> IconMap;

wxDEFINE_EVENT(LR_RESULT_LIST_UPDATE, wxCommandEvent);

struct ResultListCtrl::Data
{
    Data(ResultListCtrl* owner);
    void OnUpdateUI(wxCommandEvent&);

    ResultListCtrl* owner;
    int             icon_width = 16;
    int             icon_height = 16;

    std::mutex result_mutex; /* Mutex for content list. */
    ResultVec  results;      /* Content list. */

    wxImageList* icon_list; /* Image list for icons, working in UI thread. */
    IconMap      icon_map;  /* File icon and index. Key=ext, value=index. */
};

ResultListCtrl::Data::Data(ResultListCtrl* owner)
{
    this->owner = owner;
}

static wxIcon GetSystemIconForExtension(const wxString& ext)
{
    std::unique_ptr<wxFileType> fileType(wxTheMimeTypesManager->GetFileTypeFromExtension(ext));
    if (fileType == nullptr)
    {
        return wxNullIcon;
    }

    wxIconLocation iconPath;
    if (!fileType->GetIcon(&iconPath))
    {
        return wxNullIcon;
    }
    return wxIcon(iconPath);
}

static wxIcon ResizeIcon(const wxIcon& icon, int width, int height)
{
    wxBitmap bmp;
    bmp.CopyFromIcon(icon);

    wxImage img = bmp.ConvertToImage();
    img.Rescale(width, height);

    wxBitmap scaledBmp(img);

    wxIcon newIcon;
    newIcon.CopyFromBitmap(scaledBmp);

    return newIcon;
}

ResultListCtrl::ResultListCtrl(wxWindow* parent)
    : wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                 wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_NONE | wxLC_VIRTUAL)
{
    m_data = new Data(this);

    m_data->icon_list = new wxImageList(m_data->icon_width, m_data->icon_height);
    wxListCtrl::AssignImageList(m_data->icon_list, wxIMAGE_LIST_SMALL);

    Bind(LR_RESULT_LIST_UPDATE, &Data::OnUpdateUI, m_data);
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

size_t ResultListCtrl::GetCount() const
{
    std::lock_guard<std::mutex> lock(m_data->result_mutex);
    return m_data->results.size();
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

int ResultListCtrl::OnGetItemColumnImage(long item, long column) const
{
    if (column != 0)
    {
        return -1;
    }

    Searcher::Result ret;
    {
        std::lock_guard<std::mutex> lock(m_data->result_mutex);
        if (item >= static_cast<long>(m_data->results.size()))
        {
            return -1;
        }
        ret = m_data->results[item];
    }

    if (!ret.path)
    {
        return -1;
    }
    wxString path = ret.path.value();
    wxString ext;
    wxFileName::SplitPath(path, nullptr, nullptr, &ext, wxPATH_NATIVE);
    if (ext.empty())
    {
        return -1;
    }

    /* Search for icon in image list. */
    std::wstring      wext = ext.ToStdWstring();
    IconMap::iterator it = m_data->icon_map.find(wext);
    if (it != m_data->icon_map.end())
    {
        return it->second;
    }

    int    idx = -1;
    wxIcon icon = GetSystemIconForExtension(ext);
    if (icon.IsOk())
    {
        icon = ResizeIcon(icon, m_data->icon_width, m_data->icon_height);
        idx = m_data->icon_list->Add(icon);
    }
    m_data->icon_map.insert(IconMap::value_type(wext, idx));
    return idx;
}

void ResultListCtrl::Data::OnUpdateUI(wxCommandEvent&)
{
    long count = 0;
    {
        std::lock_guard<std::mutex> lock(result_mutex);
        count = results.size();
    }

    owner->wxListCtrl::SetItemCount(count);
}
