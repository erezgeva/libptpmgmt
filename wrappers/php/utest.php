<?php declare(strict_types=1);
/* SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com>
 *
 * Unit test with php wrapper of libptpmgmt
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 *
 */

use PHPUnit\Framework\TestCase;
# See: https://phpunit.readthedocs.io/
#      man phpunit

class myDisp extends MessageDispatcher {
    public int $priority1;
    public int $func;
    public string $id;
    function __construct() {
        parent::__construct();
        $this->priority1 = 0;
        $this->func = 0;
        $this->id = '';
    }
    function PRIORITY1_h($msg, $tlv, $tlv_id) : void {
        $this->func |= 0x1;
        $this->id = $tlv_id;
        $this->priority1 = $tlv->priority1;
    }
    function noTlv($msg) : void {
        $this->func |= 0x2;
    }
    function noTlvCallBack($msg, $tlv_id) : void {
        $this->func |= 0x4;
        $this->id = $tlv_id;
    }
}
class myBuild extends MessageBuilder {
    public int $run;
    function PRIORITY1_b($msg, $tlv) : bool {
        $this->run = 1;
        $tlv->priority1 = 117;
        return true;
    }
}

final class utest extends TestCase
{
    private Message $msg;
    private myDisp $disp;
    private myBuild $build;

    protected function setUp(): void
    {
        $this->msg = new Message();
        $this->disp = new myDisp();
        $this->build = new myBuild($this->msg);
        $this->build->run = 0;
    }

    # Tests callHadler with empty TLV
    public function testParsedCallHadlerEmptyTlv(): void {
      $this->disp->callHadler($this->msg);
      $this->assertEquals(0x2, $this->disp->func,      'should call noTlv');
      $this->assertEquals('',  $this->disp->id,        'should have empty ID');
      $this->assertEquals(0,   $this->disp->priority1, 'should not have priority1 value');
    }

    # Tests callHadler method with TLV
    public function testParsedCallHadlerTlv(): void {
      $tlv = new PRIORITY1_t();
      $tlv->priority1 = 117;
      $this->disp->callHadler($this->msg, ptpmgmt::PRIORITY1, $tlv);
      $this->assertEquals(0x1,         $this->disp->func,      'should call PRIORITY1_h');
      $this->assertEquals('PRIORITY1', $this->disp->id,        'should have PRIORITY1 ID');
      $this->assertEquals(117,         $this->disp->priority1, 'should have priority1 value');
    }

    # Tests callHadler method with TLV without callback
    public function testParsedCallHadlerTlvWithoutCallback(): void {
      $tlv = new PRIORITY2_t();
      $tlv->priority2 = 117;
      $this->disp->callHadler($this->msg, ptpmgmt::PRIORITY2, $tlv);
      $this->assertEquals(0x4,         $this->disp->func,      'should call noTlvCallBack');
      $this->assertEquals('PRIORITY2', $this->disp->id,        "should have PRIORITY2 ID");
      $this->assertEquals(0,           $this->disp->priority1, "shouldn't have priority1 value");
    }

    # Tests build empty TLV
    public function test_buildEmptyTlv(): void {
      $this->assertTrue($this->build->buildTlv(ptpmgmt::COMMAND, ptpmgmt::ENABLE_PORT), 'should pass');
      $this->assertEquals(0, $this->build->run, "shouldn't call PRIORITY1 callback");
      $this->build->clear();
    }

    # Tests build TLV
    public function test_buildTlv(): void {
      $this->assertTrue($this->build->buildTlv(ptpmgmt::SET, ptpmgmt::PRIORITY1), 'should pass');
      $this->assertEquals(1, $this->build->run, 'should call PRIORITY1 callback');
      $this->build->clear();
    }

    # Tests build TLV that lack callback
    public function test_buildTlvWithoutCallback(): void {
      $this->assertFalse($this->build->buildTlv(ptpmgmt::SET, ptpmgmt::PRIORITY2), 'should no pass');
      $this->assertEquals(0, $this->build->run, "shouldn't call PRIORITY1 callback");
      $this->build->clear();
    }
}
# LD_PRELOAD=../../libptpmgmt.so PHPRC=. phpunit ./utest.php
