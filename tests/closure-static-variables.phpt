--TEST--
Test that Closures with static variables cannot be assigned to thread-safe objects
--DESCRIPTION--
We can't prevent static variables from gaining non-thread-safe static variable values after assignment to thread-safe objects, so they must be completely prohibited.
--FILE--
<?php

$func = function() : void{
	static $a = 1;
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
