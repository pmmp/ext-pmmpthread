--TEST--
Test that unbound anonymous classes correctly inherit interfaces from their parent classes
--DESCRIPTION--
In some places it was assumed that the number of interface implemented by a class won't change.
This is not the case when binding anonymous classes, because a previously unknown parent might
cause the anonymous class to have more interfaces after linking.
--FILE--
<?php
$worker = new \pmmp\thread\Worker();

$worker->start(\pmmp\thread\Thread::INHERIT_ALL);

interface Dummy {
}

interface Dummy2 {
}

class Base extends \pmmp\thread\Runnable implements Dummy {
	public function run() : void{}
}
$collectable = new class extends Base implements Dummy2 {
	public function run() : void{
		var_dump($this instanceof Dummy);
		var_dump($this instanceof Dummy2);
	}
};

$collectable->run();
$worker->stack($collectable);
$worker->shutdown();
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
