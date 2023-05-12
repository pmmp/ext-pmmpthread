--TEST--
Test pthreads workers rules (stack)
--DESCRIPTION--
This test verifies that workers cannot be misused (stack)
--FILE--
<?php
class Work extends \pmmp\thread\Runnable {
	public function run() : void{}
}

class Test extends \pmmp\thread\Thread {
	public function __construct(\pmmp\thread\Worker $worker) {
		$this->worker = $worker;
	}
	
	public function run() : void{
		$c = new Work();
		$this->worker
			->stack($c);
	}
}

$worker = new \pmmp\thread\Worker();
$worker->start();
$test = new Test($worker);
$test->start();
$test->join();
$worker->shutdown();
?>
--EXPECTF--
Fatal error: Uncaught RuntimeException: only the creator of this pmmp\thread\Worker may call stack in %s:%d
Stack trace:
#0 %s(%d): pmmp\thread\Worker->stack(Object(Work))
#1 [internal function]: Test->run()
#2 {main}
  thrown in %s on line %d

