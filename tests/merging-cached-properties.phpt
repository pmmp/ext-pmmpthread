--TEST--
Test that ThreadSafeArray::merge() doesn't hold onto stale local property caches
--FILE--
<?php

$t1 = new \pmmp\thread\ThreadSafeArray();
$t2 = new \pmmp\thread\ThreadSafeArray();

$a1 = new \pmmp\thread\ThreadSafeArray();
$a2 = new \pmmp\thread\ThreadSafeArray();

$a1["f"] = 1;
$a2["f"] = 2;
$t1["a"] = $a1;
$t2["a"] = $a2;

$t1->merge($t2);
var_dump($t1["a"], $t2["a"]); //should be the same object
?>
--EXPECT--
object(pmmp\thread\ThreadSafeArray)#4 (1) {
  ["f"]=>
  int(2)
}
object(pmmp\thread\ThreadSafeArray)#4 (1) {
  ["f"]=>
  int(2)
}
