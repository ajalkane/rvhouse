#include <stdlib.h>

#include <string>
#include <vector>
#include <sstream>

#include "regexp.h"
#include "app_version.h"

static void app_version_to_arr(int v[], const char *s) {
    std::vector<std::string> res(4);

    ACE_DEBUG((LM_DEBUG, "app_version_to_arr: %s\n", s));
    
    if (regexp::match("([0-9]+)\\.([0-9]+)(\\.([0-9]+))?", s, res.begin())) {
        ACE_DEBUG((LM_DEBUG, "app_version_to_arr: "
        "matched major %s, minor %s, patch %s\n",
        res[0].c_str(), res[1].c_str(), res[3].c_str()));

        if (!res[0].empty()) v[0] = atoi(res[0].c_str());
        if (!res[1].empty()) v[1] = atoi(res[1].c_str());
        if (!res[3].empty()) v[2] = atoi(res[3].c_str());

        ACE_DEBUG((LM_DEBUG, "app_version_to_arr: "
        "result: %d.%d.%d\n", v[0], v[1], v[2]));       
    } else {
        ACE_ERROR((LM_ERROR, "app_version_to_arr: "
        "invalid version number: %s\n", s));
    }
}

int app_version_compare(const char *a, const char *b) {
    // Version numbers are of the form:
    // major.minor[.patch]
    // for example: 0.52.2
    int av[3] = {0,0,0};
    int bv[3] = {0,0,0};

    app_version_to_arr(av, a);
    app_version_to_arr(bv, b);
    
    // To make comparing easy, allocate each patch level
    // 1 byte from a 4 byte (= 255 biggest allowed number
    // for the parts)
    unsigned int an = (av[0] << 16) | (av[1] << 8) | (av[2]);
    unsigned int bn = (bv[0] << 16) | (bv[1] << 8) | (bv[2]);
    
    ACE_DEBUG((LM_DEBUG, "app_version_compare: comparing "
              "%d.%d.%d <-> %d.%d.%d (%d <-> %d)\n",
              av[0], av[1], av[2], bv[0], bv[1], bv[2], an, bn));
              
    return an - bn;
}
