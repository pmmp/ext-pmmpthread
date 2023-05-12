--TEST--
Test that workers don't accept tasks after shutdown
--FILE--
<?php

$w = new \pmmp\thread\Worker();
$w->start();
$w->join();

try{
	$w->stack(new class extends \pmmp\thread\Runnable{
		public function run() : void{
			echo "hi\n";
		}
	});
}catch(\Exception $e){
	echo $e->getMessage() . PHP_EOL;
}
?>
--EXPECT--
this pmmp\thread\Worker is no longer running and cannot accept tasks
