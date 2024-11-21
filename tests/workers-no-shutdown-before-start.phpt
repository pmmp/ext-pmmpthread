--TEST--
Test pthreads Worker::shutdown
--DESCRIPTION--
This test verifies shutdown of a Worker (or Thread) not yet started doesn't fault
--FILE--
<?php
$worker = new \pmmp\thread\Worker();

$worker->stack(new class extends \pmmp\thread\Runnable {
	public function run() : void{
		var_dump($this);
	}
});

$worker->shutdown();
?>
--EXPECTF--
Fatal error: Uncaught RuntimeException: pmmp\thread\Worker has not been started in %s:10
Stack trace:
#0 %s(10): pmmp\thread\Worker->shutdown()
#1 {main}
  thrown in %s on line 10
