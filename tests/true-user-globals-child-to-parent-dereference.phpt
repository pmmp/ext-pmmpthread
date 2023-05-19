--TEST--
Test that thread-safe objects in PMMPTHREAD_SHARED_GLOBALS placed by child threads are synced by parent threads during join
--DESCRIPTION--
PMMPTHREAD_SHARED_GLOBALS isn't owned by any thread, so chain of ownership of objects is interrupted.
We need to ensure that join() rescues objects in the shared globals which were put there by the child thread.
--FILE--
<?php

use pmmp\thread\Thread;
use pmmp\thread\ThreadSafeArray;

$t = new class extends Thread{
	public function run() : void{
		$PMMPTHREAD_SHARED_GLOBALS["test"] = new ThreadSafeArray();
	}
};
$t->start(Thread::INHERIT_ALL) && $t->join();

var_dump($PMMPTHREAD_SHARED_GLOBALS["test"]);
?>
--EXPECT--
object(pmmp\thread\ThreadSafeArray)#3 (0) {
}
