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
		$this->worker->start();
	}
}

$worker = new \pmmp\thread\Worker();
$test = new Test($worker);
$test->start();
$test->join();
?>
--EXPECTF--
Fatal error: Uncaught RuntimeException: only the creator of this pmmp\thread\Worker may start it in %s:8
Stack trace:
#0 %s(8): pmmp\thread\Thread->start()
#1 [internal function]: Test->run()
#2 {main}
  thrown in %s on line 8


