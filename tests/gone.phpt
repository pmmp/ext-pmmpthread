--TEST--
Test objects that have gone away
--DESCRIPTION--
This test verifies that objects that have gone away do not cause segfaults
--FILE--
<?php

class T extends \pmmp\thread\Thread {
	public bool $dereferenced1 = false;
	public bool $destroyedFromMain = false;

	public function __construct(
		public ?\pmmp\thread\ThreadSafeArray $array
	){}

	public function run() : void{
		$array = $this->array;
		$this->array = null; //erase the child thread cache and TS storage
		$array["otherThing"] = new \pmmp\thread\ThreadSafeArray();
		$this->synchronized(function() : void{
			$this->dereferenced1 = true;
			$this->notify();
		});
		$this->synchronized(function() : void{
			while(!$this->destroyedFromMain){
				$this->wait();
			}
		});
		$array["abc"] = new \pmmp\thread\ThreadSafeArray(); //trigger pthreads_store_sync_local_properties()
		var_dump($array["sub"]); //this is now the only remaining reference, and all gateways to "sub" have been destroyed because we never dereferenced ours
	}
}

$array = new \pmmp\thread\ThreadSafeArray();
$array["sub"] = new \pmmp\thread\ThreadSafeArray();

$t = new T($array);
$t->start(\pmmp\thread\Thread::INHERIT_ALL);
$t->synchronized(function() use ($t) : void{
	while(!$t->dereferenced1){
		$t->wait();
	}
});
$t->array = null; //destroy the cached ref from our side - now there is no chain of ownership
$array["otherThing"] = new class extends \pmmp\thread\ThreadSafe{}; //overwrite their object with one that will soon no longer exist
unset($array); //destroy our ref and all its descendents
$t->synchronized(function() use ($t) : void{
	$t->destroyedFromMain = true;
	$t->notify();
});

$t->join();

?>
--EXPECTF--
Fatal error: Uncaught %s: pmmpthread detected an attempt to connect to an object which has already been destroyed in %s:%d
Stack trace:
#0 [internal function]: T->run()
#1 {main}
  thrown in %s on line %d

