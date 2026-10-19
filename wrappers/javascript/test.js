/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2026 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief testing for JavaScript wrapper of libptpmgmt
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2026 Erez Geva
 *
 */

/* Bridging from Node.js to JavaScriptCore setting */
var prePath = '';
if(typeof process != 'undefined' && process != null ) {
  ENGINE = process.env.ENGINE;
  args = process.argv.slice(1);
  prePath = './' + ENGINE + '/';
}

const ptpmgmt = require(prePath + 'ptpmgmt.node');
console.log(`ptpmgmt version ${ptpmgmt.getVersion()}`);

// @  class myDisp extends MessageDispatcher {
// @    public void PRIORITY1_h(Message msg, PRIORITY1_t tlv, String tlv_id) {
// @      System.out.println("Get reply for " + tlv_id);
// @      System.out.println("priority1: " + tlv.getPriority1());
// @    }
// @    public void USER_DESCRIPTION_h(Message msg,
// @                                   USER_DESCRIPTION_t tlv, String tlv_id) {
// @      System.out.println("Get reply for " + tlv_id);
// @      System.out.println("get user desc: " +
// @        tlv.getUserDescription().getTextField());
// @    }
// @  }
// @
// @  class myBuild extends MessageBuilder {
// @    public short pr;
// @    public myBuild(Message msg) {
// @      super(msg);
// @    }
// @    public boolean PRIORITY1_b(Message msg, PRIORITY1_t tlv) {
// @      tlv.setPriority1(this.pr);
// @      return true;
// @    }
// @  }
// @
// @  final static BigInteger b500 = BigInteger.valueOf(500);
// @  public SockUnix sk = new SockUnix();
// @  Message msg = new Message();
// @  Buf buf = new Buf(1000);
// @  myDisp dispacher = new myDisp();
// @  myBuild builder = new myBuild(msg);
// @  int sequence = 0;
// @
// @  int printError(String msg) {
// @    if(c_error.isError())
// @      System.err.println(c_error.getError());
// @    else
// @      System.err.println(msg);
// @    return -1;
// @  }
// @
// @  int nextSequence() {
// @    // Ensure sequence in in range of unsigned 16 bits
// @    sequence += 1;
// @    if(sequence > 0xffff)
// @      sequence = 1;
// @    return sequence;
// @  }
// @
// @  int setPriority1(int p) {
// @    short newPriority1 = (short)p;
// @    boolean useBuild = true;
// @    mng_vals_e id = mng_vals_e.PRIORITY1;
// @    if (useBuild) {
// @      builder.pr = newPriority1;
// @      builder.buildTlv(actionField_e.SET, id);
// @    } else {
// @      PRIORITY1_t pr1 = new PRIORITY1_t();
// @      pr1.setPriority1(newPriority1);
// @      msg.setAction(actionField_e.SET, id, pr1);
// @    }
// @    int seq = nextSequence();
// @    MNG_PARSE_ERROR_e err = msg.build(buf, seq);
// @    if(err != MNG_PARSE_ERROR_e.MNG_PARSE_ERROR_OK) {
// @      String txt = Message.err2str_c(err);
// @      System.err.println("build error " + txt);
// @    }
// @    if(!sk.send(buf, msg.getMsgLen()))
// @      return printError("send fail");
// @    msg.clearData();
// @    if(!sk.poll(b500))
// @      return printError("timeout");
// @    int cnt = sk.rcvBuf(buf);
// @    if(cnt <= 0)
// @      return printError("rcv error " + cnt);
// @    err = msg.parse(buf, cnt);
// @    if(err != MNG_PARSE_ERROR_e.MNG_PARSE_ERROR_OK || msg.getTlvId() != id ||
// @       seq != msg.getSequence()) {
// @      System.err.println("set fails");
// @      return -1;
// @    }
// @    System.out.println("set new priority " + newPriority1 + " success");
// @    msg.setAction(actionField_e.GET, id);
// @    seq = nextSequence();
// @    err = msg.build(buf, seq);
// @    if(err != MNG_PARSE_ERROR_e.MNG_PARSE_ERROR_OK) {
// @      String txt = Message.err2str_c(err);
// @      System.err.println("build error " + txt);
// @    }
// @    if(!sk.send(buf, msg.getMsgLen()))
// @      return printError("send fail");
// @    if(!sk.poll(b500))
// @      return printError("timeout");
// @    cnt = sk.rcvBuf(buf);
// @    if(cnt <= 0)
// @      return printError("rcv error " + cnt);
// @    err = msg.parse(buf, cnt);
// @    if(err == MNG_PARSE_ERROR_e.MNG_PARSE_ERROR_MSG)
// @      System.err.println("error message");
// @    else if(err != MNG_PARSE_ERROR_e.MNG_PARSE_ERROR_OK)
// @      System.err.println("parse error " + Message.err2str_c(err));
// @    else {
// @      dispacher.callHadler(msg, msg.getTlvId(), msg.getData());
// @      return 0;
// @    }
// @    return -1;
// @  }
// @
// @  public int body(String cfg_file)
// @  {
// @    if(!buf.isAlloc()) {
// @      System.err.println("buffer allocation failed");
// @      return -1;
// @    }
// @    ConfigFile cfg = new ConfigFile();
// @    if(!cfg.read_cfg(cfg_file))
// @      return printError("fail reading configuration file");
// @    if(!sk.setDefSelfAddress() || !sk.init() || !sk.setPeerAddress(cfg))
// @      return printError("fail init socket");
// @    MsgParams prms = msg.getParams();
// @    if(cfg.haveSpp()) {
// @      if(!msg.useAuth(cfg))
// @        return printError("fail using Authentication");
// @      prms.setSendAuth(true);
// @    }
// @    int pid = (int)(ProcessHandle.current().pid() & 0xffffL);
// @    prms.getSelf_id().setPortNumber(pid);
// @    // Verify we can use implementSpecific_e;
// @    prms.setImplementSpecific(implementSpecific_e.linuxptp);
// @    prms.setDomainNumber(cfg.domainNumber());
// @    msg.updateParams(prms);
// @    msg.useConfig(cfg);
// @    mng_vals_e id = mng_vals_e.USER_DESCRIPTION;
// @    msg.setAction(actionField_e.GET, id);
// @    int seq = nextSequence();
// @    MNG_PARSE_ERROR_e err = msg.build(buf, seq);
// @    if(err != MNG_PARSE_ERROR_e.MNG_PARSE_ERROR_OK) {
// @      System.err.println("build error " + Message.err2str_c(err));
// @      return -1;
// @    }
// @    if(!sk.send(buf, msg.getMsgLen()))
// @      return printError("send fail");
// @    // You can get file descriptor with sk.fileno() and use select;
// @    if(!sk.poll(b500))
// @      return printError("timeout");
// @    int cnt = sk.rcvBuf(buf);
// @    if(cnt <= 0)
// @      return printError("rcv error " + cnt);
// @    err = msg.parse(buf, cnt);
// @    if(err == MNG_PARSE_ERROR_e.MNG_PARSE_ERROR_MSG)
// @      System.err.println("error message");
// @    else if(err != MNG_PARSE_ERROR_e.MNG_PARSE_ERROR_OK)
// @      System.err.println("parse error " + Message.err2str_c(err));
// @    else
// @      dispacher.callHadler(msg);
// @
// @    // test setting values;
// @    CLOCK_DESCRIPTION_t clk_dec = new CLOCK_DESCRIPTION_t();
// @    clk_dec.setClockType((int)clockType_e.ordinaryClock.swigValue() /*0x800*/);
// @    Binary physicalAddress = new Binary();
// @    physicalAddress.setBin(0L, (short)0xf1);
// @    physicalAddress.setBin(1L, (short)0xf2);
// @    physicalAddress.setBin(2L, (short)0xf3);
// @    physicalAddress.setBin(3L, (short)0xf4);
// @    System.out.println("physicalAddress: " + physicalAddress.toId());
// @    System.out.println("physicalAddress: " + physicalAddress.toHex());
// @    clk_dec.getPhysicalAddress().setBin(0L, (short)0xf1);
// @    clk_dec.getPhysicalAddress().setBin(1L, (short)0xf2);
// @    clk_dec.getPhysicalAddress().setBin(2L, (short)0xf3);
// @    clk_dec.getPhysicalAddress().setBin(3L, (short)0xf4);
// @    System.out.println("clk.physicalAddress: " +
// @        clk_dec.getPhysicalAddress().toId());
// @    System.out.println("clk.physicalAddress: " +
// @        clk_dec.getPhysicalAddress().toHex());
// @    System.out.println("manufacturerIdentity: " +
// @        Binary.bufToId(clk_dec.getManufacturerIdentity(), 3));
// @    clk_dec.getRevisionData().setTextField("This is a test");
// @    System.out.println("revisionData: " +
// @        clk_dec.getRevisionData().getTextField());
// @
// @    // test send;
// @    setPriority1(147);
// @    setPriority1(153);
// @
// @    SUBSCRIBE_EVENTS_NP_t event = new SUBSCRIBE_EVENTS_NP_t();
// @    event.setEvent(ptpmgmt.getNOTIFY_TIME_SYNC());
// @    System.out.println("getEvent(NOTIFY_TIME_SYNC)=" +
// @         (event.getEvent(ptpmgmt.getNOTIFY_TIME_SYNC()) ? "have" : "not"));
// @    System.out.println("getEvent(NOTIFY_PORT_STATE)=" +
// @         (event.getEvent(ptpmgmt.getNOTIFY_PORT_STATE()) ? "have" : "not"));
// @
// @    // test SigEvent that represent std.vector<SLAVE_TX_EVENT_TIMESTAMPS_rec_t>
// @    // See std_vectors.md for more information
// @    SigEvent evnts = new SigEvent();
// @    SLAVE_TX_EVENT_TIMESTAMPS_rec_t e = new SLAVE_TX_EVENT_TIMESTAMPS_rec_t();
// @    e.setSequenceId(1);
// @    e.getEventEgressTimestamp().fromFloat(4.5);
// @    evnts.add(e);
// @    System.out.printf("Events size %d, seq[0]=%d, ts[0]=%s\n", evnts.size(),
// @        evnts.get(0).getSequenceId(),
// @        evnts.get(0).getEventEgressTimestamp().string());
// @    return 0;
// @  }
// @
// @  public static void main(String argv[]) {
// @    Options opt = new Options();
// @    if(opt.parse_options(argv) == Options.loop_val.OPT_DONE) {
// @      String cfg_file = opt.val('f');
// @      if(cfg_file.isEmpty())
// @        cfg_file = DEF_CFG_FILE;
// @      System.out.println("Use configuration file " + cfg_file);
// @      testing t = new testing();
// @      t.body(cfg_file);
// @      t.sk.close();
// @    } else
// @      System.err.println("fail parsing command line");
// @  }
