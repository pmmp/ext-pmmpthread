--TEST--
Test that ThreadedBase objects verify assigned types properly
--DESCRIPTION--
Since ThreadedBase uses custom property read/write handlers, we have to verify types ourselves.
In the past, we simply didn't verify them, but this is problematic particularly when JIT is used,
which makes hard assumptions about property types for optimisations. Therefore, assigning incorrect
types leads to segfaults and other strange behaviour.
This test ensures that property types are enforced correctly on typed properties when assigning to them.
--FILE--
<?php

declare(strict_types=1);

class A extends \ThreadedBase{
	public int $x;

	public \ThreadedBase $threaded;
}

$a = new A();
$a->x = 1;
$a->threaded = new A();

try{
	$a->x = "hello";
}catch(\TypeError $e){
	echo $e->getMessage() . PHP_EOL;
}
try{
	$a->threaded = "test";
}catch(\TypeError $e){
	echo $e->getMessage() . PHP_EOL;
}
?>
--EXPECT--
Cannot assign string to property A::$x of type int
Cannot assign string to property A::$threaded of type ThreadedBase
