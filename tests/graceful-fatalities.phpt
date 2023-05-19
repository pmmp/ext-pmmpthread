--TEST--
Test graceful fatalities
--DESCRIPTION--
This test verifies that fatalities are graceful with regard to state
--FILE--
<?php
class TestThread extends \pmmp\thread\Thread {
	public function run() : void{
		/* silent fatal error */
		echo @MY::$FATAL;
	}
}
$test = new TestThread();
$test->start(\pmmp\thread\Thread::INHERIT_ALL);
$test->join();
var_dump($test->isTerminated());
?>
--EXPECTF--
Fatal error: Uncaught Error: Class %cMY%c not found in %s:5
Stack trace:
#0 [internal function]: TestThread->run()
#1 {main}
  thrown in %s on line 5
bool(true)
