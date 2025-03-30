/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief Class to get and set the configuration of time base
 *
 * @author Song Yoong Siang <yoong.siang.song@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#ifndef CLIENT_TIMEBASE_CONFIGS_H
#define CLIENT_TIMEBASE_CONFIGS_H

#include "pub/clkmgr/utility.h"
#include "pub/clkmgr/types.h"
#include <map>
#include <vector>
#include <string>

__CLKMGR_NAMESPACE_BEGIN

class ClientConnectMessage;
class TimeBaseRecord;
class TimeBaseConfigurations;
struct TimeBaseCfg;

/**
 * Class to PTP configuration of a time base configuration
 */
class PTPCfg
{
  private:
    std::string m_ifName; /**< Network interface name */
    uint8_t m_transportSpecific = 1; /**< PTP Transport specific */
    uint8_t m_domainNumber = 0; /**< PTP Domain number */
    int m_ifIndex = -1; /**< Network interface index */
    int m_ptpIndex = -1; /**< PTP device index */

  protected:
    /** @cond internal */
    friend class TimeBaseConfigurations;
    friend class TimeBaseRecord;
    PTPCfg() = default;
    /**
     * Constructor
     * @param[in] ifName Network interface name
     * @param[in] ifIndex Network interface index
     * @param[in] ptpIndex PTP device index
     * @param[in] transportSpecific PTP Transport specific
     * @param[in] domainNumber PTP Domain number
     */
    PTPCfg(const std::string &ifName, uint8_t transportSpecific,
        uint8_t domainNumber, int ifIndex = -1, int ptpIndex = -1);
    /**< @endcond */

  public:
    /**
     * Get Network interface name
     * @return Network interface name
     */
    const std::string &ifName() const;
    /**
     * Get Network interface name
     * @return Network interface name
     */
    const char *ifName_c() const;
    /**
     * Get Network interface index
     * @return Network interface index
     */
    int ifIndex() const;
    /**
     * Get PTP device index
     * @return PTP device index
     */
    int ptpIndex() const;
    /**
     * Get PTP Transport specific
     * @return PTP Transport specific
     */
    uint8_t transportSpecific() const;
    /**
     * Get PTP Domain number
     * @return PTP Domain number
     */
    uint8_t domainNumber() const;
};

/**
 * Class to hold a single time base configuration
 */
class TimeBaseRecord
{
  private:
    size_t m_index; /**< Index of the time base */
    std::string m_name; /**< Name of the time base */
    bool m_have_ptp = true; /**< does this time base use PTP */
    PTPCfg m_ptp; /**< PTP configuration */
    bool m_have_sys = true; /**< does this time base sync the system clock? */

  protected:
    /** @cond internal */
    friend class TimeBaseConfigurations;
    /**
     * Constructor
     * @param[in] index of the time base
     * @param[in] name of the time base
     */
    TimeBaseRecord(size_t index, const std::string &name);
    /**
     * Set PTP parameters
     * @param[in] ptp configuration to use
     */
    void setPtp(PTPCfg &&ptp);

  public:
    TimeBaseRecord() = default;
    /**< @endcond */
    /**
     * Get index of the time base
     * @return index of the time base
     */
    size_t index() const;
    /**
     * Get name of the time base
     * @return name of the time base
     */
    const std::string &name() const;
    /**
     * Get name of the time base
     * @return name of the time base
     */
    const char *name_c() const;
    /**
     * Query if we use PTP
     * @return true if this time base have PTP
     */
    bool havePtp() const;
    /**
     * Get PTP configuration
     * @return PTP configuration
     */
    const PTPCfg &ptp() const;
    /**
     * Query if synchronize the system clock
     * @return true if this time base does synchronize
     */
    bool haveSysClock() const;
};

/**
 * Class holding time base configurations
 */
class TimeBaseConfigurations
{
  private:
    std::map<size_t, TimeBaseRecord> m_cfgs;
    /**
     * Private constructor to prevent instantiation.
     */
    TimeBaseConfigurations() = default;
    /**
     * Get the single instance of the TimeBaseConfigurations.
     * @return writable Reference to the single instance.
     */
    static TimeBaseConfigurations &getInstWr();

  protected:
    /** @cond internal */
    friend class ClientConnectMessage;
    /**
     * Add a time base configuration.
     * @param[in] cfg TimeBaseCfg to add.
     */
    static void addTimeBaseCfg(const struct TimeBaseCfg &cfg);

  public:
    #ifndef SWIG
    class iterator
    {
      private:
        std::map<size_t, TimeBaseRecord>::const_iterator it;
      protected:
        iterator(const std::map<size_t, TimeBaseRecord>::const_iterator &it);
        friend class TimeBaseConfigurations;
      public:
        /* iterator need to support these following operators */
        iterator &operator++(); /* prefix increment */
        iterator &operator++(int); /* postfix increment */
        const TimeBaseRecord &operator*();
        bool operator!=(iterator &o);
    };
    #endif /* SWIG */
    /**< @endcond */
    /**
     * Get the single instance of the TimeBaseConfigurations.
     * @return Reference to the single instance.
     */
    static const TimeBaseConfigurations &getInstance();
    /**
     * Get record a given timeBaseIndex
     * @param[in] timeBaseIndex The time base index to fetch.
     * @return empty record if not found.
     */
    static const TimeBaseRecord &getRecord(size_t timeBaseIndex);
    /**
     * Check if a given timeBaseIndex exist
     * @param[in] timeBaseIndex The time base index to check.
     * @return true if found, false otherwise.
     */
    static bool isTimeBaseIndexPresent(size_t timeBaseIndex);
    /**
     * find index of the time base using its name.
     * @param[in] timeBaseName The time base name.
     * @param[out] timeBaseIndex The time base index.
     * @return true if found
     */
    static bool BaseNameToBaseIndex(const std::string &timeBaseName,
        size_t &timeBaseIndex);
    /**
     * Return number of time base configurations
     * @return size
     */
    static size_t size();
    #ifndef SWIG
    /**
     * Fetch iterator to the first tlv
     * @return iterator
     * @note iterators are supported using C++ only
     */
    static iterator begin();
    /**
     * Fetch iterator that mark the end of the TLVs list
     * @return iterator
     * @note iterators are supported using C++ only
     */
    static iterator end();
    #endif /* SWIG */
};

__CLKMGR_NAMESPACE_END

#endif /* CLIENT_TIMEBASE_CONFIGS_H */
