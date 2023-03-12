--TEST--
Test that Closures with use() by reference cannot be assigned to thread-safe objects
--DESCRIPTION--
Use-by-reference cannot be allowed to operate across threads due to safety issues, so closures using references must be rejected clearly.
--FILE--
<?php

$a = 1;
$func = function() use (&$a) : void{
	$a++;
	var_dump($a);
};

$array = new \ThreadedArray();
try{
	$array["func"] = $func;
}catch(\Error $e){
	echo $e->getMessage() . PHP_EOL;
}
?>
--EXPECT--
Cannot assign non-thread-safe value of type Closure to ThreadedArray
