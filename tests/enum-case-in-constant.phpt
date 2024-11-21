--TEST--
Test that enum cases work correctly when referenced in class constants
--FILE--
<?php

use pmmp\thread\Thread;

enum TestEnum{
	case ONE;
}

class TestClass extends Thread{

	public const TEST = TestEnum::ONE;

	public function run() : void{
		var_dump(self::TEST);
		var_dump(self::TEST === TestEnum::ONE);
	}
}

$t = new TestClass();
$t->start(Thread::INHERIT_ALL);
$t->join();
?>
--EXPECT--
enum(TestEnum::ONE)
bool(true)
