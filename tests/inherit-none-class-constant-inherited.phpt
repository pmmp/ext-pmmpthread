--TEST--
Inherited class constant in INHERIT_NONE thread
--DESCRIPTION--
This test ensures that inherited class constants are available inside in INHERIT_NONE threads without error.
--FILE--
<?php
class baseTest extends \pmmp\thread\Thread {

    const TEST_CONSTANT = 0x00;

    public function test() {
        $this->synchronized(function () {
            var_dump("Constant: " . self::TEST_CONSTANT);
        });
    }

    public function run() : void{}
}

class testClass extends baseTest {

    public function run() : void{
        $this->test();
    }
}

$x = new testClass();
$x->start(\pmmp\thread\Thread::INHERIT_NONE);
$x->join();
--EXPECT--
string(11) "Constant: 0"
