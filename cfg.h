/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* cfg.h Read ptp4l Configuration file
 *
 * Authors: Erez Geva <ErezGeva2@gmail.com>
 *
 */

#ifndef __CFG_H
#define __CFG_H

#include <stdint.h>
#include <string>
#include <map>

/*
 * Classes to read ptp4l configuration file
 * We only read the 3 values needed by Managment
 */

class configFile;

class configSection {
protected:
    static const int transportSpecific_val = 0;
    static const int domainNumber_val = 1;
    static const int uds_address_val = 2;
    uint8_t m_vals[2];
    bool m_set[3];
    std::string m_uds_address;

    friend class configFile;
    void setGlobal();
    bool set_val(char* line);
public:
    configSection();
};

class configFile {
private:
    std::map<std::string, configSection> configPerSection;
    configSection &configGlobal;

    uint8_t get(int idx, const char *section);
    uint8_t get(int idx, std::string &section);
    bool is_global(int idx, const char *section);
    bool is_global(int idx, std::string &section);
public:
    configFile();
    bool read_cfg(const char *file);
    bool read_cfg(std::string &file)
        {return read_cfg(file.c_str());}
    uint8_t transportSpecific(const char *section = nullptr)
        {return get(configSection::transportSpecific_val, section);}
    uint8_t transportSpecific(std::string &section)
        {return get(configSection::transportSpecific_val, section);}
    uint8_t domainNumber(const char *section = nullptr)
        {return get(configSection::domainNumber_val, section);}
    uint8_t domainNumber(std::string &section)
        {return get(configSection::domainNumber_val, section);}
    const std::string &uds_address(const char *section = nullptr);
    const std::string &uds_address(std::string &section);
    const char *uds_address_c(const char *section = nullptr)
        {return uds_address(section).c_str();}
    const char *uds_address_c(std::string &section)
        {return uds_address(section).c_str();}
};

#endif /*__CFG_H*/
