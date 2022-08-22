--TEST--
Test that ThreadedBase coerces values to the proper type when assigned to typed properties in weak type mode
--FILE--
<?php

class A extends \ThreadedBase{
	public int $x;
}

$a = new A();
$a->x = "1";
var_dump($a->x);
?>
--EXPECT--
int(1)
