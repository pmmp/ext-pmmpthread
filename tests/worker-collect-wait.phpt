--TEST--
Test that Worker::collect() works in conjunction with wait()
--DESCRIPTION--
Sometimes (especially in tests) it's desirable to wait on a worker for a task to be completed.
This should be achievable by using collect() and wait() inside a synchronized() block.
However, this requires that the internal worker system notify the worker context when a task is made available for collection.
--FILE--
<?php

use pmmp\thread\Thread;
use pmmp\thread\Worker;
use pmmp\thread\Runnable;

$worker = new Worker();
$worker->start(Thread::INHERIT_ALL);

$worker->stack(new class extends Runnable{
	public function run() : void{
		sleep(1);
		echo "hello from task\n";
	}
});

$worker->synchronized(function() use ($worker) : void{
	while($worker->collect() > 0){
		$worker->wait();
	}
});

$worker->shutdown();
echo "OK\n";
?>
--EXPECT--
hello from task
OK
