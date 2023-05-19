--TEST--
Basic test of PMMPTHREAD_SHARED_GLOBALS
--FILE--
<?php

use pmmp\thread\ThreadSafeArray;
use pmmp\thread\Thread;

var_dump($PMMPTHREAD_SHARED_GLOBALS);
$PMMPTHREAD_SHARED_GLOBALS["test"] = new ThreadSafeArray();
var_dump($PMMPTHREAD_SHARED_GLOBALS);

$thread = new class extends Thread{
	public function run() : void{
		var_dump($PMMPTHREAD_SHARED_GLOBALS);
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
