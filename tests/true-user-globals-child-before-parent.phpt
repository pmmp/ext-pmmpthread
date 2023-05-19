--TEST--
Test that Thread::getSharedGlobals() works correctly when accessed on child thread before parent
--DESCRIPTION--
Due to chain of ownership, shared globals must be initialized by the main thread.
We cannot JIT it, or a parent thread might try to access it and find out that it doesn't exist.
--FILE--
<?php

use pmmp\thread\Thread;

$t = new class extends Thread{

	public function run() : void{
		Thread::getSharedGlobals()["test"] = 1;
	}
};
$t->start(Thread::INHERIT_ALL) && $t->join();

var_dump(Thread::getSharedGlobals());

?>
--EXPECT--
object(pmmp\thread\ThreadSafeArray)#1 (1) {
  ["test"]=>
  int(1)
}
