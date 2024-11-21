--TEST--
Test that using Thread::setAutoloadFile() with invalid paths errors correctly
--DESCRIPTION--
Not sure if we can validate paths at the time of setting them. They might not exist
when set, or might be deleted before we can use them. This means it's ultimately
up to the thread itself to handle errors from wrong include paths correctly.
--FILE--
<?php

use pmmp\thread\Thread;

Thread::setAutoloadFile(__DIR__ . '/assets/i-dont-exist.php');

$thread = new class extends Thread{
	public function run() : void{

	}
};

$thread->start(Thread::INHERIT_NONE);
$thread->join();
?>
--EXPECT--
Warning: Unknown: Failed to open stream: No such file or directory in Unknown on line 0

Fatal error: Unable to open thread autoload file %si-dont-exist.php in Unknown on line 0
