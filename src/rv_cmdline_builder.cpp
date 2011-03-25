#include "rv_cmdline_builder.h"
#include "main.h"
#include "config_file.h"
#include "app_options.h"
#include "win_registry.h"

rv_cmdline_builder::rv_cmdline_builder() : _is_set_cmdline(false) {
    // First store the original commandline for restoring
    win_registry r(win_registry::id_dplay, "", "Re-Volt");
    _original_cmdline = r.get<std::string>("CommandLine", "");

    // Detect settings and set initial commandline
    _preset_cmdline();
}

rv_cmdline_builder::~rv_cmdline_builder() {
    if (_is_set_cmdline) {
        ACE_DEBUG((LM_DEBUG, "cmdline_builder::~cmdline_builder: Restoring original commandLine to '%s'\n", _original_cmdline.c_str()));
        win_registry r(win_registry::id_dplay, "", "Re-Volt");
        r.set<std::string>("CommandLine", _original_cmdline);
    } else {
        ACE_DEBUG((LM_DEBUG, "cmdline_builder::~cmdline_builder: Original commandLine not altered, not restoring\n"));
    }
}

const char *
rv_cmdline_builder::_option_to_s(const option opt) const {
    switch (opt) {
    case rv12_version_all:
        return "-netcomp 1";
    case rv12_version_12_only:
        return "-netcomp 0";
    case sli:
        return "-sli";

    }
    return NULL;
}

void
rv_cmdline_builder::add_option(const option opt) {
    const char *opt_as_str = _option_to_s(opt);
    if (opt_as_str != NULL) {
        _add_to_cmdline(opt_as_str);
    }
}

void
rv_cmdline_builder::_add_to_cmdline(const char *opt) {
    if (!_cmdline.empty()) {
        _cmdline.append(" ");
    }
    _cmdline.append(opt);
    _is_set_cmdline = true;
}

void
rv_cmdline_builder::_preset_cmdline() {
    std::string cmdline_switch = pref()->get<std::string>("advanced", "cmdline_switch", "cmdline_autoset");
    if (cmdline_switch == "cmdline_autoset") {
        ACE_DEBUG((LM_DEBUG, "cmdline_builder::_set_rv_cmdline: autoset commandline\n"));
        if (os::is_windows_vista_or_later()) {
            ACE_DEBUG((LM_DEBUG, "cmdline_builder::_set_rv_cmdline: Windows Vista or later detected\n"));
            add_option(sli);
        } else {
            _add_to_cmdline("");
        }
    } else if (cmdline_switch == "cmdline_manual") {
        ACE_DEBUG((LM_DEBUG, "cmdline_builder::_set_rv_cmdline: manual commandline\n"));
        _add_to_cmdline(pref()->get<std::string>("advanced", "cmdline", "").c_str());
    }
}

void
rv_cmdline_builder::set_rv_cmdline() {
    if (_is_set_cmdline) {
        ACE_DEBUG((LM_DEBUG, "_set_rv_cmdline::set_rv_cmdline: Setting commandLine to '%s'\n", _cmdline.c_str()));
        win_registry r(win_registry::id_dplay, "", "Re-Volt");
        r.set<std::string>("CommandLine", _cmdline);
    } else {
        ACE_DEBUG((LM_DEBUG, "_set_rv_cmdline::_set_rv_cmdline: commandLine not altered\n"));

    }

}
