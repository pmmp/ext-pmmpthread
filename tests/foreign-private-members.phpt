--TEST--
Testing foreign private member access, magic methods bug #32
--DESCRIPTION--
Test that the fix for bug #32 is a success
--FILE--
<?php
class MY {
        private $test = "Hello World";

        public function getTest(){
                return $this->test;
        }
}

class TEST extends \pmmp\thread\Thread {
        public function __construct($my) {
                $this->my = serialize($my);
        }

        public function run() : void{
                printf("TEST: %s\n", unserialize($this->my)->getTest());
        }
}

$my = new MY();
$test = new TEST($my);
$test->start();
--EXPECT--
TEST: Hello World


