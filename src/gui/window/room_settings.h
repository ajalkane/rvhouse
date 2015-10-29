#ifndef ROOM_SETTINGS_WINDOW_H_
#define ROOM_SETTINGS_WINDOW_H_

#include <string>

#include <QDialog>
#include <QLineEdit>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QPushButton>
#include <QRadioButton>
#include <QString>
#include <QSpinBox>
#include <QCheckBox>
#include <QWidget>

#include "../../common.h"
#include "../../messaging/messenger.h"
#include "../../messaging/message.h"
#include "../../messaging/message_string.h"
#include "../../chat_gaming/room.h"

#include "../message_handler.h"

namespace gui {
namespace window {

class room_settings : public QDialog {
    Q_OBJECT

    typedef QDialog super;

    QLineEdit *_topic_field;
    QLineEdit *_pass_field;
    QSpinBox  *_laps_field;
    QSpinBox  *_players_field;
    QCheckBox *_pickups_check;
    QCheckBox *_version_check;
    QActionGroup *_version_group;
    QRadioButton *_version_all;
    QRadioButton *_version_12_only;
    QRadioButton *_version_rvgl;
    QPushButton  *_ok_button;
    QPushButton  *_cancel_button;

    chat_gaming::room::id_type _room_id_prev;
    
    void _create_actions();
    void _create_widgets();
    void _create_layout();
    void _connect_signals();

    void _room_to_form(const chat_gaming::room &r);
    void _form_to_room(chat_gaming::room &r) const;
    void _registry_to_room(chat_gaming::room &r) const;
    void _room_to_registry(const chat_gaming::room &r) const;

    ::message *_to_settings_and_form_room_message();
    void _from_settings_to_form();

public:
    room_settings(QWidget *parent);
        
public slots:
    // IMPROVE: I'm pretty sure there's a better way to do this instead of having
    // three functions.
    void version_state_changed();
    void version_state_changed(bool toggled);
    void version_state_changed(int state);

    // Overrides of QDialog
    virtual void accept();
    virtual void reject();

};

} // ns window
} // ns gui

#endif //ROOM_SETTINGS_WINDOW_H_
