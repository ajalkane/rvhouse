#include <string>
#include <list>

#include <QtGui>

#include "../../app_version.h"
#include "../../icon_store.h"
#include "../../lang/manager.h"
#include "../../lang/info.h"
#include "../../lang/util.h"
#include "about.h"

namespace gui {
namespace window {


about::about(QWidget *parent)
    : QDialog(parent)
{
    this->setWindowTitle(langstr("about_win/title"));
    this->setWindowIcon(app_icons()->get("about"));

    QVBoxLayout *l = new QVBoxLayout;
    l->setSpacing(0);
    l->addWidget(new QLabel(APP_NAME " " APP_VERSION));
    l->addWidget(new QLabel);
    l->addWidget(new QLabel("Copyright (C) 2006-2015 Arto Jalkanen"));
    l->addWidget(new QLabel);
    l->addWidget(new QLabel(langstr("about_win/license", "GPL").c_str()));
    l->addWidget(new QLabel);
    l->addWidget(new QLabel(langstr("about_win/icons_by", "ADX").c_str()));
    // l->addWidget(new QLabel("<br>"));
    l->addWidget(new QLabel(langstr("about_win/house_icon_by", "Pigasque").c_str()));
    l->addWidget(new QLabel);
    l->addWidget(new QLabel(langstr("about_win/other_sites")));
    l->addWidget(new QLabel);

    std::list<const char *> urls;
    urls.push_back("http://revoltzone.net");
    urls.push_back("http://rv12.revoltzone.net");
    urls.push_back("http://rvhouse.revoltzone.net");
    urls.push_back("http://www.revoltrace.net");
    for (std::list<const char *>::iterator i = urls.begin(); i != urls.end(); ++i) {
        std::string ahref = "<a href=\"";
        ahref.append(*i);
        ahref.append("\">");
        ahref.append(*i);
        ahref.append("</a>");
        QLabel *label = new QLabel(ahref.c_str());
        label->setOpenExternalLinks(true);
        l->addWidget(label);
    }

    l->addWidget(new QLabel);

    l->addWidget(new QLabel(langstr("about_win/using")));
    l->addWidget(new QLabel(langstr("about_win/using_lib", "Qt 4.8 GUI Library", "Nokia").c_str()));
    l->addWidget(new QLabel(langstr("about_win/using_lib", "ACE network library", "Douglas C. Schmidth").c_str()));
    l->addWidget(new QLabel(langstr("about_win/using_lib", "KadC P2P library", "Enzo Michelangeli").c_str()));
    l->addWidget(new QLabel(langstr("about_win/using_lib", "libdht, libreudp, libnetcomgrp", "ajalkane").c_str()));
    l->addWidget(new QLabel(langstr("about_win/using_lib", "Boost serialization 1.55", "Robert Ramey").c_str()));

    l->addWidget(new QLabel);

    std::list<lang::info> lang_infos = lang_mngr()->lang_infos();
    // Remove english, as its the original language and not a translation,
    // from the list before listing translations
    std::list<lang::info>::iterator new_end
      = std::remove_if(lang_infos.begin(), lang_infos.end(),
                       lang::info_match_lang_fobj("English"));
    lang_infos.erase(new_end, lang_infos.end());
    if (lang_infos.size() > 0) {
        l->addWidget(new QLabel(langstr("about_win/translations")));
        std::list<lang::info>::const_iterator i = lang_infos.begin();
        for (; i != lang_infos.end(); i++) {
            l->addWidget(new QLabel(langstr("about_win/translation",
                                            i->lang().c_str(),
                                            i->author().c_str(),
                                            i->email().c_str(),
                                            i->version().c_str()).c_str()));
        }
    }

    /**
     * Bottom part
     */
    l->addStretch(1);
    QFrame      *separator = new QFrame;
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    l->addWidget(separator);
    QDialogButtonBox *button_box = new QDialogButtonBox;
    QPushButton *ok_button = new QPushButton(langstr("common/ok_button"), this);
    button_box->addButton(ok_button, QDialogButtonBox::AcceptRole);
    l->addWidget(button_box);

    connect(ok_button, SIGNAL(clicked()), this, SLOT(accept()));

    this->setLayout(l);
}

} // ns window
} // ns gui
