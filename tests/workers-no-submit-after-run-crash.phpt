--TEST--
Test that Workers don't accept tasks after their run() crashed
--DESCRIPTION--
If Worker::run() crashes, it may leave critical state uninitialized, so we can't allow tasks to be submitted in this case.
--FILE--
<?php

$w = new class extends \Worker{
	public function run() : void{
		throw new \Exception();
	}
};
$w->start();
$w->synchronized(function() use ($w) : void{
	while(!$w->isTerminated()){
		$w->wait();
	}
});

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
#0 [internal function]: Worker@anonymous->run()
#1 {main}
  thrown in %s on line %d
this Worker@anonymous is no longer running and cannot accept tasks
