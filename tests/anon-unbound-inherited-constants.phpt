--TEST--
Test that unbound anonymous classes correctly inherit constants from parent classes and interfaces
--DESCRIPTION--
Linking an anonymous class can cause new constants to become available. These should be accessible
from copied versions of the anonymous class.
--FILE--
<?php
$worker = new Worker();

$worker->start();

interface Dummy {
	public const A_CONSTANT = 1;
}

interface Dummy2 {
	public const A_CONSTANT_2 = 2;
}

class Base extends Threaded implements Dummy {
	public const BASE_CONSTANT = 3;
}

$collectable = new class extends Base implements Dummy2 {
	public function run() {
		var_dump(self::A_CONSTANT);
		var_dump(static::A_CONSTANT);
		var_dump(self::A_CONSTANT_2);
		var_dump(static::A_CONSTANT_2);
		var_dump(self::BASE_CONSTANT);
		var_dump(static::BASE_CONSTANT);
	}
};

$collectable->run();
$worker->stack($collectable);
$worker->shutdown();
--EXPECT--
int(1)
int(1)
int(2)
int(2)
int(3)
int(3)
int(1)
int(1)
int(2)
int(2)
int(3)
int(3)
