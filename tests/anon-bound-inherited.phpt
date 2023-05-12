--TEST--
Test anonymous classes (bound inherited class)
--DESCRIPTION--
This test verifies that anonymous ThreadSafe objects work as expected
--FILE--
<?php
$worker = new \pmmp\thread\Worker();

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

$worker->start();
$worker->stack($collectable);
$worker->shutdown();
--EXPECT--
bool(true)
int(1)
int(2)
bool(false)
