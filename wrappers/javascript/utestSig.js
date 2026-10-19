/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2026 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 *
 * @brief Unit test for signaling with JavaScript wrapper of libptpmgmt
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
const utest_help = require(ADD_PATH + 'utest_help');

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
var buf = new ptpmgmt.Buf(100);
var size = utest_help.get2MngTlvsSig(buf.get(), buf.size());
assert(size > 0, "get2MngTlvsSig");
var prms = msg.getParams();
prms.rcvSignaling = true;
prms.filterSignaling = false;
assertEquals(msg.parseBuf(buf, size), ptpmgmt.MNG_PARSE_ERROR_SIG, "parse");
assertEquals(msg.getSigTlvsCount(), 2, "getSigTlvsCount");
assertEquals(msg.getSigTlvType(0), ptpmgmt.MANAGEMENT, "getSigTlvType");
assertEquals(msg.getSigMngTlvType(0), ptpmgmt.PRIORITY2, "getSigMngTlvType");
assertEquals(msg.getSigTlvType(1), ptpmgmt.MANAGEMENT, "getSigTlvType");
assertEquals(msg.getSigMngTlvType(1), ptpmgmt.DOMAIN, "getSigMngTlvType");
var sigTlv = msg.getSigTlv(0);
assertEquals(ptpmgmt.get_MngTlvId(sigTlv), ptpmgmt.PRIORITY2, "get_MngTlvId");
var mngTlv1_1 = msg.getSigMngTlv(0);
var mngTlv1_2 = ptpmgmt.get_BaseMngTlv(sigTlv);
var pr2_1 = ptpmgmt.conv_PRIORITY2(mngTlv1_1);
var pr2_2 = ptpmgmt.conv_PRIORITY2(mngTlv1_2);
assertEquals(pr2_1.priority2, 119, "pr2_1.priority2");
assertEquals(pr2_2.priority2, 119, "pr2_2.priority2");
sigTlv = msg.getSigTlv(1);
assertEquals(ptpmgmt.get_MngTlvId(sigTlv), ptpmgmt.DOMAIN, "get_MngTlvId");
mngTlv1_1 = msg.getSigMngTlv(1);
mngTlv1_2 = ptpmgmt.get_BaseMngTlv(sigTlv);
var domain1 = ptpmgmt.conv_DOMAIN(mngTlv1_1);
var domain2 = ptpmgmt.conv_DOMAIN(mngTlv1_2);
assertEquals(domain1.domainNumber, 7, "domain1.domainNumber");
assertEquals(domain2.domainNumber, 7, "domain2.domainNumber");
// MessageSigTlvCallback - JavaScript do not support directors!
