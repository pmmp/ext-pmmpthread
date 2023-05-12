--TEST--
var_dump() ThreadSafe object fields should have the same values as when dumping the ThreadSafe itself
--FILE--
<?php

$t = new \pmmp\thread\ThreadSafeArray();

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
object(pmmp\thread\ThreadSafeArray)#1 (1) {
  ["sock"]=>
  object(Closure)#2 (0) {
  }
}
object(Closure)#2 (0) {
}
object(pmmp\thread\ThreadSafeArray)#1 (1) {
  ["sock"]=>
  object(Closure)#2 (0) {
  }
}
