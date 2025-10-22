#ifndef LAUNCHR_UTILS_OPENFILE_HPP
#define LAUNCHR_UTILS_OPENFILE_HPP

#include <wx/string.h>

namespace LR
{

/**
 * Opens a file or launches its associated default application depending on the
 * given file path. The behavior is platform-dependent.
 *
 * @param[in] path The file path to be opened or launched.
 * @return Returns true if the operation succeeds.
 */
bool OpenFile(const wxString& path);

} // namespace LR

#endif
