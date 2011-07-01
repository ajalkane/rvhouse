#include <utility>

#include <QtGui>

#include "../../main.h"
#include "../../messaging/message_room.h"
#include "../../messaging/message_user.h"
#include "../../util.h"
#include "../../validation.h"
#include "../../chat_gaming/room.h"
#include "../../chat_gaming/user.h"
#include "../../model/house.h"
#include "../../model/self.h"
#include "../../win_registry.h"
#include "../../config_file.h"

#include "room_settings.h"

namespace {
    const int max_rv_players = 12;
    const int max_rv12_players = 16;
}
namespace gui {
namespace window {

room_settings::room_settings(QWidget *parent)
    : QDialog(parent)
{
    this->setWindowTitle(langstr("room_settings_win/title"));

    _create_actions();
    _create_widgets();
    _create_layout();

    _from_settings_to_form();

    _connect_signals();

    // Initialize version information
    version_state_changed(0);
}

void
room_settings::_create_actions() {
    ACE_DEBUG((LM_DEBUG, "room_settings::_create_actions\n"));
    _version_group = new QActionGroup(this);
}

void
room_settings::_create_widgets() {
    ACE_DEBUG((LM_DEBUG, "room_settings::_create_widgets\n"));
    _topic_field   = new QLineEdit;
    _pass_field    = new QLineEdit;
    _laps_field    = new QSpinBox;
    _players_field = new QSpinBox;
    _pickups_check = new QCheckBox(langstr("room_settings_win/pickups"));

    _version_check   = new QCheckBox(langstr("room_settings_win/version"));
    _version_all     = new QRadioButton(langstr("room_settings_win/version_all"));
    _version_12_only = new QRadioButton(langstr("room_settings_win/version_12_only"));

    _laps_field->setMinimum(1);
    _laps_field->setMaximum(20);
    _players_field->setMinimum(2);

    _version_check->setToolTip(langstr("room_settings_win/version_tip"));
    _version_all->setToolTip(langstr("room_settings_win/version_all_tip"));
    _version_12_only->setToolTip(langstr("room_settings_win/version_12_tip"));

    _ok_button      = new QPushButton(langstr("common/ok_button"));
    _cancel_button  = new QPushButton(langstr("common/cancel_button"));
}

void
room_settings::_connect_signals() {
    ACE_DEBUG((LM_DEBUG, "room_settings::_connect_signals\n"));

    connect(_version_check,   SIGNAL(stateChanged(int)), this, SLOT(version_state_changed(int)));
    connect(_version_all,     SIGNAL(toggled(bool)), this, SLOT(version_state_changed(bool)));
    connect(_version_12_only, SIGNAL(toggled(bool)), this, SLOT(version_state_changed(bool)));

    connect(_ok_button,     SIGNAL(clicked()), this, SLOT(accept()));
    connect(_cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
}

void
room_settings::_create_layout() {
    ACE_DEBUG((LM_DEBUG, "room_settings::_create_layout\n"));
    QVBoxLayout *l_main     = new QVBoxLayout;
    QFormLayout *l_base     = new QFormLayout;
    QVBoxLayout *l_game     = new QVBoxLayout;
    QFormLayout *l_game_spin= new QFormLayout;
    QVBoxLayout *l_version  = new QVBoxLayout;
    QHBoxLayout *l_game_version = new QHBoxLayout;

    QGroupBox   *g_game     = new QGroupBox;
    QGroupBox   *g_version  = new QGroupBox;

    QDialogButtonBox *button_box = new QDialogButtonBox;

    l_base->addRow(langstr("room_settings_win/topic"),    _topic_field);
    l_base->addRow(langstr("room_settings_win/password"), _pass_field);

    l_game_spin->addRow(langstr("room_settings_win/laps"), _laps_field);
    l_game_spin->addRow(langstr("room_settings_win/players"), _players_field);
    l_game->addWidget(_pickups_check);
    l_game->addLayout(l_game_spin);
    g_game->setLayout(l_game);

    l_version->addWidget(_version_check);
    l_version->addWidget(_version_all);
    l_version->addWidget(_version_12_only);
    g_version->setLayout(l_version);

    l_game_version->addWidget(g_game);
    l_game_version->addWidget(g_version);

    button_box->addButton(_ok_button,     QDialogButtonBox::AcceptRole);
    button_box->addButton(_cancel_button, QDialogButtonBox::RejectRole);

    l_main->addLayout(l_base);
    l_main->addLayout(l_game_version);
    l_main->addWidget(button_box);

    this->setLayout(l_main);

    _topic_field->setFocus();
}


void
room_settings::_room_to_form(const chat_gaming::room &r)
{
    _topic_field->setText(r.topic().c_str());
    _pass_field->setText(r.password().c_str());
    _laps_field->setValue(r.laps());
    _players_field->setValue(r.max_players());
    _pickups_check->setChecked(r.pickups());

    ACE_DEBUG((LM_DEBUG, "room_settings::_room_to_form cars %d\n", r.max_players()));
}

void
room_settings::_form_to_room(chat_gaming::room &r) const
{
    // Some sensible limits
    if (_topic_field->text().toLatin1().size() > 80)
        _topic_field->setText(_topic_field->text().left(80));
    if (_pass_field->text().toLatin1().size() > 32)
        _pass_field->setText(_pass_field->text().left(32));
    
    r.topic(_topic_field->text().trimmed().toLatin1().constData());
    r.password(_pass_field->text().toLatin1().constData());
    r.laps(_laps_field->value());
    r.max_players(_players_field->value());
    r.pickups(_pickups_check->checkState() == Qt::Checked);

    ACE_DEBUG((LM_DEBUG, "room_settings::_form_to_room cars %d\n", r.max_players()));

}

void
room_settings::_registry_to_room(chat_gaming::room &r) const
{
    // RV has NCars registry field, unfortunately in multiplayer
    // RV doesn't read it so using it doesn't matter. Furthermore
    // RV can crash when it's more than 12, which is supported by
    // RV 1.2. The solution is to remember the setting in user prefs.
    r.laps       (game_registry()->get<int>("NLaps", 10));
    r.pickups    (game_registry()->get<int>("Pickups", 0));
    r.max_players(pref()->get<int>("room_settings/cars", 10));

    ACE_DEBUG((LM_DEBUG, "room_settings::_registry_to_room cars %d\n", r.max_players()));

}

void
room_settings::_room_to_registry(const chat_gaming::room &r) const
{
    // RV has NCars registry field, unfortunately in multiplayer
    // RV doesn't read it so using it doesn't matter. Furthermore
    // RV can crash when it's more than 12, which is supported by
    // RV 1.2. The solution is to remember the setting in user prefs.
    game_registry()->set("NLaps",     r.laps());
    game_registry()->set("Pickups",   r.pickups() ? 1 : 0);
    pref()->set("room_settings/cars", r.max_players());

    ACE_DEBUG((LM_DEBUG, "room_settings::_room_to_registry cars %d\n", r.max_players()));
}

// Overridden from QDialog
void
room_settings::accept() {
    ACE_DEBUG((LM_DEBUG, "room_settings::accept\n"));
    ::message *msg = _to_settings_and_form_room_message();
    if (msg) {
        net_messenger()->send_msg(msg);
        super::accept();
    }
}

// Overridden from QDialog
void
room_settings::reject() {
    ACE_DEBUG((LM_DEBUG, "room_settings::reject\n"));
    // Set the user to be in the room he was before the call
    // (this done only when new room)
    if (_room_id_prev != self_model()->hosting_room().id()) {
        self_model()->user().room_id(_room_id_prev);
        self_model()->hosting_room().id(chat_gaming::room::id_type());
        ::message *msg = self_model()->user_as_message();
        net_messenger()->send_msg(msg);
    }

    super::reject();
}

void
room_settings::version_state_changed(int state) {
    version_state_changed();
}

void
room_settings::version_state_changed(bool toggled) {
    version_state_changed();
}

void
room_settings::version_state_changed() {
    ACE_DEBUG((LM_DEBUG, "room_settings::version_state_changed\n"));
    if (!_version_check->isChecked()) {
        // _version_all->setEnabled(false);
        _version_all->setChecked(false);
        _version_all->setEnabled(false);
        _version_all->setCheckable(false);
        _version_all->update();
        _version_12_only->setEnabled(false);
        _version_12_only->setChecked(false);
        _version_12_only->setCheckable(false);
        _version_12_only->update();
        _players_field->setMaximum(max_rv_players);
    } else {
        _version_all->setEnabled(true);
        _version_all->setCheckable(true);
        _version_12_only->setEnabled(true);
        _version_12_only->setCheckable(true);
        if (!_version_all->isChecked() && !_version_12_only->isChecked()) {
            _version_all->setChecked(true);
        }

        if (_version_all->isChecked()) {
            _players_field->setMaximum(max_rv_players);
        } else {
            _players_field->setMaximum(max_rv12_players);
        }

    }
    ACE_DEBUG((LM_DEBUG, "room_settings::/version_state_changed\n"));

}

void
room_settings::_from_settings_to_form() {
    chat_gaming::room &target_room = self_model()->hosting_room();

    _version_check->setChecked(self_model()->room_version());
    _version_all->setChecked(self_model()->room_version_all());
    _version_12_only->setChecked(self_model()->room_version_12_only());

    _registry_to_room(target_room);
    _room_id_prev = target_room.id();
    _room_to_form(target_room);

    if (_room_id_prev.empty()) {
        // New room branch
        target_room.owner_id(self_model()->user().id());
        target_room.generate_id();

        // Set the user to be in this room and send an update
        self_model()->user().room_id(target_room.id());
        self_model()->user_send();
    }
}

::message *
room_settings::_to_settings_and_form_room_message() {
    if (_topic_field->text().trimmed().isEmpty()) {
        QMessageBox msg_box(this);
        msg_box.setWindowTitle(langstr(("room_settings_win/error_title")));
        msg_box.setText(langstr("room_settings_win/topic_missing"));
        msg_box.setIcon(QMessageBox::Critical);
        msg_box.exec();

        return NULL;
    }

    _form_to_room(self_model()->hosting_room());
    _room_to_registry(self_model()->hosting_room());
    
    self_model()->room_version(_version_check->isChecked());
    self_model()->room_version_all(_version_all->isChecked());
    self_model()->room_version_12_only(_version_12_only->isChecked());

    return self_model()->hosting_room_as_message();
}

} // ns window
} // ns gui
