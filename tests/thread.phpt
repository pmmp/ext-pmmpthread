--TEST--
Test basic threading
--DESCRIPTION--
This test will create and join a simple thread
--FILE--
<?php
class ThreadTest extends \pmmp\thread\Thread {
	public function run() : void{
		/* nothing to do */
	}
}
$thread = new ThreadTest();
if($thread->start(\pmmp\thread\Thread::INHERIT_ALL))
	var_dump($thread->join());
?>
--EXPECT--
bool(true)
