#include <iostream>
#include <fstream>
#include <string>

#include "../common.h"
#include "../config_file.h"
#include "../file_util.h"
#include "../networking/impl/kadc/util.h"

#include "export_contact_dat.h"

namespace app_functionality {

export_contact_dat::export_contact_dat(
    int argc, char **argv,
    const std::string &file_name
) : _export_to(file_name) {
}

export_contact_dat::~export_contact_dat() {
}

void
export_contact_dat::run() {    
    ACE_DEBUG((LM_DEBUG, "Starting export_contact_dat to %s\n", 
               _export_to.c_str()));

    std::string kadcini_file = 
        conf()->get_value("net_serverless_kadc/kadc_inifile");

    ACE_DEBUG((LM_DEBUG, "Starting export_contact_dat %s -> %s\n", 
               kadcini_file.c_str(), _export_to.c_str()));
        
    std::ofstream overnet(_export_to.c_str(),
                          std::ios::out | std::ios::binary);
    std::ifstream kadcini(app_rel_path(kadcini_file.c_str()).c_str());

    if (!overnet.is_open()) throw "Could not open overnet file";
    if (!kadcini.is_open()) throw "Could not open kadc.ini file";
    
    int contacts = networking::kadc::util::kadc_ini_to_overnet_contacts(kadcini, overnet);

    ACE_DEBUG((LM_DEBUG, "Wrote %d contacts\n", contacts));
}

} // ns
