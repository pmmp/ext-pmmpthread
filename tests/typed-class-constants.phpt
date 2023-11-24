--TEST--
Test that class constant types are copied correctly
--SKIPIF--
<?php if(PHP_VERSION_ID < 80300) die("skip this test is for 8.3+ only"); ?>
--FILE--
<?php

use pmmp\thread\Thread;

class Test{
	public const int TEST = 1;
}

$thread = new class extends Thread{
	public function run() : void{
		$reflect = new \ReflectionClass(Test::class);
		$const = $reflect->getReflectionConstant("TEST");
		var_dump($const->getType()->getName());
		var_dump($const->getType()->allowsNull());
	}
};
$thread->start(Thread::INHERIT_ALL) && $thread->join();
?>
--EXPECT--
string(3) "int"
bool(false)
