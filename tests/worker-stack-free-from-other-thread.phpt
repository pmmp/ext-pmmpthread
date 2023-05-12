--TEST--
Test that worker stacks are freed from the correct thread
--DESCRIPTION--
A mistake was made during the refactor that allowed Threaded internal datastructures to outlive their creators;
Worker stacks were moved into the shared datastructure, causing them to be deallocated by the lst thread referencing them.
However, Worker stacks are not usable by any other thread than the one that created the Worker, and must be freed by the
creator.

This test ensures that no unexpected behaviour occurs when a Worker instance is shared between multiple threads.
--FILE--
<?php

class TestThread extends \pmmp\thread\Thread{
	public $worker = null;
	public $shutdown = false;

	public function run() : void{
		$this->worker = new \pmmp\thread\Worker();
		$this->worker->start();
		$this->synchronized(fn() => $this->notify());
		$this->synchronized(function() : void{
			while(!$this->shutdown){
				$this->wait();
			}
		});
		$this->worker->join();
	}
};

$thread = new TestThread;
$thread->start();
$thread->synchronized(function() use ($thread) : void{
	while($thread->worker === null){
		$thread->wait();
	}
});
$worker = $thread->worker; //dereference, create connection
$thread->shutdown = true;
$thread->synchronized(fn() => $thread->notify());
$thread->join();
unset($thread);
unset($worker); //trigger destructor
echo "OK\n";
?>
--EXPECT--
OK
