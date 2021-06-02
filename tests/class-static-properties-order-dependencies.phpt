--TEST--
Testing class statics properties and dependencies (gh bug #666)
--DESCRIPTION--
If a static property holds a reference to a threaded object, the order of class definition is important.
--FILE--
<?php
class FirstClass extends \ThreadedBase
{
    public static $prop = [];
}

class SecondClass extends \ThreadedBase {}

FirstClass::$prop[] = new SecondClass();

$thread = new class extends Thread {
    public function run() {
        var_dump(FirstClass::$prop);
    }
};

$thread->start() && $thread->join();
--EXPECT--
array(1) {
  [0]=>
  object(SecondClass)#1 (0) {
  }
}

