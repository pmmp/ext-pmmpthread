--TEST--
Test assigning Closure to ThreadSafe doesn't break use()d variables
--FILE--
<?php

$t = new class extends \pmmp\thread\ThreadSafe{};
$name = "eren5960";
$type = "normal";
$t->func = function() use($name, $type) : void{
	var_dump($name, $type);
};
($t->func)();

?>
--EXPECT--
string(8) "eren5960"
string(6) "normal"
