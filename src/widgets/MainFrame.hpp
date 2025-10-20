#ifndef LAUNCHR_WIDGETS_MAINFRAME_HPP
#define LAUNCHR_WIDGETS_MAINFRAME_HPP

#include <wx/wx.h>

namespace LR
{

class MainFrame : public wxFrame
{
public:
    explicit MainFrame(wxWindow* parent);
    ~MainFrame() override;

public:
    struct Data;
    struct Data* m_data;
};

} // namespace LR

#endif
