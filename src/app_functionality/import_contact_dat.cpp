#include <iostream>
#include <fstream>
#include <string>

#include "../common.h"
#include "../config_file.h"
#include "../file_util.h"
#include "../networking/impl/kadc/util.h"

#include "import_contact_dat.h"

namespace app_functionality {

import_contact_dat::import_contact_dat(
    int argc, char **argv,
    const std::string &file_name
) : _import_from(file_name) {
}

import_contact_dat::~import_contact_dat() {
}

void
import_contact_dat::run() {    
    ACE_DEBUG((LM_DEBUG, "Starting import_contact_dat from %s\n", 
               _import_from.c_str()));

    std::string kadcini_file = "kadc.ini";

    ACE_DEBUG((LM_DEBUG, "Starting import_contact_dat %s -> %s\n", 
               _import_from.c_str(), kadcini_file.c_str()));
        
    std::ifstream overnet(_import_from.c_str(),
                          std::ios::in | std::ios::binary);
    std::ofstream kadcini(kadcini_file.c_str());

    if (!overnet.is_open()) throw "Could not open overnet file";
    if (!kadcini.is_open()) throw "Could not open kadc.ini file";
    
    int contacts = 
        networking::kadc::util::overnet_contacts_to_kadc_ini(overnet, kadcini);

    ACE_DEBUG((LM_DEBUG, "Wrote %d contacts\n", contacts));
}

} // ns
