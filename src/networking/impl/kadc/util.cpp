#include <iomanip>

#include "util.h"

namespace networking {
namespace kadc {
namespace util {

using namespace std;

class binary_ip {
    unsigned int _binip;
public:
    inline binary_ip(unsigned int binip) : _binip(binip) {}
    friend ostream &operator<<(ostream &out, const binary_ip &bip);
};

// This function modified from KadC's contact_dat.c
ostream &operator<<(ostream &out, const binary_ip &bip) {
    unsigned char *b = (unsigned char *)&bip._binip;
    out << (int)b[3] << '.'
    << (int)b[2] << '.'
    << (int)b[1] << '.'
    << (int)b[0];
    return out;
}

ostream &operator<<(ostream &out, const kadc_pars &p) {
    out << "[local]" << endl;
    out << "0 0.0.0.0 "
    << p.udp_port << ' '
    << p.tcp_port << ' '
    << '0' << endl;
    return out;
}

// This function from KadC's contact_dat.c
static unsigned long int changelongendianity(unsigned long int ul) {
    unsigned char uc;
    unsigned char *puc1 = (unsigned char *)&ul;
    unsigned char *puc2 = (unsigned char *)&ul+3;
    uc = *puc1;
    *puc1++ = *puc2;
    *puc2-- = uc;
    uc = *puc1;
    *puc1++ = *puc2;
    *puc2-- = uc;
    return ul;
}

// This function modified from KadC's contact_dat.c
int	overnet_contacts_to_kadc_ini(std::istream &inp,
                                 std::ostream &out,
                                 kadc_pars p)
{
    int num_contacts = 0;
    unsigned int id[4];
    unsigned int binip;
    unsigned short int udpport;
    // unsigned short int tcpport;
    unsigned char type;

    istream::iostate iexcflags = inp.exceptions();
    istream::iostate oexcflags = out.exceptions();

    inp.exceptions(istream::eofbit | istream::failbit | istream::badbit);
    out.exceptions(istream::eofbit | istream::failbit | istream::badbit);

    int i = 0;

    try {
        inp.read((char *)&num_contacts, sizeof(num_contacts));

        ACE_DEBUG((LM_DEBUG, "read %d contacts\n", num_contacts));

        if (num_contacts == 0) {
            ACE_DEBUG((LM_DEBUG, "no contacts in contacts stream"));
        } else {
            out << p;
            out << "[overnet_peers]" << endl;
        }

        for (i = 0; i < num_contacts; i++) {
            inp.read((char *)id,       sizeof(id));
            inp.read((char *)&binip,   sizeof(binip));
            inp.read((char *)&udpport, sizeof(udpport));
            inp.read((char *)&type,    sizeof(type));

            binip = changelongendianity(binip);
            /* also MD4 in nodes.dat are little endian */
            out << hex << setfill('0');
            for (int j = 0; j < 4; j++) out << setw(8) << id[j]; //printf("%08x", id[j]);
            out << dec << setfill(' ') << ' ';
            out << binary_ip(binip) << ' '
            << udpport          << ' '
            << (int)type        << endl;
        }
    } catch (const std::exception &e) {
        ACE_DEBUG((LM_ERROR, "IO exception: %s\n", e.what()));
        // Restore exceptions flags
        /* inp.exceptions(iexcflags);
        out.exceptions(iexcflags); */
    }

    // Restore exceptions flags
    inp.exceptions(iexcflags);
    out.exceptions(oexcflags);

    return i;
}

} // ns util
} // ns kadc
} // ns networking

