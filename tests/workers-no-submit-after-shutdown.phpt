--TEST--
Test that workers don't accept tasks after shutdown
--FILE--
<?php

$w = new \Worker();
$w->start();
$w->join();

try{
	$w->stack(new class extends \ThreadedRunnable{
		public function run() : void{
			echo "hi\n";
		}
	});
}catch(\Exception $e){
	echo $e->getMessage() . PHP_EOL;
}
?>
--EXPECT--
this Worker is no longer running and cannot accept tasks
