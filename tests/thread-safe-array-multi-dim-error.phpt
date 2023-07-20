--TEST--
Test that writing to nonexisting multidimensional offsets of ThreadSafeArray generates the correct errors (issue #125)
--FILE--
<?php
$threaded = new \pmmp\thread\ThreadSafeArray();

$threaded[0][0] = 4;
?>
--EXPECTF--
Fatal error: Uncaught Error: Indirect modification of non-ThreadSafe members of pmmp\thread\ThreadSafeArray is not supported in %s:%d
Stack trace:
#0 {main}
  thrown in %s on line %d
