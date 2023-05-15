--TEST--
Test stateful fatalities
--DESCRIPTION--
This test verifies that state includes fatalities
--FILE--
<?php
class TestThread extends \pmmp\thread\Thread {
	public function run() : void{
		i_do_not_exist();
	}
}
$test = new TestThread();
$test->start(\pmmp\thread\Thread::INHERIT_ALL);
$test->join();
var_dump($test->isTerminated());
?>
--EXPECTF--
Fatal error: Uncaught Error: Call to undefined function i_do_not_exist() in %s:4
Stack trace:
#0 [internal function]: TestThread->run()
#1 {main}
  thrown in %s on line 4
bool(true)

