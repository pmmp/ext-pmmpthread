--TEST--
Test that property_exists() works as expected with dynamic ThreadSafe properties
--DESCRIPTION--
has_property() handler wasn't correctly implemented on ThreadSafe objects, leading to inconsistent behaviour when assigning dynamic properties.
--FILE--
<?php

function test(object $t) : void{
	$t->prop = null;
	var_dump(property_exists($t, 'prop'));
	var_dump(property_exists($t, 'doesntExist'));
}

test(new \stdClass);
test(new \pmmp\thread\ThreadSafe);
?>
--EXPECT--
bool(true)
bool(false)
bool(true)
bool(false)

