--TEST--
Test pthreads Worker::unstack
--DESCRIPTION--
This test verifies that unstack functions as intended
--FILE--
<?php
$worker = new \pmmp\thread\Worker();

$worker->stack(new class extends \pmmp\thread\Runnable {
	public function run() : void{
		var_dump($this);
	}
});

var_dump($worker->unstack());
?>
--EXPECTF--
object(%s@anonymous)#%d (%d) {
}
