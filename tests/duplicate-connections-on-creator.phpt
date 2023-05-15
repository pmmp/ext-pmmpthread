--TEST--
Threads shouldn't be auto-joined while connections still exist on the creator thread
--DESCRIPTION--
After refcounting on ThreadSafe internal structures was introduced, the distinction between an original ThreadSafe object and connections was removed.

This was fine until we ran into a problem with threads: they have to be joined by their creators, regardless of how many references to them remain.

Because of the removal of distinction, it became possible to have two distinct ThreadSafe objects referring to the same internal structure on the same thread, leading to implicit destruction of the thread unexpectedly when one of the references was destroyed.

In this test, creating a duplicate connection to the Worker context by dereferencing the connection provided by the worker thread caused the Worker to be incorrectly stopped when the duplicate connection goes out of scope.
--SKIPIF--
<?php if (defined('PHP_WINDOWS_VERSION_MAJOR')) die("skip: this test is too unreliable on Windows right now for reasons unknown"); ?>
--FILE--
<?php

$w = new \pmmp\thread\Worker;
class Dummy extends \pmmp\thread\Runnable{
	/** @var Worker */
	public $worker;

	/** @var bool */
	public $running = false;

	public function run() : void{
		$this->synchronized(function() : void{
			$this->running = true;
			$this->notify();
			echo "hi from worker\n";
		});
	}

	public function waitUntilRunning() : void{
		$this->synchronized(function() : void{
			while(!$this->running){
				if(!$this->wait(60 * 1000000)){
					throw new \RuntimeException("wait timeout");
				}
			}
		});
	}
}
$w->start(\pmmp\thread\Thread::INHERIT_ALL);

echo "first task\n";
$w->stack($t = new Dummy);
$t->waitUntilRunning();
echo "first collect\n";
while($w->collect()){
	usleep(20_000);
}
//this creates a new cached connection to the worker distinct from $w
$t->worker;
unset($t);

echo "second task\n";
$w->stack($t2 = new Dummy);
$t2->waitUntilRunning();
echo "second collect\n";
while($w->collect()){
	usleep(20_000);
}

$w->shutdown();
echo "ok\n";
?>
--EXPECT--
first task
hi from worker
first collect
second task
hi from worker
second collect
ok
