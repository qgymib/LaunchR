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

private:
    void OnHello(wxCommandEvent& event);

    void OnExit(wxCommandEvent& event);

    void OnAbout(wxCommandEvent& event);
};

}

#endif
