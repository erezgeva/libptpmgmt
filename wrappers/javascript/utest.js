/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2026 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 *
 * @brief Unit test with JavaScript wrapper of libptpmgmt
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2026 Erez Geva
 *
 */

if(typeof process != 'undefined' && process != null) {
  // Bridging Node.js
  ADD_PATH = process.env.ADD_PATH;
  args = process.argv.slice(1);
}
if(typeof ADD_PATH === 'undefined' && ADD_PATH == null)
  ADD_PATH = '';
const ptpmgmt = require(ADD_PATH + 'ptpmgmt');

class myDisp extends ptpmgmt.MessageDispatcher {
  priority1; // integer
  func;      // integer
  id;        // string
  clear() {
    this.priority1 = 0;
    this.func = 0;
    this.id = '';
  }
  PRIORITY1_h(msg, tlv, idStr) {
    this.func |= 0x1;
    this.id = idStr;
    this.priority1 = tlv.priority1;
  }
  noTlv(msg) { this.func |= 0x2; }
  noTlvCallBack(msg, idStr) {
    this.func |= 0x4;
    this.id = idStr;
  }
}

class myBuild extends ptpmgmt.MessageBuilder {
  run = 0;
  PRIORITY1_b(msg, tlv) {
    this.run = 1;
    tlv.priority1 = 117;
    return true;
  }
}

function assert(condition, msg) {
  if(!condition) {
    if(typeof msg === 'undefined')
      throw new Error('Assertion failed');
    else
      throw new Error(msg);
  }
}
function assertEquals(a, b, msg) { assert(a === b, msg); }

var msg = new ptpmgmt.Message;
var disp = new myDisp;
var build = new myBuild(msg);

// Tests callHadler with empty TLV
disp.clear();
disp.callHadler(msg);
assertEquals(disp.func, 0x2, 'should call noTlv');
assertEquals(disp.id, '', 'should have empty ID');
assertEquals(disp.priority1, 0, 'should not have priority1 value');

// Tests callHadler method with TLV
var tlv = new ptpmgmt.PRIORITY1_t;
tlv.priority1 = 117;
disp.clear();
disp.callHadler(msg, ptpmgmt.PRIORITY1, tlv);
assertEquals(disp.func, 0x1, 'should call PRIORITY1_h');
assertEquals(disp.id, 'PRIORITY1', 'should have PRIORITY1 ID');
assertEquals(disp.priority1, 117, 'should have priority1 value');

// Tests callHadler method with TLV without callback
tlv = new ptpmgmt.PRIORITY2_t;
tlv.priority2 = 117;
disp.clear();
disp.callHadler(msg, ptpmgmt.PRIORITY2, tlv);
assertEquals(disp.func, 0x4, 'should call noTlvCallBack');
assertEquals(disp.id, 'PRIORITY2', 'should have PRIORITY2 ID');
assertEquals(disp.priority1, 0, "shouldn't have priority1 value");

// Tests build empty TLV
build.run = 0;
assert(build.buildTlv(ptpmgmt.COMMAND, ptpmgmt.ENABLE_PORT), 'should pass');
assertEquals(build.run, 0, "shouldn't call PRIORITY1 callback");
build.clear();

// Tests build TLV
build.run = 0;
assert(build.buildTlv(ptpmgmt.SET, ptpmgmt.PRIORITY1), 'should pass');
assertEquals(build.run, 1, 'should call PRIORITY1 callback');
build.clear();

// Tests build TLV that lack callback
build.run = 0;
assert(!build.buildTlv(ptpmgmt.SET, ptpmgmt.PRIORITY2), 'should no pass');
assertEquals(build.run, 0, "shouldn't call PRIORITY1 callback");
build.clear();
