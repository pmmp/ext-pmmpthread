--TEST--
Test the copying of child to parent thread
--DESCRIPTION--
When copying a class from a child thread to a parent thread, there are problems
with interned strings (since they were not being copied).
--FILE--
<?php

class Foo extends Thread
{
	public $running = true;
	private $shared;

	public function run() : void{
		require __DIR__ . '/child-to-parent-class-copying-helper.php';

		$this->shared['baseClass'] = new ExternalBaseClass();
		$this->synchronized(function() : void{
			$this->notify();
		});

		$this->synchronized(function() : void{
			while($this->running){
				$this->wait();
			}
		});
	}
}

$foo = new Foo();
$foo->shared = new ThreadedArray();
$foo->start();

$foo->synchronized(function() use ($foo) : void{
	while(!isset($foo->shared['baseClass'])){
		$foo->wait();
	}
});

$baseClass = $foo->shared['baseClass']; // copy zend_class_entry
$foo->running = false;
$foo->synchronized(function() use ($foo) : void{
	$foo->notify();
});

$foo->join();
echo "OK\n";
--EXPECT--
OK
