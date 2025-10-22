#ifndef LAUNCHR_WIDGETS_MAINFRAME_HPP
#define LAUNCHR_WIDGETS_MAINFRAME_HPP

#include <wx/wx.h>

namespace LR
{

struct MainFrame : wxFrame
{
    explicit MainFrame(wxWindow* parent);
    ~MainFrame() override;

    struct Data;
    struct Data* m_data;
};

} // namespace LR

#endif
