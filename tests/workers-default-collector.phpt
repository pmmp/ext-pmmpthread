--TEST--
Test pthreads workers default collector
--DESCRIPTION--
This test verifies that the default collector works as expected
--FILE--
<?php
$worker = new \pmmp\thread\Worker();
$worker->start();

$i = 0;
while ($i<10) {
	$worker->stack(new class extends \pmmp\thread\Runnable{
		public function run() : void{}
	});
	$i++;
}

var_dump($i);
while ($worker->collect()){
	usleep(20_000);
}
var_dump($worker->getStacked());
$worker->shutdown();
?>
--EXPECTF--
int(10)
int(0)
