--TEST--
Test that using Thread::setAutoloadFile() inside an autoload file doesn't cause a deadlock
--FILE--
<?php

use pmmp\thread\Thread;

Thread::setAutoloadFile(__DIR__ . '/assets/autoload-file-recursion.php');

$thread = new class extends Thread{
	public function run() : void{
		echo "ok\n";
	}
};

$thread->start(Thread::INHERIT_NONE);
$thread->join();
?>
--EXPECT--
ok
