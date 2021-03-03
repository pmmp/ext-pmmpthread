--TEST--
Test that overloaded property modification is disallowed properly on Threaded objects
--DESCRIPTION--
Previously, writes like threaded->nonThreaded->field = something would just silently fail, leading to unexpected code behaviour.
Now, we explicitly disallow any property and dimension reads which are not BP_VAR_R or BP_VAR_IS, which causes errors to be
thrown on attempts to indirectly modify properties in this manner.
This test verifies that these cases actually throw errors like they are supposed to.
--FILE--
<?php

$std = new \stdClass;
$std->something = new \stdClass;

$threaded = new \ThreadedBase;
$threaded->std = $std;
try{
	$threaded->std->something = "this won't work";
}catch(\Error $e){
	echo $e->getMessage() . PHP_EOL;
}

$threaded->threaded = new \ThreadedBase;
$threaded->threaded->something = "this is also ok";
echo $threaded->threaded->something . PHP_EOL;
?>
--EXPECT--
Indirect modification of non-Threaded members of ThreadedBase is not supported
this is also ok
