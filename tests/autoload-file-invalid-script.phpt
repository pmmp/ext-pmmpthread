--TEST--
Test that using Thread::setAutoloadFile() with a broken PHP file errors properly
--DESCRIPTION--
Not sure if we can validate paths at the time of setting them. They might not exist
when set, or might be deleted before we can use them. This means it's ultimately
up to the thread itself to handle errors from wrong include paths correctly.
--FILE--
<?php

use pmmp\thread\Thread;

Thread::setAutoloadFile(__DIR__ . '/assets/autoload-file-syntax-error.php');

$thread = new class extends Thread{
	public function run() : void{
		echo "unreachable\n";
	}
};

$thread->start(Thread::INHERIT_NONE);
$thread->join();
?>
--EXPECTF--
Parse error: Unclosed '(' on line 3 in %s on line %d

Fatal error: Error compiling thread autoload file %sautoload-file-syntax-error.php in Unknown on line 0
