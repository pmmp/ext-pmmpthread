--TEST--
Test pthreads workers rules (start)
--DESCRIPTION--
This test verifies that workers cannot be misused (start)
--FILE--
<?php
class Test extends \pmmp\thread\Thread {
	public function __construct(\pmmp\thread\Worker $worker) {
		$this->worker = $worker;
	}
	
	public function run() : void{
		$this->worker->start(\pmmp\thread\Thread::INHERIT_ALL);
	}
}

$worker = new \pmmp\thread\Worker();
$test = new Test($worker);
$test->start(\pmmp\thread\Thread::INHERIT_ALL);
$test->join();
?>
--EXPECTF--
Fatal error: Uncaught RuntimeException: only the creator of this pmmp\thread\Worker may start it in %s:8
Stack trace:
#0 %s(8): pmmp\thread\Thread->start(%d)
#1 [internal function]: Test->run()
#2 {main}
  thrown in %s on line 8


