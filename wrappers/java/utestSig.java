/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2026 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Unit test for signaling with Java wrapper of libptpmgmt
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2026 Erez Geva
 *
 */

import static org.junit.jupiter.api.Assertions.*;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.BeforeAll;
import utest_help.*;
import ptpmgmt.*;

/* This is a bypass to get Buf C pointer!
 * This is NOT official interface.
 * Production code should NOT use this method!
 */
class myBuf extends Buf {
  public static long getCPtr(Buf o) {
    return Buf.getCPtr(o);
  }
}

class mySigCb extends MessageSigTlvCallback {
  public int mask = 0;
  public mySigCb() {
      super();
  }
  public boolean callback(Message msg, tlvType_e tlvType, BaseSigTlv sigTlv) {
    if(tlvType != tlvType_e.MANAGEMENT)
      return true; // return true on failure!
    mng_vals_e id = ptpmgmt.get_MngTlvId(sigTlv);
    BaseMngTlv tlv = ptpmgmt.get_BaseMngTlv(sigTlv);
    // First TLV
    if(id == mng_vals_e.PRIORITY2) {
      PRIORITY2_t pr2 = ptpmgmt.conv_PRIORITY2(tlv);
      this.mask += 1;
      return pr2.getPriority2() != 119; // return false on success!
    }
    // Second TLV
    if(id == mng_vals_e.DOMAIN) {
      DOMAIN_t domain = ptpmgmt.conv_DOMAIN(tlv);
      this.mask += 10;
      return domain.getDomainNumber() != 7; // return false on success!
    }
    return true; // return true on failure!
  }
}

public class utestSig {
  @BeforeAll
  static void loadPtpmgmt() {
    try {
      System.loadLibrary("ptpmgmt");
    } catch (UnsatisfiedLinkError e) {
      System.err.println("Fail to load ptpmgmt Native library:\n" + e);
      System.exit(1);
    }
    try {
      System.loadLibrary("utest_help");
    } catch (UnsatisfiedLinkError e) {
      System.err.println("Fail to load utest Native library:\n" + e);
      System.exit(1);
    }
  }

  @Test
  void testPtpmgmtTraverseSig() {
    Message msg = new Message();
    Buf buf = new Buf(100);
    long size = utest_help.get2MngTlvsSigCPtr(myBuf.getCPtr(buf));
    assertTrue(size > 0, "get2MngTlvsSig");
    MsgParams prms = msg.getParams();
    prms.setRcvSignaling(true);
    prms.setFilterSignaling(false);
    assertTrue(msg.updateParams(prms), "updateParams");
    assertEquals(msg.parse(buf, (int)size), MNG_PARSE_ERROR_e.MNG_PARSE_ERROR_SIG, "parse");
    assertEquals(msg.getSigTlvsCount(), 2, "getSigTlvsCount");
    assertEquals(msg.getSigTlvType(0), tlvType_e.MANAGEMENT, "getSigTlvType");
    assertEquals(msg.getSigMngTlvType(0), mng_vals_e.PRIORITY2, "getSigMngTlvType");
    assertEquals(msg.getSigTlvType(1), tlvType_e.MANAGEMENT, "getSigTlvType");
    assertEquals(msg.getSigMngTlvType(1), mng_vals_e.DOMAIN, "getSigMngTlvType");
    BaseSigTlv sigTlv = msg.getSigTlv(0);
    assertEquals(ptpmgmt.get_MngTlvId(sigTlv), mng_vals_e.PRIORITY2, "get_MngTlvId");
    BaseMngTlv mngTlv1_1 = msg.getSigMngTlv(0);
    BaseMngTlv mngTlv1_2 = ptpmgmt.get_BaseMngTlv(sigTlv);
    PRIORITY2_t pr2_1 = ptpmgmt.conv_PRIORITY2(mngTlv1_1);
    PRIORITY2_t pr2_2 = ptpmgmt.conv_PRIORITY2(mngTlv1_2);
    assertEquals(pr2_1.getPriority2(), 119, "pr2_1.priority2");
    assertEquals(pr2_2.getPriority2(), 119, "pr2_2.priority2");
    sigTlv = msg.getSigTlv(1);
    assertEquals(ptpmgmt.get_MngTlvId(sigTlv), mng_vals_e.DOMAIN, "get_MngTlvId");
    mngTlv1_1 = msg.getSigMngTlv(1);
    mngTlv1_2 = ptpmgmt.get_BaseMngTlv(sigTlv);
    DOMAIN_t domain1 = ptpmgmt.conv_DOMAIN(mngTlv1_1);
    DOMAIN_t domain2 = ptpmgmt.conv_DOMAIN(mngTlv1_2);
    assertEquals(domain1.getDomainNumber(), 7, "domain1.domainNumber");
    assertEquals(domain2.getDomainNumber(), 7, "domain2.domainNumber");
    mySigCb cb = new mySigCb();
    assertFalse(msg.traversSigTlvsCl(cb), "traversSigTlvsCl");
    assertEquals(cb.mask, 11, "mySigCb mask");
  }
}

/*

mkdir -p utestSig && javac -d utestSig -cp ptpmgmt.jar:utest_help.jar:/usr/share/java/junit-platform-console-standalone.jar utestSig.java

*/
