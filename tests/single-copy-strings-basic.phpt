--TEST--
Test that string copying works correctly from live and dead threads
--DESCRIPTION--
We implement some optimisations to allow strings to be copied only 1 time, as long as they live on the child thread for long enough for the parent thread to dereference them.
This test verifies the basic functionality, with a string that will be single-copied (the "a" string) and another which will be copied the old way with just-in-time rescue when the object is destroyed.
--FILE--
<?php

$thread = new class extends \Thread{

	public \ThreadedArray $buffer;

	public function __construct(){
		$this->buffer = new \ThreadedArray();
	}

	public ?string $str = null;
	public bool $shutdown = false;


	public function run() : void{
		$this->synchronized(function() : void{
			$this->buffer[] = str_repeat("a", 20);
			$this->buffer[] = str_repeat("b", 20);
			$this->notify();
		});
		$this->synchronized(function() : void{
			while(!$this->shutdown){
				$this->wait();
			}
		});
	}
};
$thread->start();

$thread->synchronized(function() use ($thread) : void{
	while($thread->buffer->count() === 0){
		$thread->wait();
	}
});
var_dump($thread->buffer->shift());
$thread->synchronized(function() use ($thread) : void{
	$thread->shutdown = true;
	$thread->notify();
});

$thread->join();
var_dump($thread->buffer->shift());

echo "OK\n";
?>
--EXPECT--
string(20) "aaaaaaaaaaaaaaaaaaaa"
string(20) "bbbbbbbbbbbbbbbbbbbb"
OK

