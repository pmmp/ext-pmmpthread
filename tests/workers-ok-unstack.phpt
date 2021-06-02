--TEST--
Test pthreads Worker::unstack
--DESCRIPTION--
This test verifies that unstack functions as intended
--FILE--
<?php
$worker = new Worker();

$worker->stack(new class extends ThreadedRunnable {
	public function run() {
		var_dump($this);
	}
});

var_dump($worker->unstack());
?>
--EXPECTF--
object(%s@anonymous)#%d (%d) {
}
