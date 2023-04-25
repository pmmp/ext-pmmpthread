--TEST--
Test that thread-safe arrays reject properties correctly
--DESCRIPTION--
We don't permit dynamic properties on thread-safe arrays, but this wasn't previously enforced, leading to some inconsistent behaviour.

This test verifies that the object handlers correctly reject incorrect usages.
--FILE--
<?php

$array = new \ThreadedArray();
$array["thing"] = 2;

var_dump($array->thing); //generates warning

var_dump(isset($array->thing)); //false
var_dump(isset($array->thing->otherThing)); //false

try{
	$array->thing = 1;
	echo "writing property is not supposed to work" . PHP_EOL;
}catch(\Error $e){
	echo $e->getMessage() . PHP_EOL;
}

try{
	$array->thing->otherThing = 1;
	echo "writing indirect property is not supposed to work" . PHP_EOL;
}catch(\Error $e){
	echo $e->getMessage() . PHP_EOL;
}

unset($array->thing); //does nothing
unset($array->thing->otherThing); //does nothing - this doesn't match zend for now, but it's good enough

var_dump($array);

?>
--EXPECT--
Warning: Undefined property: ThreadedArray::$thing in C:\Users\dylan-work\Documents\projects\php-dev\php-src\ext\pthreads\tests\threaded-array-properties.php on line 6
NULL
bool(false)
bool(false)
Cannot create dynamic property ThreadedArray::$thing

Warning: Undefined property: ThreadedArray::$thing in C:\Users\dylan-work\Documents\projects\php-dev\php-src\ext\pthreads\tests\threaded-array-properties.php on line 19
Attempt to assign property "otherThing" on null

Warning: Undefined property: ThreadedArray::$thing in C:\Users\dylan-work\Documents\projects\php-dev\php-src\ext\pthreads\tests\threaded-array-properties.php on line 26
object(ThreadedArray)#1 (1) {
  ["thing"]=>
  int(2)
}
