--TEST--
New class after thread creation
--DESCRIPTION--
Fixes bug #711. This requires a class to be declared after a new thread has been
created, where the new class implements at least one interface
--FILE--
<?php

$worker = new \pmmp\thread\Worker();
$worker->start(\pmmp\thread\Thread::INHERIT_ALL);

interface A {}
class task extends \pmmp\thread\Runnable implements A {
	public function run() : void{}
}

$worker->stack(new task());
$worker->shutdown();
--EXPECT--
