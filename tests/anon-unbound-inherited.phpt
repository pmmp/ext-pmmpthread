--TEST--
Test anonymous classes (unbound inherited class)
--DESCRIPTION--mae
This test verifies that anonymous ThreadSafe objects work as expected
--FILE--
<?php
$worker = new \pmmp\thread\Worker();

$worker->start(\pmmp\thread\Thread::INHERIT_ALL);

$collectable = new class extends \pmmp\thread\Runnable {
	/** z */
	const Z = 1;
	/** a */
	static $a = 2;
	/** c */
	public $c = false;

	public function run() : void{
		var_dump(
			$this instanceof \pmmp\thread\Runnable,
			self::Z,
			self::$a,
			$this->c
		);
	}
};

$worker->stack($collectable);
$worker->shutdown();
--EXPECT--
bool(true)
int(1)
int(2)
bool(false)
