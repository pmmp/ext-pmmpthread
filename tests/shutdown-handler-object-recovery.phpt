--TEST--
Test that shutdown handlers are called before thread enters AWAIT_JOIN state
--DESCRIPTION--
If a thread creates ThreadSafe objects during the shutdown handler to be retrieved by the main thread,
the store sync must take place after the shutdown handler has run, but before the thread's object destructors are called.
Previously, shutdown handlers were called directly by php_request_shutdown(), giving no opportunity to do this.
--FILE--
<?php

use pmmp\thread\Thread;
use pmmp\thread\ThreadSafeArray;

$c = new class extends Thread{

	public ThreadSafeArray $array;

	public function run() : void{
		register_shutdown_function(function() : void{
			$this->array = new ThreadSafeArray();
		});
	}
};
$c->start(Thread::INHERIT_NONE);
$c->join();
var_dump($c->array);
?>
--EXPECT--
object(pmmp\thread\ThreadSafeArray)#3 (0) {
}

