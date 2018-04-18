#include <utility>
#include <string>
#include <map>

#include <QtGui>

#include "../../messaging/message_grouped.h"
#include "../../icon_store.h"
#include "../house_app.h"
#include "settings.h"

namespace gui {
namespace window {

namespace {
    const char *general_check_order[] = {
        "flash_main_chat",
        // "flash_room_chat",
        "flash_new_room",
        // "away_minimize"
        "flash_new_user",
        //"flash_new_user_in_room",
        "flash_nick",
        "global_ignore",
        "send_ip"
    };
    bool general_check_defaults[] = {
        false,
        // false,
        false,
        // true,
        false,
        // false,
        false,
        true,
        true
    };

    const char *advanced_check_order[] = {
        "allow_started_race_join"
    };

    bool advanced_check_defaults[] = {
        true
    };

    const char *cmdline_autoset_key = "cmdline_autoset";
    const char *cmdline_dontset_key = "cmdline_dontset";
    const char *cmdline_manual_key  = "cmdline_manual";
    const char *cmdline_pref_switch_default = cmdline_autoset_key;
    const char *cmdline_pref_switch_key = "cmdline_switch";
    const char *cmdline_pref_key = "cmdline";

    const char *rvgl_path_key = "rvgl_path";
    const char *rvgl_cmdline_key = "rvgl_cmdline";
}

sub_settings::~sub_settings() {}

settings::settings(QWidget *parent) : size_restoring_window<QMainWindow>("settings", parent)
{
    ACE_DEBUG((LM_DEBUG, "settings::ctor\n"));
    this->setWindowIcon(app_icons()->get("rv_house"));
    this->setWindowTitle(langstr("settings_win/title"));

    _create_actions();
    _create_widgets();
    _create_layout();

    _connect_signals();

    _init();
}

void
settings::_create_actions() {
}

void
settings::_create_widgets() {
    _sub_settings.push_back(new settings_general);
    _sub_settings.push_back(new settings_advanced);
    _section_tab = new QTabWidget(this);
    for (_sub_settings_type::iterator i = _sub_settings.begin(); i != _sub_settings.end(); ++i) {
        _section_tab->addTab(*i, (*i)->title());
    }

    _ok_button     = new QPushButton(langstr("common/ok_button"), this);
    _cancel_button = new QPushButton(langstr("common/cancel_button"), this);
}

void
settings::_connect_signals() {
    connect(_ok_button,     SIGNAL(clicked()), this, SLOT(accept()));
    connect(_cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
}

void
settings::_create_layout() {
    QVBoxLayout      *l          = new QVBoxLayout;
    QDialogButtonBox *button_box = new QDialogButtonBox;

    _ok_button->setDefault(true);
    button_box->addButton(_ok_button, QDialogButtonBox::AcceptRole);
    button_box->addButton(_cancel_button, QDialogButtonBox::RejectRole);

    l->addWidget(_section_tab);
    l->addWidget(button_box);

    QWidget *centralWidget = new QWidget;
    centralWidget->setLayout(l);
    this->setCentralWidget(centralWidget);
}

void
settings::_init() {
    for (_sub_settings_type::iterator i = _sub_settings.begin(); i != _sub_settings.end(); ++i) {
        (*i)->load_settings();
    }
}

settings::~settings() {
    ACE_DEBUG((LM_DEBUG, "settings::dtor\n"));
}

void
settings::accept() {
    for (_sub_settings_type::iterator i = _sub_settings.begin(); i != _sub_settings.end(); ++i) {
        (*i)->save_settings();
    }

    app_opts.init();

    this->close();
}

void
settings::reject() {
    this->close();
}

/**
 * Settings general
 */

settings_general::settings_general() {
    QVBoxLayout *l          = new QVBoxLayout;

    for (size_t i = 0; i < array_sizeof(general_check_order); i++) {
        const char *key     = general_check_order[i];
        std::string langkey = "settings_win/";
        langkey += key;
        QCheckBox *check = new QCheckBox(langstr(langkey.c_str()));
        _check_map[key] = check;
        l->addWidget(check);
    }

    l->addStretch(1);
    this->setLayout(l);
}

const char *
settings_general::title() {
    return langstr("settings_win/general_tab");
}

void
settings_general::save_settings() {
    for (size_t i = 0; i < array_sizeof(general_check_order); i++) {
        const char *key = general_check_order[i];
        bool        val = _check_map[key]->isChecked();
        pref()->set<bool>("general", key, val);
    }
}

void
settings_general::load_settings() {
    for (size_t i = 0; i < array_sizeof(general_check_order); i++) {
        const char *key = general_check_order[i];
        bool    def_val = general_check_defaults[i];
        bool        val = pref()->get<bool>("general", key, def_val);
        _check_map[key]->setChecked(val);
    }
}

/**
 * Settings advanced
 */
settings_advanced::settings_advanced() {
    QVBoxLayout *l              = new QVBoxLayout;
    QGroupBox   *cmdline_group  = new QGroupBox(langstr("settings_win/cmdline_title"));
    QVBoxLayout *cmdline_layout = new QVBoxLayout;
    ACE_DEBUG((LM_DEBUG, "settings::create cmdline_autoset\n"));

    _cmdline_switch_map[cmdline_autoset_key] = new QRadioButton(langstr("settings_win/cmdline_autoset"));
    _cmdline_switch_map[cmdline_dontset_key] = new QRadioButton(langstr("settings_win/cmdline_dontset"));
    _cmdline_switch_map[cmdline_manual_key]  = new QRadioButton(langstr("settings_win/cmdline_manual"));

    _cmdline_field   = new QLineEdit;
    _cmdline_field->setDisabled(true);

    cmdline_layout->addWidget(_cmdline_switch_map[cmdline_autoset_key]);
    cmdline_layout->addWidget(_cmdline_switch_map[cmdline_dontset_key]);
    cmdline_layout->addWidget(_cmdline_switch_map[cmdline_manual_key]);
    cmdline_layout->addWidget(_cmdline_field);

    cmdline_group->setLayout(cmdline_layout);

    l->addWidget(cmdline_group);

    for (size_t i = 0; i < array_sizeof(advanced_check_order); i++) {
        const char *key     = advanced_check_order[i];
        std::string langkey = "settings_win/";
        langkey += key;
        QCheckBox *check = new QCheckBox(langstr(langkey.c_str()));
        _check_map[key] = check;
        l->addWidget(check);
    }

    // RVGL settings, perhaps move to a new tab?
    QGroupBox   *rvgl_group  = new QGroupBox(langstr("settings_win/rvgl_title"));
    QVBoxLayout *rvgl_layout = new QVBoxLayout;

    _rvgl_path_field    = new QLineEdit;
    _rvgl_path_field->setEnabled(true);
    _rvgl_path_browse   = new QPushButton(langstr("settings_win/rvgl_browse"));
    _rvgl_cmdline_field = new QLineEdit;
    _rvgl_cmdline_field->setEnabled(true);

    rvgl_layout->addWidget(new QLabel(langstr("settings_win/rvgl_path")));
    rvgl_layout->addWidget(_rvgl_path_field);
    rvgl_layout->addWidget(_rvgl_path_browse);
    rvgl_layout->addWidget(new QLabel(langstr("settings_win/rvgl_cmdline")));
    rvgl_layout->addWidget(_rvgl_cmdline_field);

    rvgl_group->setLayout(rvgl_layout);

    l->addWidget(rvgl_group);

    l->addStretch(1);

    this->setLayout(l);

    connect(_cmdline_switch_map[cmdline_manual_key], SIGNAL(toggled(bool)), _cmdline_field, SLOT(setEnabled(bool)));
    connect(_rvgl_path_browse, SIGNAL(clicked()), this, SLOT(getPath()));
}

const char *
settings_advanced::title() {
    return langstr("settings_win/advanced_tab");
}

void
settings_advanced::save_settings() {
    _cmdline_switch_map_type::iterator i = _cmdline_switch_map.begin();
    for (; i != _cmdline_switch_map.end(); i++) {
        if (i->second->isChecked()) {
            ACE_DEBUG((LM_DEBUG, "settings_advanced::save_settings: setting cmdline_pref_switch_key to %s\n", i->first.c_str()));
            pref()->set<std::string>("advanced", cmdline_pref_switch_key, i->first);
        }
    }
    pref()->set<std::string>("advanced", cmdline_pref_key, _cmdline_field->text().toLatin1().constData());

    for (size_t i = 0; i < array_sizeof(advanced_check_order); i++) {
        const char *key = advanced_check_order[i];
        bool        val = _check_map[key]->isChecked();
        pref()->set<bool>("advanced", key, val);
    }

    pref()->set<std::string>("advanced", rvgl_path_key, _rvgl_path_field->text().toLatin1().constData());
    pref()->set<std::string>("advanced", rvgl_cmdline_key, _rvgl_cmdline_field->text().toLatin1().constData());
}

void
settings_advanced::load_settings() {
    std::string cmdline_switch = pref()->get<std::string>("advanced", cmdline_pref_switch_key, cmdline_pref_switch_default);
    _set_cmdline_switch_state(cmdline_switch);

    std::string cmdline = pref()->get<std::string>("advanced", cmdline_pref_key, "");
    _cmdline_field->setText(cmdline.c_str());

    for (size_t i = 0; i < array_sizeof(advanced_check_order); i++) {
        const char *key = advanced_check_order[i];
        bool    def_val = advanced_check_defaults[i];
        bool        val = pref()->get<bool>("advanced", key, def_val);
        _check_map[key]->setChecked(val);
    }

    std::string rvgl_path = pref()->get<std::string>("advanced", rvgl_path_key, "");
    _rvgl_path_field->setText(rvgl_path.c_str());

    std::string rvgl_cmdline = pref()->get<std::string>("advanced", rvgl_cmdline_key, "");
    _rvgl_cmdline_field->setText(rvgl_cmdline.c_str());
}

void
settings_advanced::getPath() {
    QString filter = langstr("settings_win/rvgl_browse_filter");
    filter += " (rvgl.exe rvgl.32 rvgl.64 rvgl)";

    QString path = QFileDialog::getOpenFileName(this, langstr("settings_win/rvgl_browse_desc"), _rvgl_path_field->text(), filter);
    if (path.isEmpty()) return;

#ifdef WIN32
    path.replace('/', '\\');
    int pos = path.lastIndexOf('\\');
#else
    int pos = path.lastIndexOf('/');
#endif
    if (pos != -1) {
        path = path.left(pos);
    }

    _rvgl_path_field->setText(path);
}

void
settings_advanced::_set_cmdline_switch_state(const std::string &cmdline_switch) {
    _cmdline_switch_map_type::iterator i = _cmdline_switch_map.find(cmdline_switch);
    QRadioButton *obj = (i == _cmdline_switch_map.end() ? _cmdline_switch_map[cmdline_pref_switch_default] : i->second);
    obj->setChecked(true);
}

} // ns window
} // ns gui
