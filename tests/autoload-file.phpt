--TEST--
Tests basic functionality of Thread::setAutoloadFile()
--FILE--
<?php

use pmmp\thread\Thread;

Thread::setAutoloadFile(__DIR__ . '/assets/TestAutoloadFile.php');

$t = new class extends Thread{
	public function run() : void{
		(new TestAutoloadClass())->hi();
	}
};
$t->start(Thread::INHERIT_NONE);
$t->join();
?>
--EXPECT--
string(20) "everything is great!"
