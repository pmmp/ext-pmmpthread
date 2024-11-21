--TEST--
Test pthreads constant array copy
--FILE--
<?php
const CONSTANT_ARRAY = ['ok'];

class Test extends \pmmp\thread\Thread {
        public function run() : void{
                echo CONSTANT_ARRAY[0];
        }
}

$test = new Test();
$test->start(\pmmp\thread\Thread::INHERIT_ALL);
$test->join();
--EXPECT--
ok
