#ifndef _APP_VERSION_H_
#define _APP_VERSION_H_

#define APP_NAME    "RV House (Blue Ghost Edition)"
#define APP_VERSION_ONLY "0.91.6"
#define APP_VERSION APP_VERSION_ONLY

// #define RV_HOUSE_TEST

// Returns negative if version a is less than version b
// and positive if otherwise. 0 if the same.
int app_version_compare(const char *a, const char *b);

inline int app_version_compare(const std::string &a, const std::string &b) {
    return app_version_compare(a.c_str(), b.c_str());
}


#endif //_APP_VERSION_H_
