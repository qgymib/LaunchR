#include <wx/wx.h>
#include <wx/utils.h>
#include "OpenFile.hpp"

using namespace LR;

bool LR::OpenFile(const wxString& path)
{
#if defined(WIN32)
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    HINSTANCE ret = ShellExecuteW(nullptr, nullptr, path.wc_str(), nullptr, nullptr, SW_SHOWNORMAL);
    return reinterpret_cast<INT_PTR>(ret) > 32;
#else
    return wxLaunchDefaultApplication(path);
#endif
}
