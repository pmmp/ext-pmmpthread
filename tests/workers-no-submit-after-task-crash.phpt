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
$w->synchronized(function() use ($w) : void{
	while($w->collect() > 0){
		$w->wait();
	}
});
echo "stacking next task" . PHP_EOL;

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
stacking next task
this pmmp\thread\Worker is no longer running and cannot accept tasks
