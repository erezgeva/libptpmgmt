#!/usr/bin/python3
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# testing Json convertor
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2021 Erez Geva
#
###############################################################################

import os
import sys
from ptpmgmt import *

DEF_CFG_FILE = "/etc/linuxptp/ptp4l.conf"

sk = SockUnix()
assert sk is not None, "Fail socket"
msg = Message()
buf = Buf(1000)
cfg = ConfigFile()
sequence = 0

def printError(msg):
  if Error.isError():
    print(Error.getError())
  else:
    print(msg)
  return -1

def runId(id):
  global sk, msg, buf, sequence
  msg.setAction(GET, id)
  err = msg.build(buf, ++sequence)
  txt = Message.err2str_c(err)
  assert err == MNG_PARSE_ERROR_OK, "build error txt"
  assert sk.send(buf, msg.getMsgLen()), "send"
  # You can get file descriptor with sk.fileno() and use select I/O completion
  if not sk.poll(500):
    return printError("timeout")
  cnt = sk.rcv(buf)
  if cnt <= 0:
    return printError("rcv cnt")
  msg.parse(buf, cnt)
  j = msg2json(msg)
  print("{},".format(j))

def creatJsonTest():
  global sk, msg, buf, cfg
  assert buf.isAlloc(), "buffer allocation failed"
  if len(sys.argv) > 1:
    cfg_file = sys.argv[1]
  else:
    cfg_file = DEF_CFG_FILE
  assert os.path.exists(cfg_file), "Config file uds_address does not exist"
  assert cfg.read_cfg(cfg_file), "ConfigFile"
  assert sk.setDefSelfAddress() and sk.init() and sk.setPeerAddress(cfg), "SockUnix"
  assert msg.useConfig(cfg), "useConfig"
  prms = msg.getParams()
  if cfg.haveSpp():
    assert msg.useAuth(cfg), "fail using Authentication"
    prms.sendAuth = True
  prms.self_id.portNumber = os.getpid() & 0xffff
  msg.updateParams(prms)
  print("[")
  ids = [ANNOUNCE_RECEIPT_TIMEOUT, CLOCK_ACCURACY, CLOCK_DESCRIPTION,
         CURRENT_DATA_SET, DEFAULT_DATA_SET, DELAY_MECHANISM, DOMAIN,
         LOG_ANNOUNCE_INTERVAL, LOG_MIN_PDELAY_REQ_INTERVAL,
         LOG_SYNC_INTERVAL, PARENT_DATA_SET,
         PRIORITY1, PRIORITY2, SLAVE_ONLY, TIMESCALE_PROPERTIES,
         TIME_PROPERTIES_DATA_SET, TRACEABILITY_PROPERTIES,
         USER_DESCRIPTION, VERSION_NUMBER, PORT_DATA_SET,
         TIME_STATUS_NP, GRANDMASTER_SETTINGS_NP, PORT_DATA_SET_NP,
         PORT_PROPERTIES_NP, PORT_STATS_NP, SUBSCRIBE_EVENTS_NP,
         SYNCHRONIZATION_UNCERTAIN_NP,
         PATH_TRACE_LIST] # last one is unsupported by linuxptp
  for n in ids:
    runId(n)
  print("{}]")

def toJsonTest():
  json2msg = Json2msg()
  json = '''\
{
  "sequenceId" : 12,
  "sdoId" : 0,
  "domainNumber" : 0,
  "versionPTP" : 2,
  "minorVersionPTP" : 1,
  "unicastFlag" : false,
  "PTPProfileSpecific" : 0,
  "messageType" : "Management",
  "sourcePortIdentity" :
  {
    "clockIdentity" : "c47d46.fffe.20acae",
    "portNumber" : 0
  },
  "targetPortIdentity" :
  {
    "clockIdentity" : "000000.0000.000000",
    "portNumber" : 20299
  },
  "actionField" : "SET",
  "tlvType" : "MANAGEMENT",
'''
  jsonPr1 = '''\
  "managementId" : "PRIORITY1",
  "dataField" :
  {
    "priority1" : 153
  }
'''
  assert json2msg.fromJson(json + jsonPr1 + "}"), "fail fromJson test 1"
  jsonTbl = '''\
  "managementId" : "UNICAST_MASTER_TABLE",
  "dataField" :
  {
    "logQueryInterval" : 1,
    "actualTableSize" : 3,
    "PortAddress" :
    [
      {
        "networkProtocol" : "IEEE_802_3",
        "addressField" : "c4:7d:46:20:ac:ae"
      },
      {
        "networkProtocol" : "IEEE_802_3",
        "addressField" : "c4:7d:46:20:ac:ae"
      },
      {
        "networkProtocol" : "IEEE_802_3",
        "addressField" : "c4:7d:46:20:ac:ae"
      }
    ]
  }
'''
  assert json2msg.fromJson(json + jsonTbl + "}"), "fail fromJson test 2"

creatJsonTest()
toJsonTest()
sk.close()

# LD_PRELOAD=./.libs/libptpmgmt.so LD_LIBRARY_PATH=.libs PYTHONPATH=wrappers/python:wrappers/python/3 tools/testJson.py | jq
