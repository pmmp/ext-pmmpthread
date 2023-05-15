--TEST--
Test pthreads workers rules (join)
--DESCRIPTION--
This test verifies that workers cannot be misused (join)
--FILE--
<?php
class Test extends \pmmp\thread\Thread {
	public function __construct(\pmmp\thread\Worker $worker) {
		$this->worker = $worker;
	}
	
	public function run() : void{
		$this->worker->shutdown();
	}
}

$worker = new \pmmp\thread\Worker();
$worker->start(\pmmp\thread\Thread::INHERIT_ALL);
$test = new Test($worker);
$test->start(\pmmp\thread\Thread::INHERIT_ALL);
$test->join();
?>
--EXPECTF--
Fatal error: Uncaught RuntimeException: only the creator of this pmmp\thread\Worker may join with it in %s:8
Stack trace:
#0 %s(8): pmmp\thread\Worker->shutdown()
#1 [internal function]: Test->run()
#2 {main}
  thrown in %s on line 8


