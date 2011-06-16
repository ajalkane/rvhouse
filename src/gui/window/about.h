#ifndef GUI_WINDOW_ABOUT_H_
#define GUI_WINDOW_ABOUT_H_

#include <QDialog>

#include "../../common.h"

namespace gui {
namespace window {
    
class about : public QDialog {
    Q_OBJECT
        
public: 
    about(QWidget *parent);
};

} // ns window
} // ns gui

#endif //GUI_WINDOW_ABOUT_H_
