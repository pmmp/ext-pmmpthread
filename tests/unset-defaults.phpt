--TEST--
Test unset defaults [moot: default values are ignored]
--DESCRIPTION--
This test verifies that unset members do not cause a problem in pthreads objects
--FILE--
<?php
class TestThread extends \pmmp\thread\Thread {
	public $default;
	
	public function run() : void{
		var_dump($this->default); 
	}
}

$thread = new TestThread();
$thread->start();
?>
--EXPECT--
NULL
