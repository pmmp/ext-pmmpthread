--TEST--
Tests that errors and uncaught exceptions on threads are correctly output
--DESCRIPTION--
Ensure errors and uncaught exceptions are properly displayed when display_startup_errors is not enabled (gh issue #761)
--INI--
display_startup_errors=0
error_reporting=-1
--FILE--
<?php

function throwException(){
	throw new \Exception("Exception");
}

function undefined(){
	if($a === true);
}

$w = new \pmmp\thread\Worker();
$w->start(\pmmp\thread\Thread::INHERIT_ALL);
$w->stack(new class extends \pmmp\thread\Runnable{
	public function run() : void{
		throwException();
	}
});
$w->shutdown();

$w = new \pmmp\thread\Worker();
$w->start(\pmmp\thread\Thread::INHERIT_ALL);
$w->stack(new class extends \pmmp\thread\Runnable{
	public function run() : void{
		undefined();
	}
});
$w->shutdown();
--EXPECTF--
Fatal error: Uncaught Exception: Exception in %s:%d
Stack trace:
#0 %s(%d): throwException()
#1 [internal function]: %s@anonymous->run()
#2 {main}
  thrown in %s on line %d

%s: Undefined variable%s in %s on line %d
