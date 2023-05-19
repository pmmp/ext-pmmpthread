--TEST--
Test statics (bug 19)
--DESCRIPTION--
This test verifies that static members in declarations made outside of threads are available inside threads without error
--FILE--
<?php
class TestThread extends \pmmp\thread\Thread {
	static $static = "pthreads rocks!";

	public function run() : void{ var_dump(self::$static); }
}

$thread = new TestThread();
$thread->start(\pmmp\thread\Thread::INHERIT_ALL);
?>
--EXPECT--
string(15) "pthreads rocks!"
