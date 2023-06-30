--TEST--
Test ThreadSafeArray issue #125
--DESCRIPTION--
Testing issue #125
--FILE--
<?php
$threaded = new \pmmp\thread\ThreadSafeArray();

$threaded[0][0] = 4;
var_dump($threaded[0][0]);
?>
--EXPECTF--
int(4)
