#include <fx.h>

#include "launcher_filetype_unix.h"

namespace executable {

launcher_filetype_unix::launcher_filetype_unix() {  
}

launcher_filetype_unix::~launcher_filetype_unix() {
}
    
int
launcher_filetype_unix::start(const std::string &app) {
// Code originally by Rafael de Pelegrini Soares?
    FXString link(app.c_str());
    FXString ext = FXFile::extension(link);
    FXString list;
    if (comparecase(link.section(':',0), "http")==0 ||
        comparecase(link.section(':',0), "ftp")==0 ||
        comparecase(ext, "htm")==0 || comparecase(ext, "html")==0 ||
        comparecase(ext, "php")==0 || comparecase(ext, "asp")==0)
        list = "mozilla-firefox\tmozilla\tnetscape\tkonqueror\tdillo\tlynx";
    else if(comparecase(ext, "pdf")==0)
        list = "acroread\tkghostview\tgpdf\txpdf";
    if(list.length()){
        FXString software;
        FXint index=0;
        FXString path = FXFile::getExecPath();
        
        software = list.section("\t",index);
        while(!software.empty()){
            software = FXFile::search(path, software);
            if(software.length())
                return system( FXString().format( "%s \"%s\" >/dev/null2>&1 & ",
            software.text(),link.text()).text())>0?0:1;
            index++;
            software = list.section("\t",index);
        }
    }
    else if(FXFile::isExecutable(link))
        return system( (link + " >/dev/null 2>&1 & ").text()) > 0 ? 0:1;
    return 0;       
}

} // ns executable


