--TEST--
Test pthreads workers default collector
--DESCRIPTION--
This test verifies that the default collector works as expected
--FILE--
<?php
$worker = new \pmmp\thread\Worker();
$worker->start(\pmmp\thread\Thread::INHERIT_ALL);

$i = 0;
while ($i<10) {
	$worker->stack(new class extends \pmmp\thread\Runnable{
		public function run() : void{}
	});
	$i++;
}

var_dump($i);
$worker->synchronized(function() use ($worker) : void{
	while($worker->collect() > 0){
		$worker->wait();
	}
});
var_dump($worker->getStacked());
$worker->shutdown();
?>
--EXPECTF--
int(10)
int(0)
