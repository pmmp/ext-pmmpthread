--TEST--
Testing normalizing members
--DESCRIPTION--
This tests that normalizing members works without effort
--FILE--
<?php

$t = new \pmmp\thread\ThreadSafeArray();
$t[] = "one";
$t[] = "two";
$t["three"] = "three";

/* get a normal array */
$normal = (array) $t;
var_dump($normal);
?>
--EXPECT--
array(3) {
  [0]=>
  string(3) "one"
  [1]=>
  string(3) "two"
  ["three"]=>
  string(5) "three"
}
