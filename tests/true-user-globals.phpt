--TEST--
Basic test of Thread::getSharedGlobals()
--FILE--
<?php

use pmmp\thread\ThreadSafeArray;
use pmmp\thread\Thread;

var_dump(Thread::getSharedGlobals());
Thread::getSharedGlobals()["test"] = new ThreadSafeArray();
var_dump(Thread::getSharedGlobals());

$thread = new class extends Thread{
	public function run() : void{
		var_dump(Thread::getSharedGlobals());
	}
};
$thread->start(Thread::INHERIT_ALL) && $thread->join();
?>
--EXPECT--
object(pmmp\thread\ThreadSafeArray)#1 (0) {
}
object(pmmp\thread\ThreadSafeArray)#1 (1) {
  ["test"]=>
  object(pmmp\thread\ThreadSafeArray)#2 (0) {
  }
}
object(pmmp\thread\ThreadSafeArray)#1 (1) {
  ["test"]=>
  object(pmmp\thread\ThreadSafeArray)#3 (0) {
  }
}
