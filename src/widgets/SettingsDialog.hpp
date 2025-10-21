#ifndef LAUNCHR_WIDGETS_SETTINGS_DIALOG_HPP
#define LAUNCHR_WIDGETS_SETTINGS_DIALOG_HPP

#include <wx/wx.h>

namespace LR
{

class SettingsDialog : public wxDialog
{
public:
    explicit SettingsDialog(wxWindow* parent);

public:
    struct Data;
    struct Data* m_data;
};

}

#endif
