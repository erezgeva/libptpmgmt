/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2026 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 *
 * @brief testing for JavaScript wrapper of libptpmgmt
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2026 Erez Geva
 *
 */

if(typeof process != 'undefined' && process != null) {
  // Bridging Node.js
  ADD_PATH = process.env.ADD_PATH;
  args = process.argv.slice(1);
  getpid = function() { return process.pid; }
}
if(typeof ADD_PATH === 'undefined' && ADD_PATH == null)
  ADD_PATH = '';
const ptpmgmt = require(ADD_PATH + 'ptpmgmt');

class myDisp extends ptpmgmt.MessageDispatcher {
  PRIORITY1_h(msg, tlv, tlv_id) {
    console.log('Get reply for ' + tlv_id);
    console.log('priority1: ' + tlv.priority1);
  }
  USER_DESCRIPTION_h(msg, tlv, tlv_id) {
    console.log('Get reply for ' + tlv_id);
    console.log('get user desc: ' + tlv.userDescription.textField);
  }
}
class myBuild extends ptpmgmt.MessageBuilder {
  pr; // integer
  PRIORITY1_b(msg, tlv) {
    tlv.priority1 = this.pr;
    return true;
  }
}

const DEF_CFG_FILE = '/etc/linuxptp/ptp4l.conf';
var sk = new ptpmgmt.SockUnix;
var msg = new ptpmgmt.Message;
var buf = new ptpmgmt.Buf(1000);
var dispacher = new myDisp;
var builder = new myBuild(msg);
var sequence = 0;

function printError(message) {
  if(ptpmgmt.Error.isError())
    message = ptpmgmt.Error.getError();
  throw new Error(message);
}
function nextSequence() {
  // Ensure sequence in in range of unsigned 16 bits
  sequence += 1;
  if(sequence > 0xffff)
    sequence = 1;
  return sequence;
}
function setPriority1(newPriority1) {
  const useBuild = true;
  const id = ptpmgmt.PRIORITY1;
  if(useBuild) {
    builder.pr = newPriority1;
    builder.buildTlv(ptpmgmt.SET, id);
  } else {
    let pr1 = new ptpmgmt.PRIORITY1_t();
    pr1.priority1 = newPriority1;
    msg.setAction(ptpmgmt.SET, id, pr1);
  }
  let seq = nextSequence();
  let err = msg.build(buf, seq);
  if(err != ptpmgmt.MNG_PARSE_ERROR_OK) {
    let txt = ptpmgmt.Message.err2str_c(err);
    throw new Error('build error ' + txt);
  }
  if(!sk.sendBuf(buf, msg.getMsgLen()))
    printError('send fail');
  msg.clearData();
  if(!sk.poll(500))
    printError('timeout');
  let cnt = sk.rcvBuf(buf);
  if(cnt <= 0)
    printError('rcv error ' + cnt);
  err = msg.parseBuf(buf, cnt);
  if(err != ptpmgmt.MNG_PARSE_ERROR_OK || msg.getTlvId() != id ||
     seq != msg.getSequence())
    throw new Error('set fails');
  console.log(`set new priority ${newPriority1} success`);
  msg.setAction(ptpmgmt.GET, id);
  seq = nextSequence();
  err = msg.build(buf, seq);
  if(err != ptpmgmt.MNG_PARSE_ERROR_OK) {
    let txt = ptpmgmt.Message.err2str_c(err);
    throw new Error('build error ' + txt);
  }
  if(!sk.sendBuf(buf, msg.getMsgLen()))
    printError('send fail');
  if(!sk.poll(500))
    printError('timeout');
  cnt = sk.rcvBuf(buf);
  if(cnt <= 0)
    printError('rcv error ' + cnt);
  err = msg.parseBuf(buf, cnt);
  if(err == ptpmgmt.MNG_PARSE_ERROR_MSG)
    throw new Error('error message');
  else if(err != ptpmgmt.MNG_PARSE_ERROR_OK)
    throw new Error('parse error ' + ptpmgmt.Message.err2str_c(err));
  dispacher.callHadler(msg, msg.getTlvId(), msg.getData());
}
function initSocket(cfg)
{
  return sk.setDefSelfAddress() && sk.init() && sk.setPeerAddress(cfg);
}
function body(cfg_file)
{
  let ret = 0;
  try {
    if(!buf.isAlloc())
      throw new Error('buffer allocation failed');
    let cfg = new ptpmgmt.ConfigFile();
    if(!cfg.read_cfg(cfg_file))
      printError('fail reading configuration file');
    if(!initSocket(cfg))
      printError('fail init socket');
    let prms = msg.getParams();
    if(cfg.haveSpp()) {
      if(!msg.useAuth(cfg))
        printError('fail using Authentication');
      prms.sendAuth = true;
    }
    prms.self_id.portNumber = getpid() & 0xffff;
    // Verify we can use implementSpecific_e;
    prms.implementSpecific = ptpmgmt.linuxptp;
    prms.domainNumber = cfg.domainNumber();
    msg.updateParams(prms);
    msg.useConfig(cfg);
    id = ptpmgmt.USER_DESCRIPTION;
    msg.setAction(ptpmgmt.GET, id);
    let seq = nextSequence();
    let err = msg.build(buf, seq);
    if(err != ptpmgmt.MNG_PARSE_ERROR_OK)
      throw new Error('build error ' + ptpmgmt.Message.err2str_c(err));
    if(!sk.sendBuf(buf, msg.getMsgLen()))
      printError('send fail');
    // You can get file descriptor with sk.fileno() and use select;
    if(!sk.poll(500))
      printError('timeout');
    let cnt = sk.rcvBuf(buf);
    if(cnt <= 0)
      printError('rcv error ' + cnt);
    err = msg.parseBuf(buf, cnt);
    if(err == ptpmgmt.MNG_PARSE_ERROR_MSG)
      throw new Error('error message');
    else if(err != ptpmgmt.MNG_PARSE_ERROR_OK)
      throw new Error('parse error ' + ptpmgmt.Message.err2str_c(err));
    dispacher.callHadler(msg);

    // test setting values;
    let clk_dec = new ptpmgmt.CLOCK_DESCRIPTION_t();
    clk_dec.clockType = ptpmgmt.ordinaryClock;
    let physicalAddress = new ptpmgmt.Binary();
    physicalAddress.setBin(0, 0xf1);
    physicalAddress.setBin(1, 0xf2);
    physicalAddress.setBin(2, 0xf3);
    physicalAddress.setBin(3, 0xf4);
    console.log('physicalAddress: ' + physicalAddress.toId());
    console.log('physicalAddress: ' + physicalAddress.toHex());
    clk_dec.physicalAddress.setBin(0, 0xf1);
    clk_dec.physicalAddress.setBin(1, 0xf2);
    clk_dec.physicalAddress.setBin(2, 0xf3);
    clk_dec.physicalAddress.setBin(3, 0xf4);
    console.log('clk.physicalAddress: ' + clk_dec.physicalAddress.toId());
    console.log('clk.physicalAddress: ' + clk_dec.physicalAddress.toHex());
    console.log('manufacturerIdentity: ' +
                ptpmgmt.Binary.bufToId(clk_dec.manufacturerIdentity, 3));
    clk_dec.revisionData.textField = 'This is a test';
    console.log('revisionData: ' + clk_dec.revisionData.textField);

    // test send;
    setPriority1(147);
    setPriority1(153);

    let evnt = new ptpmgmt.SUBSCRIBE_EVENTS_NP_t();
    evnt.setEvent(ptpmgmt.NOTIFY_TIME_SYNC);
    console.log('getEvent(NOTIFY_TIME_SYNC)=' +
         (evnt.getEvent(ptpmgmt.NOTIFY_TIME_SYNC) ? 'have' : 'not'));
    console.log('getEvent(NOTIFY_PORT_STATE)=' +
         (evnt.getEvent(ptpmgmt.NOTIFY_PORT_STATE) ? 'have' : 'not'));

    // test SigEvent that represent std::vector<SLAVE_TX_EVENT_TIMESTAMPS_rec_t>
    // See std_vectors.md for more information
    let evnts = new ptpmgmt.SigEvent();
    let e = new ptpmgmt.SLAVE_TX_EVENT_TIMESTAMPS_rec_t();
    e.sequenceId = 1;
    e.eventEgressTimestamp.fromFloat(4.5);
    evnts.add(e);
    console.log(`Events size ${evnts.size()}, ` +
                `seq[0]=${evnts.get(0).sequenceId}, ` +
                `ts[0]=${evnts.get(0).eventEgressTimestamp.string()}`);
  } catch(err) {
    console.error(err.message);
    ret = -1;
  } finally {
    sk.close();
  }
  return ret;
}

var opt = new ptpmgmt.Options();
if(opt.parse_options(args) === ptpmgmt.Options.OPT_DONE) {
  let cfg_file = opt.val('f');
  if(cfg_file.length === 0)
    cfg_file = DEF_CFG_FILE;
  console.log('Use configuration file ' + cfg_file);
  body(cfg_file);
} else
  console.error('fail parsing command line');
