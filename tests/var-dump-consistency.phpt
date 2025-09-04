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
--EXPECTF--
object(stdClass)#4 (0) {
}
object(pmmp\thread\ThreadSafeArray)#2 (1) {
  ["sock"]=>
  object(Closure)#3 (%d) {%A
  }
}
object(Closure)#3 (%d) {%A
}
object(pmmp\thread\ThreadSafeArray)#2 (1) {
  ["sock"]=>
  object(Closure)#3 (%d) {%A
  }
}
