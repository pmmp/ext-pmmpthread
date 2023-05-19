--TEST--
Test that thread-safe objects in Thread::getSharedGlobals() placed by child threads are synced by parent threads during join
--DESCRIPTION--
Thread shared globals aren't owned by any thread, so there is no chain of ownership for object rescue.
We need to ensure that join() rescues objects in the shared globals which were put there by the child thread.
--FILE--
<?php

use pmmp\thread\Thread;
use pmmp\thread\ThreadSafeArray;

$t = new class extends Thread{
	public function run() : void{
		Thread::getSharedGlobals()["test"] = new ThreadSafeArray();
	}
};
$t->start(Thread::INHERIT_ALL) && $t->join();

var_dump(Thread::getSharedGlobals()["test"]);
?>
--EXPECT--
object(pmmp\thread\ThreadSafeArray)#3 (0) {
}
