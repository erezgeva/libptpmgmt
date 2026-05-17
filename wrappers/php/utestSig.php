<?php declare(strict_types=1);
/* SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2026 Erez Geva <ErezGeva2@gmail.com>
 *
 * Unit test for signaling with php wrapper of libptpmgmt
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2026 Erez Geva
 *
 */

use PHPUnit\Framework\TestCase;
// See: https://phpunit.readthedocs.io/ or 'man phpunit'

class mySigCb extends MessageSigTlvCallback {
    public int $mask;
    function __construct() {
        parent::__construct();
        $this->mask = 0;
    }
    function callback($msg, $tlvType, $sigTlv) : bool {
        if($tlvType != ptpmgmt::MANAGEMENT) {
          return true; // return true on failure!
        }
        $id = get_MngTlvId($sigTlv);
        $tlv = get_BaseMngTlv($sigTlv);
        // First TLV
        if($id == ptpmgmt::PRIORITY2) {
          $pr2 = conv_PRIORITY2($tlv);
          $this->mask += 1;
          return $pr2->priority2 != 119; // return false on success!
        }
        // Second TLV
        if($id == ptpmgmt::DOMAIN) {
          $domain = conv_DOMAIN($tlv);
          $this->mask += 10;
          return $domain->domainNumber != 7; // return false on success!
        }
        return true; // return true on failure!
    }
}

final class utestSig extends TestCase
{
    // Tests callHadler with empty TLV
    public function testPtpmgmtTraverseSig(): void {
        $msg = new Message();
        $buf = new Buf(100);
        $size = get2MngTlvsSig($buf->get(), $buf->size());
        $this->assertTrue($size > 0, 'get2MngTlvsSig');
        $prms = $msg->getParams();
        $prms->rcvSignaling = true;
        $prms->filterSignaling = false;
        $this->assertTrue($msg->updateParams($prms), 'updateParams');
        $this->assertEquals($msg->parse($buf, $size), ptpmgmt::MNG_PARSE_ERROR_SIG, 'parse');
        $this->assertEquals($msg->getSigTlvsCount(), 2, 'getSigTlvsCount');
        $this->assertEquals($msg->getSigTlvType(0), ptpmgmt::MANAGEMENT, 'getSigTlvType');
        $this->assertEquals($msg->getSigMngTlvType(0), ptpmgmt::PRIORITY2, 'getSigMngTlvType');
        $this->assertEquals($msg->getSigTlvType(1), ptpmgmt::MANAGEMENT, 'getSigTlvType');
        $this->assertEquals($msg->getSigMngTlvType(1), ptpmgmt::DOMAIN, 'getSigMngTlvType');
        $sigTlv = $msg->getSigTlv(0);
        $this->assertEquals(get_MngTlvId($sigTlv), ptpmgmt::PRIORITY2, 'get_MngTlvId');
        $mngTlv1_1 = $msg->getSigMngTlv(0);
        $mngTlv1_2 = get_BaseMngTlv($sigTlv);
        $pr2_1 = conv_PRIORITY2($mngTlv1_1);
        $pr2_2 = conv_PRIORITY2($mngTlv1_2);
        $this->assertEquals($pr2_1->priority2, 119, 'pr2_1.priority2');
        $this->assertEquals($pr2_2->priority2, 119, 'pr2_2.priority2');
        $sigTlv = $msg->getSigTlv(1);
        $this->assertEquals(get_MngTlvId($sigTlv), ptpmgmt::DOMAIN, 'get_MngTlvId');
        $mngTlv1_1 = $msg->getSigMngTlv(1);
        $mngTlv1_2 = get_BaseMngTlv($sigTlv);
        $domain1 = conv_DOMAIN($mngTlv1_1);
        $domain2 = conv_DOMAIN($mngTlv1_2);
        $this->assertEquals($domain1->domainNumber, 7, 'domain1.domainNumber');
        $this->assertEquals($domain2->domainNumber, 7, 'domain2.domainNumber');
        # TODO the callback crash when calling get_MngTlvId() or get_BaseMngTlv()
        #$cb = new MySigCb();
        #$this->assertFalse($msg->traversSigTlvsCl($cb), 'traversSigTlvsCl');
        #$this->assertEquals($cb->mask, 11, 'mySigCb mask');
    }
}
