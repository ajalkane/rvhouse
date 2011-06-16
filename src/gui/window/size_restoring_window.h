#ifndef SIZE_RESTORING_WINDOW_H_
#define SIZE_RESTORING_WINDOW_H_

#include <ace/Log_Msg.h>

#include <string>

#include <QWidget>
#include <QString>
#include <QByteArray>
#include <QCloseEvent>

#include "../../main.h"
#include "../../config_file.h"
#include "../watched_object.h"

namespace gui {
namespace window {

/**
 * Implements a window that will remember it's size (across re-runs also). Is automatically also a RV House
 * watched object (which can receive RV House messages).
 */
template <class T>
class size_restoring_window : public T, public watched_object
{
    std::string conf_section;

public:
    size_restoring_window(const std::string &conf_section, QWidget *parent = 0, Qt::WindowFlags flags = 0)
        : T(parent, flags), watched_object(this)
    {
        ACE_DEBUG((LM_DEBUG, "size_restoring_window: from section %s, parent %d\n", conf_section.c_str(), parent));
        this->conf_section = conf_section;
    }
    
protected:
    // Qt overloads
    void closeEvent(QCloseEvent *event) {
        ACE_DEBUG((LM_DEBUG, "size_restoring_window::closeEvent %d\n", event->type()));
        pref()->set(conf_section.c_str(), "geometry",    T::saveGeometry().toBase64().constData());
        pref()->set(conf_section.c_str(), "windowState", T::saveState().toBase64().constData());
        this->_object_destroyed();
        T::closeEvent(event);
    }

public slots:
    void show() {
        std::string geometry = pref()->get(conf_section.c_str(), "geometry", std::string());
        std::string state    = pref()->get(conf_section.c_str(), "windowState", std::string());

        ACE_DEBUG((LM_DEBUG, "size_restoring_window: geometry %s, size %d\n", geometry.c_str(), geometry.size()));
        ACE_DEBUG((LM_DEBUG, "size_restoring_window: state %s, size %d\n", state.c_str(), state.size()));

        if (geometry.size() > 0) T::restoreGeometry(QByteArray::fromBase64(geometry.c_str()));
        if (state.size()    > 0) T::restoreState(QByteArray::fromBase64(state.c_str()));

        ACE_DEBUG((LM_DEBUG, "size_restoring_window: calling parent show\n"));

        T::show();
    }
};

} // ns window
} // ns gui

#endif //SIZE_RESTORING_WINDOW_H_
