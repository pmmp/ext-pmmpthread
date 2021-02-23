--TEST--
Testing member count
--DESCRIPTION--
This test verifies that getting member counts works
--FILE--
<?php
$t = new Threaded();
$t[] = "one";
$t[] = "two";
$t["three"] = "three";
var_dump(count($t));
?>
--EXPECT--
int(3)
