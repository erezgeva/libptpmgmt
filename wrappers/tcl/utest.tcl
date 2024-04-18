#!/usr/bin/tclsh
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com>
#
# Unit test with tcl wrapper of libptpmgmt
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2022 Erez Geva
#
###############################################################################

package require ptpmgmt
package require tcltest
# See: man tcltest
#      https://wiki.tcl-lang.org/page/tcltest

::oo::class create myDisp {
  superclass ptpmgmt::MessageDispatcher
  variable _priority1
  variable _func
  variable _id
  method priority1 {} {
    return $_priority1
  }
  method func {} {
    return $_func
  }
  method id {} {
    return $_id
  }
  constructor {} {
    set _priority1 0
    set _func 0
    set _id ""
  }
  method PRIORITY1_h {msg tlv tlv_id} {
    set _priority1 [ $tlv cget -priority1 ]
    set _func [ expr { $_func + 1 } ]
    set _id $tlv_id
  }
  method noTlv {msg} {
    set _func [ expr { $_func + 2 } ]
  }
  method noTlvCallBack {msg tlv_id} {
    set _func [ expr { $_func + 4 } ]
    set _id $tlv_id
  }
}
::oo::class create myBuild {
  superclass ptpmgmt::MessageBuilder
  variable _run
  constructor {msg} {
    next {*}$msg
    set _run 0
  }
  method run {} {
    return $_run
  }
  method PRIORITY1_b { msg tlv } {
    set _run 1
    $tlv configure -priority1 117
    return true
  }
}

###############################################################################

tcltest::test testParsedCallHadlerEmptyTLV {Tests callHadler with empty TLV
} -setup {
    variable msg
    variable disp
    set msg [ ptpmgmt::Message new ]
    set disp [ myDisp new ]
} -body {
    $disp callHadler $msg
    return [list [ $disp func ] [ $disp priority1 ] [ $disp id ] ]
    # Call noTlv method
} -result {2 0 {}}

tcltest::test testParsedCallHadlerTLV {Tests callHadler method with TLV
} -setup {
    variable msg
    variable disp
    variable tlv
    set msg [ ptpmgmt::Message new ]
    set disp [ myDisp new ]
    set tlv [ ptpmgmt::PRIORITY1_t ]
    $tlv configure -priority1 117
} -body {
    $disp callHadler $msg $ptpmgmt::PRIORITY1 $tlv
    return [list [ $disp func ] [ $disp priority1 ] [ $disp id ] ]
    # Call PRIORITY1_h method
} -result {1 117 PRIORITY1}

tcltest::test testParsedCallHadlerTLVNoCallback {Tests callHadler method with TLV without callback
} -setup {
    variable msg
    variable disp
    variable tlv
    set msg [ ptpmgmt::Message new ]
    set disp [ myDisp new ]
    set tlv [ ptpmgmt::PRIORITY2_t ]
    $tlv configure -priority2 117
} -body {
    $disp callHadler $msg $ptpmgmt::PRIORITY2 $tlv
    return [list [ $disp func ] [ $disp priority1 ] [ $disp id ] ]
    # Call noTlvCallBack method
} -result {4 0 PRIORITY2}

tcltest::test test_buildEmptyTLV {Tests build empty TLV
} -setup {
    variable msg
    variable build
    set msg [ ptpmgmt::Message new ]
    set build [ myBuild new $msg ]
} -body {
    set ret [ $build buildTlv $ptpmgmt::COMMAND $ptpmgmt::ENABLE_PORT ]
    set ret2 [ $build run ]
    $build clear
    return [list $ret $ret2]
    # Call setAction without a TLV
    # SWIG convert C++ 'true' to TCL '1'
} -result {1 0}

tcltest::test test_buildTLV {Tests build TLV
} -setup {
    variable msg
    variable build
    set msg [ ptpmgmt::Message new ]
    set build [ myBuild new $msg ]
} -body {
    set ret [ $build buildTlv $ptpmgmt::SET $ptpmgmt::PRIORITY1 ]
    set ret2 [ $build run ]
    $build clear
    return [list $ret $ret2]
    # Call setAction with a TLV success
    # ptpmgmt::MessageBuilder return TCL 'true'
} -result {true 1}

tcltest::test test_buildTLVNoCallback {Tests build TLV that lack callback
} -setup {
    variable msg
    variable build
    set msg [ ptpmgmt::Message new ]
    set build [ myBuild new $msg ]
} -body {
    set ret [ $build buildTlv $ptpmgmt::SET $ptpmgmt::PRIORITY2 ]
    set ret2 [ $build run ]
    $build clear
    return [list $ret $ret2]
    # No callback, function fails
    # ptpmgmt::MessageBuilder return TCL 'false'
} -result {false 0}

###############################################################################

# Hook to determine if any of the tests failed. Then we can exit with
# proper exit code: 0=all passed, 1=one or more failed
proc tcltest::cleanupTestsHook {} {
    variable numTests
    set ::exitCode [expr {$numTests(Failed) > 0}]
}

tcltest::runAllTests
exit $exitCode
