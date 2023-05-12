--TEST--
Test static properties of classes
--DESCRIPTION--
This is yet another variation of static properties that has highlighted regressions
--FILE--
<?php

$worker = new \pmmp\thread\Worker();
$worker->start();

class TestAsyncTask extends ManInTheMiddle {
	public static $destroyed = false;

	public function run() : void{
		try {
			var_dump(self::$destroyed);
		} catch(\Error $e) {
			var_dump($e->getMessage());
		}
	}
}

abstract class ManInTheMiddle extends \pmmp\thread\Runnable{}

error_reporting(-1);

$worker->stack(new TestAsyncTask());
$worker->shutdown();
--EXPECT--
bool(false)
