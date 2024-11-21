--TEST--
Test that using Thread::setAutoloadFile() behaves properly when a file throws errors
--FILE--
<?php

use pmmp\thread\Thread;

Thread::setAutoloadFile(__DIR__ . '/assets/autoload-file-uncaught-exception.php');

$thread = new class extends Thread{
	public function run() : void{

	}
};

$thread->start(Thread::INHERIT_NONE);
$thread->join();
?>
--EXPECTF--
Fatal error: Uncaught Exception: cya in %s:%d
Stack trace:
#0 {main}
  thrown in %s on line %d

Fatal error: Uncaught exception thrown from thread autoload file %sautoload-file-uncaught-exception.php in Unknown on line 0
