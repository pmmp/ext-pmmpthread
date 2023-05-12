--TEST--
Test that user-defined Thread::join() overrides work correctly
--FILE--
<?php

$c = new class extends \pmmp\thread\Thread{
	private bool $waiting = true;

	public function run() : void{
		$this->synchronized(function() : void{
			while($this->waiting){
				$this->wait();
			}
		});
	}

	public function join() : bool{
		$this->synchronized(function() : void{
			$this->waiting = false;
			$this->notify();
		});
		return false; //not calling the parent will cause the thread to only be joined by the dtor
	}
};
$c->start();
unset($c); //trigger destructor
echo "OK\n"; //this will never be reached if the dtor doesn't do its job
--EXPECT--
OK
