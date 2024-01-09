--TEST--
Test that Thread::getRunningCount() works as expected
--FILE--
<?php

use pmmp\thread\Thread;
use pmmp\thread\Worker;

var_dump(Thread::getRunningCount()); //0, main thread is not managed by pmmpthread

$thread = new class extends Thread{

	public bool $ready = false;
	public bool $wait = true;

	public function run() : void{
		$this->synchronized(function() : void{
			$this->ready = true;
			$this->notify();
		});
		$this->synchronized(function() : void{
			while($this->wait){
				$this->wait();
			}
		});
	}
};

var_dump(Thread::getRunningCount()); //still 0, we haven't started the thread yet

$thread->start(Thread::INHERIT_ALL);
$thread->synchronized(function() use ($thread) : void{
	while(!$thread->ready){
		$thread->wait();
	}
});

var_dump(Thread::getRunningCount()); //1, the thread is now running

$thread->synchronized(function() use ($thread) : void{
	$thread->wait = false;
	$thread->notify();
});

//this is a bit flaky and might break depending on internals changes
$thread->synchronized(function() use ($thread) : void{
	while($thread->isRunning()){
		$thread->wait();
	}
});
var_dump(Thread::getRunningCount()); //still 1, thread has not yet been joined

$thread->join();

var_dump(Thread::getRunningCount()); //0, thread has been joined

//ensure it works correctly in crash conditions also
$thread = new class extends Thread{
	public function run() : void{
		throw new \Exception("test");
	}
};
$thread->start(Thread::INHERIT_ALL);
$thread->synchronized(function() use ($thread) : void{
	while(!$thread->isTerminated()){
		$thread->wait();
	}
});
var_dump(Thread::getRunningCount()); //1, thread has not yet been joined

$thread->join();
var_dump(Thread::getRunningCount()); //0, thread has been joined

?>
--EXPECTF--
int(0)
int(0)
int(1)
int(1)
int(0)

Fatal error: Uncaught Exception: test in %s:%d
Stack trace:
#0 [internal function]: pmmp\thread\Thread@anonymous->run()
#1 {main}
  thrown in %s on line %d
int(1)
int(0)
