--TEST--
Test that an object can be sent from T1 -> T2, unref'd by T1, and then sent back to T1 without errors
--DESCRIPTION--
Due to some old code intended to avoid duplicated connections, we would try to connect directly to the original
object from the thread that created the object, rather than the thread which gave us the object. This is problematic,
since the creator thread might no longer reference the object, although it could still (evidently) exist on other threads.

The following test case verifies that an object can still be connected to once its creator doesn't reference it anymore.
--FILE--
<?php

declare(strict_types=1);

use pmmp\thread\ThreadSafeArray;

$inChannel = new ThreadSafeArray();
$outChannel = new ThreadSafeArray();

$thread = new class($inChannel, $outChannel) extends \pmmp\thread\Thread{
	public bool $mainThreadReady = false;

	public function __construct(
		private ThreadSafeArray $inChannel,
		private ThreadSafeArray $outChannel
	){}

	public function run() : void{
		$object = $this->synchronized(function() : object{
			while($this->inChannel->count() === 0){
				$this->wait();
			}
			$object = $this->inChannel->shift();
			$this->notify();
			return $object;
		});
		$this->synchronized(function() : void{
			while(!$this->mainThreadReady){
				$this->wait();
			}
		});
		$this->synchronized(function() use($object) : void{
			$this->outChannel[] = $object;
			$this->notify();
		});
	}
};

$thread->start(\pmmp\thread\Thread::INHERIT_ALL);

var_dump("sending our object");
$thread->synchronized(function() use($inChannel, $thread) : void{
	$inChannel[] = new ThreadSafeArray();
	$thread->notify();
});
var_dump("waiting for thread to take object");
$thread->synchronized(function() use($inChannel, $thread) : void{
	while($inChannel->count() > 0){
		$thread->wait();
	}
});
var_dump("destroying cached ref to our object");
$inChannel["abc"] = new ThreadSafeArray(); //synchronize refs

var_dump("notifying thread to return our object");
$thread->synchronized(function() use($thread) : void{
	$thread->mainThreadReady = true;
	$thread->notify();
});
var_dump("waiting for our object to return");
$thread->synchronized(function() use($outChannel, $thread) : void{
	while($outChannel->count() === 0){
		$thread->wait();
	}
});
//our object is now returned to us
var_dump("recovering our object");
var_dump($outChannel->shift());
?>
--EXPECT--
string(18) "sending our object"
string(33) "waiting for thread to take object"
string(35) "destroying cached ref to our object"
string(37) "notifying thread to return our object"
string(32) "waiting for our object to return"
string(21) "recovering our object"
object(pmmp\thread\ThreadSafeArray)#6 (0) {
}
