--TEST--
Test objects that have gone away
--DESCRIPTION--
This test verifies that objects that have gone away do not cause segfaults
--FILE--
<?php

class T extends Thread {
	public bool $dereferenced1 = false;
	public bool $destroyedFromMain = false;

	public function __construct(
		public ?\ThreadedArray $array
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
		var_dump($array["sub"]); //this is now the only remaining reference, and all gateways to "sub" have been destroyed because we never dereferenced ours
	}
}

$array = new \ThreadedArray();
$array["sub"] = new \ThreadedArray();

$t = new T($array);
$t->start();
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
Fatal error: Uncaught %s: pthreads detected an attempt to connect to an object which has already been destroyed in %s:%d
Stack trace:
#0 [internal function]: T->run()
#1 {main}
  thrown in %s on line %d

