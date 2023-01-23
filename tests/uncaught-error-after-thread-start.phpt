--TEST--
Test that zend_error_cb() isn't getting wiped out by newly started threads
--DESCRIPTION--
Due to some change in PHP 8.2, zend_error_cb becomes NULL when a thread starts.
This test verifies that this bug has been fixed.
--FILE--
<?php

declare(strict_types=1);

function a(int $a) : void{

}

$t = new class extends \Thread{
	public bool $ready = false;

	public function run() : void{
		$this->synchronized(function() : void{
			$this->ready = true;
			$this->notify();
		});
	}
};
$t->start();
$t->synchronized(function() use ($t) : void{
	while(!$t->ready){
		$t->wait();
	}
});
a("hello");
?>
--EXPECTF--
Fatal error: Uncaught TypeError: a(): Argument #1 ($a) must be of type int, string given, called in %s:%d
Stack trace:
#0 %s(%d): a('hello')
#1 {main}
  thrown in %s on line %d
