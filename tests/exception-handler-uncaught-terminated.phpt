--TEST--
Test that Runnable::isTerminated() is true when exception handlers are used
--DESCRIPTION--
Since exception handlers don't return the flow to the original code, Runnables should still be considered terminated regardless of whether an exception handler was used.
Due to an incorrect implementation of exception handlers, throwing an uncaught exception wouldn't set the ERROR flag if an exception handler was used.

This test verifies that the bug has been fixed.
--FILE--
<?php

use pmmp\thread\Thread;
use pmmp\thread\Worker;
use pmmp\thread\Runnable;

$t = new class extends Thread{
	public function run() : void{
		set_exception_handler(function(\Throwable $e) : void{
			
		});

		throw new \RuntimeException("bye");
		echo "???\n";
	}
};
$t->start(Thread::INHERIT_ALL) && $t->join();
var_dump($t->isTerminated());

$w = new Worker();
$w->start(Thread::INHERIT_ALL);

$task = new class extends Runnable{
	public function run() : void{
		set_exception_handler(function(\Throwable $e) : void{
			
		});

		throw new \RuntimeException("bye");
		echo "???\n";
	}
};
$w->stack($task);
$w->shutdown();

var_dump($w->isTerminated(), $task->isTerminated());
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
