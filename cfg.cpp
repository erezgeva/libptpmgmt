/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* cfg.cpp Read ptp4l Configuration file
 *
 * Authors: Erez Geva <ErezGeva2@gmail.com>
 *
 */

#include "cfg.h"
#include <cstring>

const char globalSection[] = "global";

static inline char *skip_spaces(char *cur)
{
    while(isspace(*cur))cur++;
    return cur;
}
static inline void strip_end_spaces(char *end)
{
    while(isspace(*--end));
    *(end+1) = 0;
}

configSection::configSection()
{
    memset(m_set, 0, sizeof(m_set));
}
void configSection::setGlobal()
{
    // default values
    m_uds_address = "/var/run/ptp4l";
    m_vals[transportSpecific_val] = 0; // default transportSpecific
    m_vals[domainNumber_val] = 0; // default domainNumber
}
bool configSection::set_val(char* line)
{
    char *val = line;
    while(!isspace(*val))val++; // find value
    *val = 0; // leave key in line
    int idx;
    if (strcmp(line, "uds_address") == 0)
        idx = uds_address_val;
    else if (strcmp(line, "transportSpecific") == 0)
        idx = transportSpecific_val;
    else if (strcmp(line, "domainNumber") == 0)
        idx = domainNumber_val;
    else
        return true;
    val++;
    val = skip_spaces(val); // skip spaces at start of value
    strip_end_spaces(val + strlen(val));
    if (*val == 0) // empty value after chomp
        return false;
    if (idx == uds_address_val)
        m_uds_address = val;
    else {
        char *endptr;
        auto ret = strtol(val, &endptr, 0);
        m_vals[idx] = ret;
    }
    m_set[idx] = true;
    return true;
}
configFile::configFile() :
    configGlobal(configPerSection[globalSection])
{
    configGlobal.setGlobal();
}
// read PTP configuration from file
bool configFile::read_cfg(const char *file)
{
    FILE *f = fopen(file, "r");
    if (f == nullptr)
    {
        fprintf(stderr, "fail to open %s: %m\n", file);
        return false;
    }
    char buf[512];
    // remove old configuration
    configPerSection.clear();
    configGlobal = configPerSection[globalSection];
    configGlobal.setGlobal();
    std::string curSection = globalSection;
    while(fgets(buf, sizeof(buf), f) != nullptr) {
       char *cur = skip_spaces(buf);
       if (*cur == '[') {
            cur = skip_spaces(cur + 1);
            char *end = strchr(cur, ']');
            if (end == nullptr) {
                fprintf(stderr, "wrong line in %s: '%s'\n", file, buf);
                return false;
            }
            strip_end_spaces(end);
            curSection = cur;
       } else if (*cur != 0 && *cur != '#' &&
                  !configPerSection[curSection].set_val(cur)) {
            fprintf(stderr, "wrong line in %s: '%s'\n", file, buf);
            return false;
       }
    }
    fclose(f);
    return true;
}
bool configFile::is_global(int idx, const char *section)
{
    if (section == nullptr || !configPerSection[section].m_set[idx])
        return true;
    return false;
}
bool configFile::is_global(int idx, std::string &section)
{
    if (section.empty() || !configPerSection[section].m_set[idx])
        return true;
    return false;
}
uint8_t configFile::get(int idx, const char *section)
{
    if (is_global(idx, section))
        return configGlobal.m_vals[idx];
    return configPerSection[section].m_vals[idx];
}
uint8_t configFile::get(int idx, std::string &section)
{
    if (is_global(idx, section))
        return configGlobal.m_vals[idx];
    return configPerSection[section].m_vals[idx];
}
const std::string &configFile::uds_address(const char *section)
{
    if (is_global(configSection::uds_address_val, section))
        return configGlobal.m_uds_address;
    return configPerSection[section].m_uds_address;
}
const std::string &configFile::uds_address(std::string &section)
{
    if (is_global(configSection::uds_address_val, section))
        return configGlobal.m_uds_address;
    return configPerSection[section].m_uds_address;
}
