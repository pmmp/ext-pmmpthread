--TEST--
Test pthreads connections
--DESCRIPTION--
This test verifies that variables are bound properly by pthreads
--FILE--
<?php
class ThreadTesting extends \pmmp\thread\Thread {
	public $other;
	public $done;
	
	public function setOther($other){
		$this->other = $other;
	}
	public function run() : void{
		$this->synchronized(function($that) {
			$that->done = true;
			$that->notify();
		}, $this);
	}
}

class ThreadTest extends \pmmp\thread\Thread {
	public $other;
	public $done;
	
	public function setOther($other){
		$this->other = $other;
	}
	public function run() : void{
		$this->synchronized(function($that) {
			$that->done = true;
			$that->notify();
		}, $this);
	}
}

$threads[0]=new ThreadTesting();
$threads[1]=new ThreadTest();
$threads[0]->setOther($threads[1]);
$threads[1]->setOther($threads[0]);
foreach($threads as $thread)
	$thread->start(\pmmp\thread\Thread::INHERIT_ALL);
foreach($threads as $thread) {
	$thread->synchronized(function() use($thread){
		if (!$thread->done)
			var_dump($thread->wait());
		else var_dump($thread->done);
	});
}
foreach ($threads as $thread)
	$thread->join();
?>
--EXPECT--
bool(true)
bool(true)
