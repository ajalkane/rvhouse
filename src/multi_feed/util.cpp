#include "../model/house.h"
#include "../main.h"
#include "util.h"

namespace multi_feed {
    
std::string header_value(
    const std::string         &hdr,
    const feed_value_map_type &vals
) {
    // Build up the text as: 
    // (Header (specifier1/specifier2): text1/text2) 
    std::string tiv;
    // Header specifiers (blabla/blabla2)
    std::string tih;

    const model::house::group_desc_type &grp_desc = house_model()->group_desc();
      
    model::house::group_desc_type::const_iterator gi = grp_desc.begin();
    for (; gi != grp_desc.end(); gi++) {
        ACE_DEBUG((LM_DEBUG, "finding %d\n", gi->first));
        feed_value_map_type::const_iterator vi = vals.find(gi->first);
        if (vi != vals.end()) {
            ACE_DEBUG((LM_DEBUG, "found: %s\n", vi->second.c_str()));
            tih += (tih.empty() ? gi->second : "/" + gi->second);
            tiv += (tiv.empty() ? vi->second : "/" + vi->second);
        }
    }
    
    return tih.empty() || hdr.empty()
           ? header_value(hdr, tiv)
           : header_value(hdr + " (" + tih + ")", tiv); 
    
}

std::string header_value(
    const std::string &hdr,
    const std::string &value
) {
    if (hdr.empty())
        return value;
    std::string tihv;
    tihv = hdr + ": " + value; //  + "\n";
    return tihv;    
}

} // ns multi_feed
