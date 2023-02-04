--TEST--
Test that ThreadedArray::merge() doesn't hold onto stale local property caches
--FILE--
<?php

$t1 = new \ThreadedArray();
$t2 = new \ThreadedArray();

$a1 = new \ThreadedArray();
$a2 = new \ThreadedArray();

$a1["f"] = 1;
$a2["f"] = 2;
$t1["a"] = $a1;
$t2["a"] = $a2;

$t1->merge($t2);
var_dump($t1["a"], $t2["a"]); //should be the same object
?>
--EXPECT--
object(ThreadedArray)#5 (1) {
  ["f"]=>
  int(1)
}
object(ThreadedArray)#5 (1) {
  ["f"]=>
  int(1)
}
