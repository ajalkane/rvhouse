#include <iomanip>
#include <sstream>

#include "../../../exception.h"

#include "util.h"

namespace networking {
namespace kadc {
namespace util {

using namespace std;

class binary_ip {
    unsigned int _binip;
public:
    inline binary_ip(unsigned int binip) : _binip(binip) {}
    inline binary_ip() : _binip(0) {}
    friend ostream &operator<<(ostream &out, const binary_ip &bip);
    friend istream &operator>>(istream &inp, binary_ip &bip);
    
    inline const unsigned int &raw() const { return _binip; }
    inline size_t raw_size() const { return sizeof(_binip); }
};

class assert_char {
    char _what;
public:
    inline assert_char(char w) : _what(w) {}
    friend ostream &operator<<(ostream &out, const assert_char &a);
    friend istream &operator>>(istream &inp, assert_char &a);
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

istream &operator>>(istream &inp, binary_ip &bip) {
    unsigned char *b = (unsigned char *)&bip._binip;
    unsigned int a[4];
    assert_char assert_dot('.');
    
    inp >> a[0] >> assert_dot
        >> a[1] >> assert_dot
        >> a[2] >> assert_dot
        >> a[3];
    
    b[3] = a[0];
    b[2] = a[1];
    b[1] = a[2];
    b[0] = a[3];

    return inp;
}


ostream &operator<<(ostream &out, const assert_char &ac) {
    out << ac._what;
    return out;
}

istream &operator>>(istream &inp, assert_char &ac) {
    char c;
    inp.read(&c, 1);
    if (c != ac._what) {
        ACE_ERROR((LM_ERROR, "assert_char: input '%c' expected '%c'\n",
                  c, ac._what));
        throw exceptionf(0, "assert_char: input '%c' expected '%c'\n",
                  c, ac._what);
    }
    return inp;
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
static unsigned int changelongendianity(unsigned int ul) {
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
int overnet_contacts_to_kadc_ini(std::istream &inp, 
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
    } catch (const std::ios_base::failure &e) {
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

int kadc_ini_to_overnet_contacts(std::istream &inp,
                                 std::ostream &out)
{
    istream::iostate iexcflags = inp.exceptions();
    istream::iostate oexcflags = out.exceptions();
    
    inp.exceptions(istream::eofbit | istream::failbit | istream::badbit);
    out.exceptions(istream::eofbit | istream::failbit | istream::badbit);
    
    int i = 0;
    bool contact_section = false;
    
    try {
        // Write a placeholder for number of contacts
        out.write((char *)&i, sizeof(i));
        while (!inp.eof()) {
            string line;
            getline(inp, line);
        
            ACE_DEBUG((LM_DEBUG, "read line %s\n", line.c_str()));
            
            if (line[0] == '#') continue;
            
            if (!contact_section && 
                line == "[overnet_peers]")
            {
                ACE_DEBUG((LM_DEBUG, "Contacts section found\n"));
                contact_section = true;
            } else if (contact_section && line[0] == '[') {
                ACE_DEBUG((LM_DEBUG, "Contacts section end\n"));
                contact_section = false;
            } else if (contact_section) {
                istringstream inps(line);
                unsigned int id[4];
                unsigned short int udpport;
                unsigned char type;
                int type_int;
                binary_ip binip;
                assert_char assert_space(' ');                
                // inps >> hex;
                for (int j = 0; j < 4; j++) {
                    char buf[9];
                    inps.read(buf, 8);
                    buf[8] = 0;
                    ACE_DEBUG((LM_DEBUG, "read raw %s\n", buf));                    
                    istringstream hashpart(buf);
                    hashpart >> hex;
                    hashpart >> id[j]; 
                    ACE_DEBUG((LM_DEBUG, "read %08x\n", id[j]));
                }
                ACE_DEBUG((LM_DEBUG, "read2\n"));
                inps >> dec >> assert_space;
                ACE_DEBUG((LM_DEBUG, "read3\n"));
                inps >> binip   >> assert_space;
                ACE_DEBUG((LM_DEBUG, "read4\n"));
                inps >> udpport >> assert_space;
                ACE_DEBUG((LM_DEBUG, "read5\n"));
                inps >> type_int;
                type = type_int;
                
                ACE_DEBUG((LM_DEBUG, "write\n"));

                unsigned int binip2 = changelongendianity(binip.raw());
                out.write((char *)id,          sizeof(id));
                out.write((char *)&binip2,     sizeof(binip2));
                out.write((char *)&udpport,    sizeof(udpport));
                out.write((char *)&type,       sizeof(type));
                
                i++;
            }
        } // while
        
    } catch (const std::exception &e) {
        ACE_DEBUG((LM_ERROR, "IO exception: %s\n", e.what()));
        // Restore exceptions flags
        /* inp.exceptions(iexcflags);
        out.exceptions(iexcflags); */
    }

    // Go back to write the number of contacts
    long pos = out.tellp(); 
    ACE_DEBUG((LM_DEBUG, "tellp %d\n", pos));
    out.seekp(0);
    ACE_DEBUG((LM_DEBUG, "tellp %d\n", (int)out.tellp()));
    out.write((char *)&i, sizeof(i));
    ACE_DEBUG((LM_DEBUG, "tellp %d\n", (int)out.tellp()));
    out.seekp(pos);
    ACE_DEBUG((LM_DEBUG, "tellp %d\n", (int)out.tellp()));
    ACE_DEBUG((LM_DEBUG, "wrote %d contacts\n", i));        

    // Restore exceptions flags
    inp.exceptions(iexcflags);
    out.exceptions(oexcflags);

    return i;
}

} // ns util
} // ns kadc
} // ns networking

