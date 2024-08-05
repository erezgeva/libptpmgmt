dnl SPDX-License-Identifier: LGPL-3.0-or-later
dnl SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */
dnl
dnl @file
dnl @brief Create PTP management TLV structures
dnl        for main C++ library and for wrapper C
dnl
dnl @author Erez Geva <ErezGeva2@@gmail.com>
dnl @copyright © 2024 Erez Geva
dnl
dnl Create types for main C++ library and for wrapper C
dnl
include(lang().m4)dnl
/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief PTP management TLV structures
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 */

ics(PROC)

cpp_st()dnl
incpp(<vector>)dnl
inc_c(stdlib)
incb(types)

ns_s()

/** Clock description TLV */
strc(CLOCK_DESCRIPTION_t) sz(: public BaseMngTlv) {
    uint16_t clockType; /**< Clock type bit mask */
    strcc(PTPText_t) physicalLayerProtocol; /**< Physical protocol */
    UInteger16_t physicalAddressLength; /**< Address length */
    bintyp()physicalAddress; /**< Physical address */
    strcc(PortAddress_t) protocolAddress; /**< Protocol address */
    Octet_t manufacturerIdentity[3]; /**< IEEE OUI */
    strcc(PTPText_t) productDescription; /**< Product description */
    strcc(PTPText_t) revisionData; /**< Revision data */
    strcc(PTPText_t) userDescription; /**< User description */
    Octet_t profileIdentity[6]; /**< Profile ID */
};
/** User description TLV */
strc(USER_DESCRIPTION_t) sz(: public BaseMngTlv) {
    strcc(PTPText_t) userDescription; /**< User description */
};
/** Initialize event */
cnst_st() uint16_t NM(INITIALIZE_EVENT) = 0x0000;
/** Initialize TLV */
strc(INITIALIZE_t) sz(: public BaseMngTlv) {
    uint16_t initializationKey; /**< Initialization key */
};
/** Fault logging table TLV */
strc(FAULT_LOG_t) sz(: public BaseMngTlv) {
    UInteger16_t numberOfFaultRecords; /**< Number of fault records */
    vec(FaultRecord_t)faultRecords; /**< Fault records table */
};
/** Default data settings */
strc(DEFAULT_DATA_SET_t) sz(: public BaseMngTlv) {
    /**
     * Bit fields flag
     * @li bit 0: TSC defaultDS.twoStepFlag
     * @li bit 1: SO  defaultDS.slaveOnly
     */
    uint8_t flags;
cpp_cod(`    const uint8_t flagsMask = 0x3; /**< Mask for flags */')dnl
    UInteger16_t numberPorts; /**< Number of ports */
    UInteger8_t priority1; /**< Priority 1 */
    strcc(ClockQuality_t) clockQuality; /**< Clock quality */
    UInteger8_t priority2; /**< Priority 2 */
    strcc(ClockIdentity_t) clockIdentity; /**< Clock ID */
    UInteger8_t domainNumber; /**< Domain ID number */
};
/** Current data setting TLV */
strc(CURRENT_DATA_SET_t) sz(: public BaseMngTlv) {
    UInteger16_t stepsRemoved; /**< Steps removed */
    /** Offset from time transmitter clock */
    strcc(TimeInterval_t) offsetFromMaster;
    /** Mean path delay to time transmitter */
    strcc(TimeInterval_t) meanPathDelay;
};
/** Parent data set TLV */
strc(PARENT_DATA_SET_t) sz(: public BaseMngTlv) {
    strcc(PortIdentity_t) parentPortIdentity; /**< Port ID of parent */
    /**
     * Bit fields flag
     * @li bit 0: PS parentDS.parentStats
     */
    uint8_t flags;
cpp_cod(`    const uint8_t flagsMask = 0x1; /**< Mask for flags */')dnl
    /** The variance of the parent clock's phase as
        measured by the local clock */
    UInteger16_t observedParentOffsetScaledLogVariance;
    /** Estimate of the parent clock's phase change rate as
        measured by the timeReceiver clock */
    Integer32_t observedParentClockPhaseChangeRate;
    UInteger8_t grandmasterPriority1; /**< Grandmaster Priority 1 */
    /** Grandmaster clock quality */
    strcc(ClockQuality_t) grandmasterClockQuality;
    UInteger8_t grandmasterPriority2; /**< Grandmaster Priority 1 */
    strcc(ClockIdentity_t) grandmasterIdentity; /**< Grandmaster clock ID */
};
/** Time properties data set TLV */
strc(TIME_PROPERTIES_DATA_SET_t) sz(: public BaseMngTlv) {
    Integer16_t currentUtcOffset; /**< current TAI to UTC offset, leap seconds */
    /**
     * Bit fields flag
     * @li bit 0: LI-61 timePropertiesDS.leap61
     * @li bit 1: LI-59 timePropertiesDS.leap59
     * @li bit 2: UTCV  timePropertiesDS.currentUtcOffsetValid
     * @li bit 3: PTP   timePropertiesDS.ptpTimescale
     * @li bit 4: TTRA  timePropertiesDS.timeTraceable
     * @li bit 5: FTRA  timePropertiesDS.frequencyTraceable
     */
    uint8_t flags;
cpp_cod(`    const uint8_t flagsMask = 0x3f; /**< Mask for flags */')dnl
    enmc(timeSource_e) timeSource; /**< Clock source type */
};
/** Port data set TLV */
strc(PORT_DATA_SET_t) sz(: public BaseMngTlv) {
    strcc(PortIdentity_t) portIdentity; /**< Port ID */
    enmc(portState_e) portState; /**< Port state */
    /** the minimum permitted mean time interval between
        successive Delay_Req messages */
    Integer8_t logMinDelayReqInterval;
    strcc(TimeInterval_t) peerMeanPathDelay; /**< Mean path delay to peer */
    /** the mean time interval between successive Announce messages */
    Integer8_t logAnnounceInterval;
    /** specify the number of announceInterval that has to
        pass without receipt of an Announce */
    UInteger8_t announceReceiptTimeout;
    /** the mean time interval between successive Sync messages */
    Integer8_t logSyncInterval;
    /**
     * Delay mechanism values
     */
    enmc(delayMechanism_e) delayMechanism;
    /** the minimum permitted mean time interval between
        successive Pdelay_Req messages */
    Integer8_t logMinPdelayReqInterval;
    /**
     * PTP version in use on port
     * @li Nibble:  major version
     * @li Nibble:  reserved. Probably minor version in next versions
     */
    Nibble_t versionNumber;
};
/** Priority 1 TLV */
strc(PRIORITY1_t) sz(: public BaseMngTlv) {
    UInteger8_t priority1; /**< Priority 1 */
};
/** Priority 2 TLV */
strc(PRIORITY2_t) sz(: public BaseMngTlv) {
    UInteger8_t priority2; /**< Priority 2 */
};
/** Domain TLV */
strc(DOMAIN_t) sz(: public BaseMngTlv) {
    UInteger8_t domainNumber; /**< Domain ID number */
};
/** timeReceiver only flag TLV */
strc(SLAVE_ONLY_t) sz(: public BaseMngTlv) {
    /**
     * Bit fields flag
     * @li bit 0: SO  defaultDS.slaveOnly
     */
    uint8_t flags;
cpp_cod(`    const uint8_t flagsMask = 0x1; /**< Mask for flags */')dnl
};
/** Log announce interval TLV */
strc(LOG_ANNOUNCE_INTERVAL_t) sz(: public BaseMngTlv) {
    /** the mean time interval between successive Announce messages */
    Integer8_t logAnnounceInterval;
};
/** Announce receipt timeout TLV */
strc(ANNOUNCE_RECEIPT_TIMEOUT_t) sz(: public BaseMngTlv) {
    /** Specify the number of announce interval that must
        pass without receipt of an announce */
    UInteger8_t announceReceiptTimeout;
};
/** Log synchronization interval TLV */
strc(LOG_SYNC_INTERVAL_t) sz(: public BaseMngTlv) {
    /** the mean time interval between successive Sync messages */
    Integer8_t logSyncInterval;
};
/** Version number TLV */
strc(VERSION_NUMBER_t) sz(: public BaseMngTlv) {
    /**
     * PTP version in use on port
     * @li Nibble:  major version
     * @li Nibble:  reserved. Probably minor version in next versions
     */
    Nibble_t versionNumber;
};
/** Current time TLV */
strc(TIME_t) sz(: public BaseMngTlv) {
    strcc(Timestamp_t) currentTime; /**< Current time stamp */
};
/** Clock accuracy TLV */
strc(CLOCK_ACCURACY_t) sz(: public BaseMngTlv) {
    enmc(clockAccuracy_e) clockAccuracy; /**< Clock accuracy */
};
/** Utc properties TLV */
strc(UTC_PROPERTIES_t) sz(: public BaseMngTlv) {
    Integer16_t currentUtcOffset; /**< current TAI to UTC offset, leap seconds */
    /**
     * Bit fields flag
     * @li bit 0: LI-61 timePropertiesDS.leap61
     * @li bit 1: LI-59 timePropertiesDS.leap59
     * @li bit 2: UTCV  timePropertiesDS.currentUtcOffsetValid
     */
    uint8_t flags;
cpp_cod(`    const uint8_t flagsMask = 0x7; /**< Mask for flags */')dnl
};
/** Traceability properties TLV */
strc(TRACEABILITY_PROPERTIES_t) sz(: public BaseMngTlv) {
    /**
     * Bit fields flag
     * @li bit 4: TTRA  timePropertiesDS.timeTraceable
     * @li bit 5: FTRA  timePropertiesDS.frequencyTraceable
     */
    uint8_t flags;
cpp_cod(`    const uint8_t flagsMask = 0x30; /**< Mask for flags */')dnl
};
/** Timescale properties TLV */
strc(TIMESCALE_PROPERTIES_t) sz(: public BaseMngTlv) {
    /**
     * Bit fields flag
     * @li bit 3: PTP   timePropertiesDS.ptpTimescale
     */
    uint8_t flags;
cpp_cod(`    const uint8_t flagsMask = 0x8; /**< Mask for flags */')dnl
    enmc(timeSource_e) timeSource; /**< Clock source type */
};
/** Unicast negotiation enables TLV */
strc(UNICAST_NEGOTIATION_ENABLE_t) sz(: public BaseMngTlv) {
    /**
     * Bit fields flag
     * @li bit 0: EN - unicastNegotiationPortDS.enable
     */
    uint8_t flags;
cpp_cod(`    const uint8_t flagsMask = 0x1; /**< Mask for flags */')dnl
};
/** Path trace list TLV */
strc(PATH_TRACE_LIST_t) sz(: public BaseMngTlv) {
    vec(ClockIdentity_t)pathSequence; /**< clock id per path */
};
/** Path-trace enable TLV */
strc(PATH_TRACE_ENABLE_t) sz(: public BaseMngTlv) {
    /**
     * Bit fields flag
     * @li bit 0: EN pathTraceDS.enable
     */
    uint8_t flags;
cpp_cod(`    const uint8_t flagsMask = 0x1; /**< Mask for flags */')dnl
};
/** Grandmaster cluster table TLV */
strc(GRANDMASTER_CLUSTER_TABLE_t) sz(: public BaseMngTlv) {
    /** logarithm to the base 2 of the mean interval in seconds between
        unicast Announce messages from grandmaster */
    Integer8_t logQueryInterval;
    UInteger8_t actualTableSize; /**< Number of addresses table */
    /** Port addresses of grandmasters cluster */
    vec(PortAddress_t)PortAddress;
};
/** Unicast time transmitter table TLV */
strc(UNICAST_MASTER_TABLE_t) sz(: public BaseMngTlv) {
    /** logarithm to the base 2 of the mean interval in seconds between
        unicast Announce messages from time transmitter */
    Integer8_t logQueryInterval;
    UInteger16_t actualTableSize; /**< Number of addresses table */
    /** Port addresses of unicast time transmitters */
    vec(PortAddress_t)PortAddress;
};
/** Unicast time transmitter maximum table size TLV */
strc(UNICAST_MASTER_MAX_TABLE_SIZE_t) sz(: public BaseMngTlv) {
    /** Maximum number of addresses in unicast time transmitters table */
    UInteger16_t maxTableSize;
};
/** Acceptable time transmitter table TLV */
strc(ACCEPTABLE_MASTER_TABLE_t) sz(: public BaseMngTlv) {
    Integer16_t actualTableSize; /**< Number of addresses table */
    /** Acceptable time transmitter table records */
    vec(AcceptableMaster_t)list;
};
/** Acceptable time transmitter table enabled TLV */
strc(ACCEPTABLE_MASTER_TABLE_ENABLED_t) sz(: public BaseMngTlv) {
    /**
     * Bit fields flag
     * @li bit 0: EN acceptableMasterPortDS.enable.
     */
    uint8_t flags;
cpp_cod(`    const uint8_t flagsMask = 0x1; /**< Mask for flags */')dnl
};
/** Acceptable time transmitter maximum table size TLV */
strc(ACCEPTABLE_MASTER_MAX_TABLE_SIZE_t) sz(: public BaseMngTlv) {
    /** The maximum permitted number of addresses
         in the Acceptable time transmitter table */
    UInteger16_t maxTableSize;
};
/** Alternate time transmitter TLV */
strc(ALTERNATE_MASTER_t) sz(: public BaseMngTlv) {
    /**
     * Bit fields flag
     * @li bit 0: S - alternateMasterPortDS.transmitAlternateMulticastSync
     */
    uint8_t flags;
cpp_cod(`    const uint8_t flagsMask = 0x1; /**< Mask for flags */')dnl
    /** Logarithm to the base 2 of the mean period in seconds
        between Sync messages used by alternate time transmitter */
    Integer8_t logAlternateMulticastSyncInterval;
    /** Number of alternate time transmitters */
    UInteger8_t numberOfAlternateMasters;
};
/** Alternate time offset enables TLV */
strc(ALTERNATE_TIME_OFFSET_ENABLE_t) sz(: public BaseMngTlv) {
    UInteger8_t keyField; /**< the index to the alternate timescale offsets */
    /**
     * Bit fields flag
     * @li bit 0: EN alternateTimescaleOffsetsDS.list[keyField].enable
     */
    uint8_t flags;
cpp_cod(`    const uint8_t flagsMask = 0x1; /**< Mask for flags */')dnl
};
/** Alternate time offset name TLV */
strc(ALTERNATE_TIME_OFFSET_NAME_t) sz(: public BaseMngTlv) {
    UInteger8_t keyField; /**< the index to the alternate timescale offsets */
    /** Name of the alternate timescale offset */
    strcc(PTPText_t) displayName;
};
/** Alternate time offset maximum key TLV */
strc(ALTERNATE_TIME_OFFSET_MAX_KEY_t) sz(: public BaseMngTlv) {
    UInteger8_t maxKey; /**< number of alternate timescales maintained */
};
/** Alternate time offset properties TLV */
strc(ALTERNATE_TIME_OFFSET_PROPERTIES_t) sz(: public BaseMngTlv) {
    UInteger8_t keyField; /**< the index to the alternate timescale offsets */
    Integer32_t currentOffset; /**< Current offset */
    Integer32_t jumpSeconds; /**< Jump seconds */
    UInteger48_t timeOfNextJump; /**< Time of next jump */
};
/** Transparent clock port data set TLV */
strc(TRANSPARENT_CLOCK_PORT_DATA_SET_t) sz(: public BaseMngTlv) {
    strcc(PortIdentity_t) portIdentity; /**< Port ID */
    /**
     * Bit fields flag
     * @li bit 0: FLT transparentClockPortDS.faultyFlag
     */
    uint8_t flags;
cpp_cod(`    const uint8_t flagsMask = 0x1; /**< Mask for flags */')dnl
    /** the minimum permitted mean time interval between
        successive Pdelay_Req messages */
    Integer8_t logMinPdelayReqInterval;
    strcc(TimeInterval_t) peerMeanPathDelay; /**< Mean path delay to peer */
};
/** Log min pdelay req interval TLV */
strc(LOG_MIN_PDELAY_REQ_INTERVAL_t) sz(: public BaseMngTlv) {
    /** the minimum permitted mean time interval between
        successive Pdelay_Req messages */
    Integer8_t logMinPdelayReqInterval;
};
/** Transparent clock default data set TLV */
strc(TRANSPARENT_CLOCK_DEFAULT_DATA_SET_t) sz(: public BaseMngTlv) {
    strcc(ClockIdentity_t) clockIdentity; /**< Clock ID */
    UInteger16_t numberPorts; /**< Number of ports */
    /**
     * Delay mechanism values
     */
    enmc(delayMechanism_e) delayMechanism;
    UInteger8_t primaryDomain; /**< Primary Domain ID number */
};
/** Primary domain TLV */
strc(PRIMARY_DOMAIN_t) sz(: public BaseMngTlv) {
    UInteger8_t primaryDomain; /**< Primary Domain ID number */
};
/** Delay mechanism TLV */
strc(DELAY_MECHANISM_t) sz(: public BaseMngTlv) {
    /**
     * Delay mechanism values
     */
    enmc(delayMechanism_e) delayMechanism;
};
/** External port configuration enabled TLV */
strc(EXTERNAL_PORT_CONFIGURATION_ENABLED_t) sz(: public BaseMngTlv) {
    /**
     * Bit fields flag
     * @li bit 0: EPC defaultDS.externalPortConfigurationEnabled
     */
    uint8_t flags;
cpp_cod(`    const uint8_t flagsMask = 0x1; /**< Mask for flags */')dnl
};
/** Time transmitter only TLV */
strc(MASTER_ONLY_t) sz(: public BaseMngTlv) {
    /**
     * Bit fields flag
     * @li bit 0: MO - portDS.masterOnly
     */
    uint8_t flags;
cpp_cod(`    const uint8_t flagsMask = 0x1; /**< Mask for flags */')dnl
};
/** Holdover-upgrade enable TLV */
strc(HOLDOVER_UPGRADE_ENABLE_t) sz(: public BaseMngTlv) {
    /**
     * Bit fields flag
     * @li bit 0: EN holdoverUpgradeDS.enable
     */
    uint8_t flags;
cpp_cod(`    const uint8_t flagsMask = 0x1; /**< Mask for flags */')dnl
};
/** External port config port data setting TLV */
strc(EXT_PORT_CONFIG_PORT_DATA_SET_t) sz(: public BaseMngTlv) {
    /**
     * Bit fields flag
     * @li bit 0: EN acceptableMasterPortDS.enable.
     */
    uint8_t flags;
cpp_cod(`    const uint8_t flagsMask = 0x1; /**< Mask for flags */')dnl
    enmc(portState_e) desiredState; /**< Desired port state */
};
/** TIME_STATUS_NP.cumulativeScaledRateOffset scale factor */
cnst_st() float_nanoseconds NM(P41) = 1ULL << 41;
/** Time status TLV
 * @note linuxptp implementation specific
 */
strc(TIME_STATUS_NP_t) sz(: public BaseMngTlv) {
    int64_t master_offset; /**< Offset from time transmitter clock in nanoseconds */
    int64_t ingress_time;  /**< Ingress time in nanoseconds */
    /** Cumulative scaled rate offset */
    Integer32_t cumulativeScaledRateOffset;
    /** Scaled last grandmaster phase change */
    Integer32_t scaledLastGmPhaseChange;
    /** Grandmaster time base indicator */
    UInteger16_t gmTimeBaseIndicator;
    /** Scaled last grandmaster phase change, MSB part of nanoseconds */
    uint16_t nanoseconds_msb;
    /** Scaled last grandmaster phase change, LSB part of nanoseconds */
    uint64_t nanoseconds_lsb;
    /** Scaled last grandmaster phase change, nanosecond fraction */
    uint16_t fractional_nanoseconds;
    Integer32_t gmPresent; /**< Flag for grandmaster presence */
    strcc(ClockIdentity_t) gmIdentity; /**< Grandmaster clock ID */
};
/** Grandmaster settings TLV
 * @note linuxptp implementation specific
 */
strc(GRANDMASTER_SETTINGS_NP_t) sz(: public BaseMngTlv) {
    strcc(ClockQuality_t) clockQuality; /**< Clock quality */
    Integer16_t currentUtcOffset; /**< current TAI to UTC offset, leap seconds */
    /**
     * Bit fields flag
     * @li bit 0: LI-61 timePropertiesDS.leap61
     * @li bit 1: LI-59 timePropertiesDS.leap59
     * @li bit 2: UTCV  timePropertiesDS.currentUtcOffsetValid
     * @li bit 3: PTP   timePropertiesDS.ptpTimescale
     * @li bit 4: TTRA  timePropertiesDS.timeTraceable
     * @li bit 5: FTRA  timePropertiesDS.frequencyTraceable
     */
    uint8_t flags;
cpp_cod(`    const uint8_t flagsMask = 0x3f; /**< Mask for flags */')dnl
    enmc(timeSource_e) timeSource; /**< Clock source type */
};
/** Port data setting TLV
 * @note linuxptp implementation specific
 */
strc(PORT_DATA_SET_NP_t) sz(: public BaseMngTlv) {
    /** Neighbour proper delay threshold in nanoseconds */
    UInteger32_t neighborPropDelayThresh;
    Integer32_t asCapable; /**< Flag for 802@.1AS Capable */
};
/** SUBSCRIBE_EVENTS_NP.bitmask size */
cnst(int, EVENT_BITMASK_CNT, 64)
/** Notify port state offset in SUBSCRIBE_EVENTS_NP.bitmask */
cnst_st() int NM(NOTIFY_PORT_STATE) = 0;
/** Notify time synchronization offset in SUBSCRIBE_EVENTS_NP.bitmask */
cnst_st() int NM(NOTIFY_TIME_SYNC) = 1;
/** Notify parent data in SUBSCRIBE_EVENTS_NP.bitmask */
cnst_st() int NM(NOTIFY_PARENT_DATA_SET) = 2;
/** Notify Common Mean Link Delay Information in SUBSCRIBE_EVENTS_NP.bitmask */
cnst_st() int NM(NOTIFY_CMLDS) = 3;
/** Subscribe events TLV
 * @note linuxptp implementation specific
 */
strc(SUBSCRIBE_EVENTS_NP_t) sz(: public BaseMngTlv) {
    uint16_t duration; /**< duration in seconds */
    arr(uint8_t,bitmask,EVENT_BITMASK_CNT); /**< bitmask of events state */
c_cod(`};')dnl
cpp_cod(`    /**< Default constructor */')dnl
cpp_cod(`    /**< We need to zero the bitmask, as we do not set it explicity! */')dnl
cpp_cod(`    SUBSCRIBE_EVENTS_NP_t() : duration(0), bitmask{0} {}')dnl
idf()/**
idf() * Set event bit in bitmask
c_cod(idf()` * @param[in, out] tlv with the events storage')dnl
idf() * @param[in] event to set
idf() */
c_cod(`void ptpmgmt_setEvent_lnp(struct ptpmgmt_SUBSCRIBE_EVENTS_NP_t *tlv, int event);')dnl
cpp_cod(`    void setEvent(int event) {')dnl
cpp_cod(`        std::div_t d;')dnl
cpp_cod(`        if(div_event(event, d))')dnl
cpp_cod(`            bitmask[d.quot] |= d.rem;')dnl
cpp_cod(`    }')dnl
idf()/**
idf() * Clear event bit in bitmask
c_cod(idf()` * @param[in, out] tlv with the events storage')dnl
idf() * @param[in] event to clear
idf() */
c_cod(`void ptpmgmt_clearEvent_lnp(struct ptpmgmt_SUBSCRIBE_EVENTS_NP_t *tlv,')dnl
c_cod(`    int event);')dnl
cpp_cod(`    void clearEvent(int event) {')dnl
cpp_cod(`        std::div_t d;')dnl
cpp_cod(`        if(div_event(event, d))')dnl
cpp_cod(`            bitmask[d.quot] &= ~d.rem;')dnl
cpp_cod(`    }')dnl
idf()/**
idf() * Clear all events in bitmask
c_cod(idf()` * @param[in, out] tlv with the events storage')dnl
idf() */
c_cod(`void ptpmgmt_clearAll_lnp(struct ptpmgmt_SUBSCRIBE_EVENTS_NP_t *tlv);')dnl
cpp_cod(`    void clearAll() {')dnl
cpp_cod(`        memset(bitmask, 0, EVENT_BITMASK_CNT);')dnl
cpp_cod(`    }')dnl
idf()/**
idf() * Get bit value in bitmask
c_cod(idf()` * @param[in, out] tlv with the events storage')dnl
idf() * @param[in] event to query
idf() * @return true if event set
idf() */
c_cod(`bool ptpmgmt_getEvent_lnp(const struct ptpmgmt_SUBSCRIBE_EVENTS_NP_t *tlv,')dnl
c_cod(`    int event);')dnl
cpp_cod(`    bool getEvent(int event) const {')dnl
cpp_cod(`        std::div_t d;')dnl
cpp_cod(`        if(div_event(event, d))')dnl
cpp_cod(`            return (bitmask[d.quot] & d.rem) > 0;')dnl
cpp_cod(`        return false;')dnl
cpp_cod(`    }')dnl
cpp_cod(`    /** @cond internal */')dnl
cpp_cod(`    /** Divide event to byte and bit locations */')dnl
cpp_cod(`    static std::div_t div_event(int event) {')dnl
cpp_cod(`        std::div_t d;')dnl
cpp_cod(`        div_event_wo(event, d);')dnl
cpp_cod(`        return d;')dnl
cpp_cod(`    }')dnl
cpp_cod(`  private:')dnl
cpp_cod(`    /** Divide event to byte and bit locations without check */')dnl
cpp_cod(`    static void div_event_wo(int event, std::div_t &d) {')dnl
cpp_cod(`        d = div(event, 8);')dnl
cpp_cod(`        d.rem = 1 << d.rem;')dnl
cpp_cod(`    }')dnl
cpp_cod(`    /** Divide event to byte and bit locations */')dnl
cpp_cod(`    static bool div_event(int event, std::div_t &d) {')dnl
cpp_cod(`        if(event < 0 || event >= EVENT_BITMASK_CNT)')dnl
cpp_cod(`            return false;')dnl
cpp_cod(`        div_event_wo(event, d);')dnl
cpp_cod(`        return true;')dnl
cpp_cod(`    }')dnl
cpp_cod(`    /**< @endcond */')dnl
cpp_cod(`};')dnl
/** Port properties TLV
 * @note linuxptp implementation specific
 */
strc(PORT_PROPERTIES_NP_t) sz(: public BaseMngTlv) {
    strcc(PortIdentity_t) portIdentity; /**< Port ID */
    enmc(portState_e) portState; /**< Port state */
    /** time stamping in linuxptp format */
    enmc(linuxptpTimeStamp_e) timestamping;
    strcc(PTPText_t) interface; /**< Linux interface name */
};
/** size of PORT_STATS_NP rxMsgType and txMsgType */
cnst(int, MAX_MESSAGE_TYPES, 16)
/** Sync messages count in PORT_STATS_NP */
cnst_st() int NM(STAT_SYNC) = 0;
/** Delay_Req messages count in PORT_STATS_NP */
cnst_st() int NM(STAT_DELAY_REQ) = 1;
/** Pdelay_Req messages count in PORT_STATS_NP */
cnst_st() int NM(STAT_PDELAY_REQ) = 2;
/** Pdelay_Resp messages count in PORT_STATS_NP */
cnst_st() int NM(STAT_PDELAY_RESP) = 3;
/** Follow_Up messages count in PORT_STATS_NP */
cnst_st() int NM(STAT_FOLLOW_UP) = 8;
/** Delay_Resp messages count in PORT_STATS_NP */
cnst_st() int NM(STAT_DELAY_RESP) = 9;
/** Pdelay_Resp_Follow_Up messages count in PORT_STATS_NP */
cnst_st() int NM(STAT_PDELAY_RESP_FOLLOW_UP) = 10;
/** Announce messages count in PORT_STATS_NP */
cnst_st() int NM(STAT_ANNOUNCE) = 11;
/** Signalling messages count in PORT_STATS_NP */
cnst_st() int NM(STAT_SIGNALING) = 12;
/** Management messages count in PORT_STATS_NP */
cnst_st() int NM(STAT_MANAGEMENT) = 13;
/** Port statistics TLV
 * @note linuxptp implementation specific
 */
strc(PORT_STATS_NP_t) sz(: public BaseMngTlv) {
    strcc(PortIdentity_t) portIdentity; /**< Port ID */
    /** Port received messages count per PTP message type */
    arr(uint64_t, rxMsgType, MAX_MESSAGE_TYPES);
    /** Port transmitted messages count per PTP message type */
    arr(uint64_t, txMsgType, MAX_MESSAGE_TYPES);
};
/** SYNCHRONIZATION_UNCERTAIN_NP uncertain false state */
cnst_st() uint8_t NM(SYNC_UNCERTAIN_FALSE) = 0;
/** SYNCHRONIZATION_UNCERTAIN_NP uncertain true state */
cnst_st() uint8_t NM(SYNC_UNCERTAIN_TRUE) = 1;
/** SYNCHRONIZATION_UNCERTAIN_NP uncertain do not care state */
cnst_st() uint8_t NM(SYNC_UNCERTAIN_DONTCARE) = 0xff;
/** Synchronization uncertain TLV
 * @note linuxptp implementation specific
 */
strc(SYNCHRONIZATION_UNCERTAIN_NP_t) sz(: public BaseMngTlv) {
    /**
     * synchronization uncertain state
     * @li 0:       False
     * @li 1:       True
     * @li 0xff:    Do not care
     */
    uint8_t val;
};
/** Service statistics TLV
 * @note linuxptp implementation specific
 */
strc(PORT_SERVICE_STATS_NP_t) sz(: public BaseMngTlv) {
    strcc(PortIdentity_t) portIdentity; /**< port ID */
    uint64_t announce_timeout; /**< Number of announce message  timeout */
    uint64_t sync_timeout; /**< Number of sync message timeout */
    uint64_t delay_timeout; /**< Number of delay message timeout */
    /** Number of unicast service message timeout */
    uint64_t unicast_service_timeout;
    /** Number of unicast request message timeout */
    uint64_t unicast_request_timeout;
    /** Number of master announce message timeout */
    uint64_t master_announce_timeout;
    uint64_t master_sync_timeout; /**< Number of master sync message timeout */
    uint64_t qualification_timeout; /**< Number of qualification message timeout */
    uint64_t sync_mismatch; /**< Number of sync message mismatch */
    uint64_t followup_mismatch; /**< Number of followup message mismatch */
};
/** Master clocks information TLV
 * @note linuxptp implementation specific
 */
strc(UNICAST_MASTER_TABLE_NP_t) sz(: public BaseMngTlv) {
    Integer16_t actualTableSize; /**< Number of addresses table */
    /** unicast masters table */
    vec(LinuxptpUnicastMaster_t)unicastMasters;
};
/** Port Linux hardware clock properties
 * @note linuxptp implementation specific
 */
strc(PORT_HWCLOCK_NP_t) sz(: public BaseMngTlv) {
    strcc(PortIdentity_t) portIdentity; /**< Port ID */
    Integer32_t phc_index; /**< Linux PTP hardware clokc index */
    UInteger8_t flags; /**< Flags */
};
/** Power system profile TLV
 * @note linuxptp implementation specific
 */
strc(POWER_PROFILE_SETTINGS_NP_t) sz(: public BaseMngTlv) {
    /** Profile version */
    enmc(linuxptpPowerProfileVersion_e) version;
    UInteger16_t grandmasterID; /**< grand master ID */
    UInteger32_t grandmasterTimeInaccuracy; /**< grand master time inaccuracy */
    UInteger32_t networkTimeInaccuracy; /**< network time inaccuracy */
    UInteger32_t totalTimeInaccuracy; /**< total time inaccuracy */
};
/** Common Mean Link Delay Information TLV
 * @note linuxptp implementation specific
 */
strc(CMLDS_INFO_NP_t) sz(: public BaseMngTlv) {
    strcc(TimeInterval_t) meanLinkDelay; /**< Mean link delay */
    /** scaled neighbor rate ratio */
    Integer32_t scaledNeighborRateRatio;
    uint32_t as_capable; /**< Capable */
};

ns_e()
cpp_en(proc)dnl

ice(PROC)
