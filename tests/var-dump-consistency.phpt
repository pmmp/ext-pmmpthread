--TEST--
var_dump() Threaded object fields should have the same values as when dumping the Threaded itself
--FILE--
<?php

$t = new \ThreadedArray();

$t["sock"] = function(){};
$t2 = new \stdClass();
var_dump($t2);
var_dump($t);
var_dump($t["sock"]);
var_dump($t);

?>
--EXPECT--
object(stdClass)#3 (0) {
}
object(ThreadedArray)#1 (1) {
  ["sock"]=>
  object(Closure)#2 (0) {
  }
}
object(Closure)#2 (0) {
}
object(ThreadedArray)#1 (1) {
  ["sock"]=>
  object(Closure)#2 (0) {
  }
}
