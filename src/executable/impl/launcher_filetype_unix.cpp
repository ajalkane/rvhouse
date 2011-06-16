#include <QDesktopServices>
#include <QUrl>

#include "launcher_filetype_unix.h"

namespace executable {

launcher_filetype_unix::launcher_filetype_unix() {  
}

launcher_filetype_unix::~launcher_filetype_unix() {
}
    
int
launcher_filetype_unix::start(const std::string &app) {
    // IMPROVE Qt: launcher_filetype_win32/unix variants can probably be gotten rid of, can use the same code in both
    QDesktopServices::openUrl(QUrl(app.c_str()));
    return 0;
}

} // ns executable


