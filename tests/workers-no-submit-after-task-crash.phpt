--TEST--
Test that Workers don't accept tasks after a task crashed
--FILE--
<?php

$w = new \pmmp\thread\Worker();
$w->stack(new class extends \pmmp\thread\Runnable{
	public function run() : void{
		throw new \Exception();
	}
});
$w->start(\pmmp\thread\Thread::INHERIT_ALL);
while($w->collect() > 0){
	usleep(1);
}

try{
	$w->stack(new class extends \pmmp\thread\Runnable{
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
#0 [internal function]: pmmp\thread\Runnable@anonymous->run()
#1 {main}
  thrown in %s on line %d
this pmmp\thread\Worker is no longer running and cannot accept tasks
