--TEST--
Test that PMMPTHREAD_SHARED_GLOBALS works correctly when accessed on child thread before parent
--DESCRIPTION--
Due to chain of ownership, PMMPTHREAD_SHARED_GLOBALS must be initialized by the main thread.
We cannot JIT it, or a parent thread might try to access it and find out that it doesn't exist.
--FILE--
<?php

use pmmp\thread\Thread;

$t = new class extends Thread{

	public function run() : void{
		//use eval to avoid jitting the auto global on the main thread during compile
		//we want this to be compiled by the child thread
		eval('$PMMPTHREAD_SHARED_GLOBALS["test"] = 1;');
	}
};
$t->start(Thread::INHERIT_ALL) && $t->join();

var_dump($PMMPTHREAD_SHARED_GLOBALS);

?>
--EXPECT--
object(pmmp\thread\ThreadSafeArray)#1 (1) {
  ["test"]=>
  int(1)
}
