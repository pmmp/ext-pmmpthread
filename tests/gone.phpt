--TEST--
Test objects that have gone away
--DESCRIPTION--
This test verifies that objects that have gone away do not cause segfaults
--FILE--
<?php

use pmmp\thread\ThreadSafe;
use pmmp\thread\Thread;

class Child extends ThreadSafe{
	public function __destruct(){
		echo "child destroyed on " . (Thread::getCurrentThread() !== null ? "other" : "main") . " thread\n";
	}
}
class Parent_ extends ThreadSafe{
	public function __construct(public Child $child){}
}
class T extends \pmmp\thread\Thread {
	public bool $dereferenced1 = false;
	public bool $destroyedFromMain = false;

	public function __construct(
		public ?Parent_ $array
	){}

	public function run() : void{
		$array = $this->array;
		$this->array = null; //erase the child thread cache and TS storage
		$this->synchronized(function() : void{
			$this->dereferenced1 = true;
			$this->notify();
		});
		$this->synchronized(function() : void{
			while(!$this->destroyedFromMain){
				$this->wait();
			}
		});
		$array->synchronized(function(){}); //trigger pthreads_store_sync_local_properties()
		var_dump($array->child); //this is now the only remaining reference, and all gateways to "child" have been destroyed because we never dereferenced ours
	}
}

$array = new Parent_(new Child());

$t = new T($array);
$t->start(\pmmp\thread\Thread::INHERIT_ALL);
$t->synchronized(function() use ($t) : void{
	while(!$t->dereferenced1){
		$t->wait();
	}
});
$t->array = null; //destroy the cached ref from our side - now there is no chain of ownership
unset($array); //destroy our ref and all its descendents
$t->synchronized(function() use ($t) : void{
	$t->destroyedFromMain = true;
	$t->notify();
});

$t->join();

?>
--EXPECTF--
child destroyed on main thread

Fatal error: Uncaught %s: pmmpthread detected an attempt to connect to an object which has already been destroyed in %s:%d
Stack trace:
#0 [internal function]: T->run()
#1 {main}
  thrown in %s on line %d

