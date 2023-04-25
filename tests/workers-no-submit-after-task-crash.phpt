--TEST--
Test that Workers don't accept tasks after a task crashed
--FILE--
<?php

$w = new \Worker();
$w->stack(new class extends \ThreadedRunnable{
	public function run() : void{
		throw new \Exception();
	}
});
$w->start();
while($w->collect() > 0){
	usleep(1);
}

try{
	$w->stack(new class extends \ThreadedRunnable{
		public function run() : void{
			echo "hi" . PHP_EOL;
		}
	});
}catch(\Exception $e){
	echo $e->getMessage();
}
?>
--EXPECTF--
Fatal error: Uncaught Exception in %s:%d
Stack trace:
#0 [internal function]: ThreadedRunnable@anonymous->run()
#1 {main}
  thrown in %s on line %d
this Worker is no longer running and cannot accept tasks
