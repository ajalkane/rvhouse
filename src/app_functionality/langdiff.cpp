#include <iostream>
#include <utility>
#include <functional>
#include <string>
#include <list>
#include <vector>

#include <fx.h>

#include "../common.h"
#include "../lang/manager.h"
#include "../lang/mapper.h"
#include "../file_util.h"
#include "../gui/house_app.h"

#include "langdiff.h"

namespace app_functionality {

inline
const char *pair_first(const std::pair<const char *, const char *> &p) {
    return p.first;
}

langdiff::langdiff(
    int argc, char **argv,
    const std::string &lang1, 
    const std::string &lang2
) /*: house(argc, argv) */ : _lang1(lang1), _lang2(lang2) {

    _app = new FXApp;
    _app->init(argc, argv);
}

langdiff::~langdiff() {
    delete _app;
}

void
langdiff::run() {
    using namespace std;
    
    ACE_DEBUG((LM_DEBUG, "Starting langdiff for %s/%s\n", 
               _lang1.c_str(), _lang2.c_str()));

    lang::manager mngr;
    lang::mapper  lang1;
    lang::mapper  lang2;
    
    mngr.init(app_rel_path("lang"));
    mngr.mapper_init(lang1, _lang1, "");
    mngr.mapper_init(lang2, _lang2, "");
            
    _out << "Comparing " << _lang1 << " and " << _lang2 << endl << endl;
    
    const lang::mapper::map_type &lang1_map = lang1.get_map();
    const lang::mapper::map_type &lang2_map = lang2.get_map();
    
    // Create sorted vectors so that set_difference can be made
    vector<string>    keys1_sorted;
    vector<string>    keys2_sorted;
    list<string>      keys1_uniq;
    list<string>      keys2_uniq;
    list<string>::iterator i;

    insert_iterator<list<string> >
         keys1_uniq_ins(keys1_uniq, keys1_uniq.begin()),
         keys2_uniq_ins(keys2_uniq, keys2_uniq.begin());

    insert_iterator<vector<string> >
         keys1_sorted_ins(keys1_sorted, keys1_sorted.begin()),
         keys2_sorted_ins(keys2_sorted, keys2_sorted.begin());

    transform(lang1_map.begin(), lang1_map.end(), keys1_sorted_ins, 
              pair_first);
    transform(lang2_map.begin(), lang2_map.end(), keys2_sorted_ins, 
              pair_first);
              
    sort(keys1_sorted.begin(), keys1_sorted.end());
    sort(keys2_sorted.begin(), keys2_sorted.end());

    set_difference(keys1_sorted.begin(), 
                   keys1_sorted.end(), 
                   keys2_sorted.begin(),
                   keys2_sorted.end(),
                   keys1_uniq_ins);
    set_difference(keys2_sorted.begin(), 
                   keys2_sorted.end(), 
                   keys1_sorted.begin(),
                   keys1_sorted.end(),
                   keys2_uniq_ins);
    
    if (keys1_uniq.size() == 0 && keys2_uniq.size() == 0) {
        _out << "No difference found!" << endl;
        return;
    }
    
    _print_uniq(keys1_uniq, _lang1);
    _print_uniq(keys2_uniq, _lang2);

    _app->create(); 
    FXMessageBox::information(
        _app, FX::MBOX_OK, "Language diff", 
        _out.str().c_str()
    );
}

void
langdiff::_print_uniq(
    const std::list<std::string> &l, 
    const std::string &lang
)
{
    if (l.size() == 0) return;
    _out << "Keys that are only in language " << lang << ":" << std::endl;
    std::list<std::string>::const_iterator i = l.begin();
    for (; i != l.end(); i++) {
        _out << *i << std::endl;
    }
    _out << std::endl;
}

} // ns
