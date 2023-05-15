--TEST--
Test inherit none closures
--DESCRIPTION--
This test verifies that closures work when using INHERIT_NONE
--FILE--
<?php
class Test extends \pmmp\thread\Thread {
	public function run() : void{
		$this->synchronized(function(){
			echo "OK\n";
		});
	}
}

$test = new Test();
$test->start(\pmmp\thread\Thread::INHERIT_NONE);
$test->join();
--EXPECT--
OK
