#ifndef RV_CMDLINE_BUILDER_H_
#define RV_CMDLINE_BUILDER_H_

#include <string>

class rv_cmdline_builder {
public:
    enum option {
        rv12_version_all,
        rv12_version_12_only,
        sli
    };
    rv_cmdline_builder();
    ~rv_cmdline_builder();
    void add_option(const option opt);
    // Sets the commandline that was build into Re-Volt DPlay registry
    void set_rv_cmdline();

private:
    bool _is_set_cmdline;
    std::string _cmdline;
    std::string _original_cmdline;
    const char *_option_to_s(const option opt) const;
    void _add_to_cmdline(const char *opt);
    void _preset_cmdline();
};

#endif //RV_CMDLINE_BUILDER_H_
