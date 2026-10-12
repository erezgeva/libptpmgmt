/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2026 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Unit test with Java wrapper of libptpmgmt
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2026 Erez Geva
 *
 */

import static org.junit.jupiter.api.Assertions.*;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.BeforeAll;
import ptpmgmt.*;

class myDisp extends MessageDispatcher {
  public int func;
  public int priority1;
  public String id;

  public void clear() {
    this.priority1 = 0;
    this.func = 0;
    this.id = "";
  }
  public myDisp() {
    super();
  }
  public void PRIORITY1_h(Message msg, PRIORITY1_t tlv, String idStr) {
    this.func |= 0x1;
    this.id = idStr;
    this.priority1 = tlv.getPriority1();
  }
  public void noTlv(Message msg) {
    this.func |= 0x2;
  }
  public void noTlvCallBack(Message msg, String idStr) {
    this.func |= 0x4;
    this.id = idStr;
  }
}

class myBuild extends MessageBuilder {
  public int run = 0;
  public myBuild(Message msg) {
    super(msg);
  }
  public boolean PRIORITY1_b(Message msg, PRIORITY1_t tlv) {
    this.run = 1;
    tlv.setPriority1((short)117);
    return true;
  }
}

public class utest {

  static private Message msg;
  static private myDisp disp;
  static private myBuild build;

  @BeforeAll
  static void loadPtpmgmt() {
    try {
      System.loadLibrary("ptpmgmt");
    } catch (UnsatisfiedLinkError e) {
      System.err.println("Fail to load Native library:\n" + e);
      System.exit(1);
    }
    msg = new Message();
    disp = new myDisp();
    build = new myBuild(msg);
  }

  @Test
  void test_parsedCallHadlerEmptyTLV() {
    /* Tests callHadler with empty TLV */
    this.disp.clear();
    this.disp.callHadler(this.msg);
    assertEquals(this.disp.func, 0x2L, "should call noTlv");
    assertEquals(this.disp.id, "", "should have empty ID");
    assertEquals(this.disp.priority1, 0, "should not have priority1 value");
  }

  @Test
  void test_parsedCallHadlerTLV() {
    /* Tests callHadler method with TLV */
    PRIORITY1_t tlv = new PRIORITY1_t();
    tlv.setPriority1((short)117);
    this.disp.clear();
    this.disp.callHadler(this.msg, mng_vals_e.PRIORITY1, tlv);
    assertEquals(this.disp.func, 0x1L, "should call PRIORITY1_h");
    assertEquals(this.disp.id, "PRIORITY1", "should have PRIORITY1 ID");
    assertEquals(this.disp.priority1, 117, "should have priority1 value");
  }

  @Test
  void test_parsedCallHadlerTLVNoCallback() {
    /* Tests callHadler method with TLV without callback */
    PRIORITY2_t tlv = new PRIORITY2_t();
    tlv.setPriority2((short)117);
    this.disp.clear();
    this.disp.callHadler(msg, mng_vals_e.PRIORITY2, tlv);
    assertEquals(this.disp.func, 0x4, "should call noTlvCallBack");
    assertEquals(this.disp.id, "PRIORITY2", "should have PRIORITY2 ID");
    assertEquals(this.disp.priority1, 0, "shouldn't have priority1 value");
  }

  @Test
  void test_buildEmptyTLV() {
    /* Tests build empty TLV */
    this.build.run = 0;
    assertTrue(this.build.buildTlv(actionField_e.COMMAND, mng_vals_e.ENABLE_PORT), "should pass");
    assertEquals(this.build.run, 0, "shouldn't call PRIORITY1 callback");
    this.build.clear();
  }

  @Test
  void test_buildTLV() {
    /* Tests build TLV */
    this.build.run = 0;
    assertTrue(this.build.buildTlv(actionField_e.SET, mng_vals_e.PRIORITY1), "should pass");
    assertEquals(this.build.run, 1, "should call PRIORITY1 callback");
    this.build.clear();
  }

  @Test
  void test_buildTLVNoCallback() {
    /* Tests build TLV that lack callback */
    assertFalse(this.build.buildTlv(actionField_e.SET, mng_vals_e.PRIORITY2), "should no pass");
    assertEquals(this.build.run, 0, "shouldn't call PRIORITY1 callback");
    this.build.clear();
  }
}


/*

mkdir -p utest && javac -d utest -cp ptpmgmt.jar:/usr/share/java/junit-platform-console-standalone.jar utest.java &&\
LD_LIBRARY_PATH=../../.libs/:. java -Djava.library.path=. -jar /usr/share/java/junit-platform-console-standalone.jar execute -cp ptpmgmt.jar:utest -c utest

*/
