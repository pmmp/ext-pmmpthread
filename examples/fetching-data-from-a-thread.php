<?php

/*
Threaded objects (which subsequently includes Volatile objects) are tied to the
context in which they are created. They can be used to fetch data from a thread,
but must be created in the outer most thread in which they are used.
*/

// create Threaded object in the main thread
$store = new Threaded();

$thread = new class($store) extends Thread {
	public $store;

	public function __construct(Threaded $store)
	{
		$this->store = $store;
	}

	public function run()
	{
		$this->store[] = [1,2,3];
	}
};

$thread->start() && $thread->join();

print_r($store);
