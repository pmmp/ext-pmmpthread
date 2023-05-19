--TEST--
Test pthreads connections
--DESCRIPTION--
This test verifies that variables are bound properly by pthreads
--FILE--
<?php

trait A {
  protected function x() {}
}

class B {
  use A {
    x as public;
  }
}

class My extends \pmmp\thread\Thread{
    public function run() : void{
        for($i=1;$i<2;$i++){
            echo \pmmp\thread\Thread::getCurrentThreadId();
        }
    }
}

$a = new My();
$a->start(\pmmp\thread\Thread::INHERIT_ALL);
$a->join();
--EXPECTF--
%i

