--TEST--
Test that reading from nonexisting offsets of ThreadSafeArray works correctly (pmmp/ext-pmmpthread#131)
--FILE--
<?php

use pmmp\thread\ThreadSafeArray;

$array = ThreadSafeArray::fromArray(["protocol" => []]);
var_dump($array['protocol2']); //UNKNOWN:0
var_dump($array['protocol2'][0]); //segfault

?>
--EXPECTF--
NULL

Warning: Trying to access array offset on value of type null in %s on line %d
NULL
