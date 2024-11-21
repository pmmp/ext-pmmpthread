--TEST--
Test include/require functions as expected
--DESCRIPTION--
This test verifies that require_once and include are working as expected
--FILE--
<?php
define("INC", sprintf("%s/includeme.inc", dirname(__FILE__)));

include(INC);
class TestThread extends \pmmp\thread\Thread {
	public function run() : void{
		require_once(INC);
		if (!function_exists("myTestFunc")) {
			printf("FAILED\n");
		} else printf("OK\n");
	}
}
$test = new TestThread();
$test->start(\pmmp\thread\Thread::INHERIT_ALL);
?>
--EXPECT--
OK
