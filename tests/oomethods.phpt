--TEST--
Test access to user defined methods in the object context
--DESCRIPTION--
User methods are now imported from your declared class into the thread
--FILE--
<?php
class ThreadTest extends \pmmp\thread\Thread {
	public function objectTest(){
		return $this->value;
	}
	
	public function run() : void{
		$this->value = 1;
	}
}
$thread = new ThreadTest();
if($thread->start(\pmmp\thread\Thread::INHERIT_ALL)) {
	$thread->join();
	var_dump($thread->objectTest());
}
?>
--EXPECT--
int(1)
