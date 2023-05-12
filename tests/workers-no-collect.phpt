--TEST--
Test pthreads workers rules (collect)
--DESCRIPTION--
This test verifies that workers cannot be misused (collect)
--FILE--
<?php
class Test extends \pmmp\thread\Thread {
	public function __construct(\pmmp\thread\Worker $worker) {
		$this->worker = $worker;
	}
	
	public function run() : void{
		$this->worker->collect();
	}
}

$worker = new \pmmp\thread\Worker();
$worker->start();
$test = new Test($worker);
$test->start();
$test->join();
?>
--EXPECTF--
Fatal error: Uncaught RuntimeException: only the creator of this pmmp\thread\Worker may call collect in %s:%d
Stack trace:
#0 %s(%d): pmmp\thread\Worker->collect()
#1 [internal function]: Test->run()
#2 {main}
  thrown in %s on line %d
