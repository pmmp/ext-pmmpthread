--TEST--
Class constant in INHERIT_NONE thread
--DESCRIPTION--
This test ensures that class constants are available inside in INHERIT_NONE threads without error.
--FILE--
<?php
class testClass extends \pmmp\thread\Thread {

    const TEST_CONSTANT=0x00;

    public function run() : void{
        var_dump('works');
    }
}

$x = new testClass();
$x->start(\pmmp\thread\Thread::INHERIT_NONE);
$x->join();
--EXPECT--
string(5) "works"
