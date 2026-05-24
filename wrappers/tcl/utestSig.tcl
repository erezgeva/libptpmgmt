#!/usr/bin/tclsh
# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Copyright © 2026 Erez Geva <ErezGeva2@gmail.com>
#
# Unit test for signaling with tcl wrapper of libptpmgmt
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2026 Erez Geva
#
###############################################################################

package require ptpmgmt
package require utest_help
package require tcltest
# See: man tcltest
#      https://wiki.tcl-lang.org/page/tcltest

###############################################################################

tcltest::test testPtpmgmtTraverseSig { Test receive signalling message
} -setup {
    variable msg
    variable buf
    variable prms
    set msg [ ptpmgmt::Message new ]
    set buf [ ptpmgmt::Buf new 100 ]
    set prms [ $msg getParams ]
    $prms configure -rcvSignaling true
    $prms configure -filterSignaling false
} -body {
    set ret1 [ $msg updateParams $prms ]
    set size [ utest_help::get2MngTlvsSig [ $buf get ] [ $buf size ]  ]
    # ptpmgmt.MNG_PARSE_ERROR_SIG == 2
    set ret2 [ $msg parse $buf $size ]
    # ptpmgmt.MANAGEMENT == 1
    set typ1_0 [ $msg getSigTlvType 0 ]
    set typ1_1 [ $msg getSigTlvType 1 ]
    # ptpmgmt.PRIORITY2 = 14 (in library enumerator!)
    set mtyp1_0 [ $msg getSigMngTlvType 0 ]
    # ptpmgmt.DOMAIN = 15 (in library enumerator!)
    set mtyp1_1 [ $msg getSigMngTlvType 1 ]
    set sigTlv0 [ $msg getSigTlv 0 ]
    # ptpmgmt.PRIORITY2 = 14 (in library enumerator!)
    set mtyp2_0 [ ptpmgmt::get_MngTlvId $sigTlv0 ]
    set mngTlv1_0 [ $msg getSigMngTlv 0 ]
    set mngTlv2_0 [ ptpmgmt::get_BaseMngTlv $sigTlv0 ]
    # priority2 == 119
    set pr2_0 [ ptpmgmt::conv_PRIORITY2 $mngTlv1_0 ]
    set pr2_1 [ ptpmgmt::conv_PRIORITY2 $mngTlv2_0 ]
    set sigTlv1 [ $msg getSigTlv 1 ]
    # ptpmgmt.DOMAIN = 15 (in library enumerator!)
    set mtyp2_1 [ ptpmgmt::get_MngTlvId $sigTlv1 ]
    set mngTlv1_1 [ $msg getSigMngTlv 1 ]
    set mngTlv2_1 [ ptpmgmt::get_BaseMngTlv $sigTlv1 ]
    # domainNumber == 7
    set domain1 [ ptpmgmt::conv_DOMAIN $mngTlv1_1 ]
    set domain2 [ ptpmgmt::conv_DOMAIN $mngTlv2_1 ]
    return [list $ret1 $size $ret2 [ $msg getSigTlvsCount ] $typ1_0 $typ1_1 $mtyp1_0 $mtyp1_1\
            $mtyp2_0 [ $pr2_0 cget -priority2] [ $pr2_1 cget -priority2] $mtyp2_1\
            [ $domain1 cget -domainNumber] [ $domain2 cget -domainNumber]]
} -result {1 60 2 2 1 1 14 15 14 119 119 15 7 7}

###############################################################################

# Hook to determine if any of the tests failed. Then we can exit with
# proper exit code: 0=all passed, 1=one or more failed
proc tcltest::cleanupTestsHook {} {
    variable numTests
    set ::exitCode [expr {$numTests(Failed) > 0}]
}

tcltest::runAllTests
exit $exitCode
