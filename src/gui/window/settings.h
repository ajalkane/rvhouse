#ifndef GUI_WINDOW_SETTINGS_H_
#define GUI_WINDOW_SETTINGS_H_

#include <string>
#include <map>
#include <list>

#include <QMainWindow>
#include <QLineEdit>
#include <QWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QRadioButton>
#include <QTabWidget>
#include <QVBoxLayout>

#include "../../common.h"
#include "../message_handler.h"
#include "size_restoring_window.h"

namespace gui {
namespace window {

class sub_settings : public QWidget {
    Q_OBJECT
public:
    virtual const char *title() = 0;
    virtual ~sub_settings();
public slots:
    virtual void load_settings() = 0;
    virtual void save_settings() = 0;
};

class settings :
    public size_restoring_window<QMainWindow>
{
    Q_OBJECT

    typedef size_restoring_window<QMainWindow> super;
    
    QPushButton *_ok_button;
    QPushButton *_cancel_button;
    QTabWidget  *_section_tab;

    typedef std::list<sub_settings *> _sub_settings_type;
    _sub_settings_type _sub_settings;

    void _create_actions();
    void _create_widgets();
    void _create_layout();
    void _connect_signals();

    void _init();

public:
    settings(QWidget *parent = 0);
    ~settings();

public slots:
    void accept();
    void reject();
};

class settings_general : public sub_settings {
    typedef std::map<std::string, QCheckBox *> _check_map_type;
     _check_map_type _check_map;

public:
    settings_general();
    const char *title();
public:
    void load_settings();
    void save_settings();
};

class settings_advanced : public sub_settings {
    Q_OBJECT

    typedef std::map<std::string, QCheckBox *> _check_map_type;
     _check_map_type _check_map;

    typedef std::map<std::string, QRadioButton *> _cmdline_switch_map_type;
    _cmdline_switch_map_type _cmdline_switch_map;

    QLineEdit *_cmdline_field;

    void _set_cmdline_switch_state(const std::string &cmdline_switch);

public:
    settings_advanced();
    const char *title();
public:
    void load_settings();
    void save_settings();
};

} // ns window
} // ns gui

#endif //GUI_WINDOW_SETTINGS_H_
