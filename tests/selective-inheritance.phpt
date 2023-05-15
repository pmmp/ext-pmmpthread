--TEST--
Test selective inheritance
--DESCRIPTION--
This test verifies the functionality of selective inheritance
--FILE--
<?php
function TestFunction(){
	return __FUNCTION__;
}

define ("TEST_CONSTANT", true);

class TestClass {}

class TestThread extends \pmmp\thread\Thread {
	public function run() : void{ 
		var_dump(function_exists("TestFunction"));
		var_dump(defined("TEST_CONSTANT"));
		var_dump(class_exists("TestClass")); 
	}
}

$thread = new TestThread();
$thread->start(\pmmp\thread\Thread::INHERIT_ALL);
$thread->join();
unset($thread);

$thread = new TestThread();
$thread->start(\pmmp\thread\Thread::INHERIT_NONE);
$thread->join();
unset($thread);

$thread = new TestThread();
$thread->start(\pmmp\thread\Thread::INHERIT_FUNCTIONS | \pmmp\thread\Thread::INHERIT_CLASSES);
$thread->join();
unset($thread);
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(false)
bool(false)
bool(false)
bool(true)
bool(false)
bool(true)


