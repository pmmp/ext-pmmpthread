--TEST--
Test that unsupported types are correctly rejected
--FILE--
<?php

$threaded = new ThreadedArray;
foreach([
	[],
	[1, 2, 3],
	array_fill(0, 10, 0),
	new \stdClass,
	tmpfile()
] as $bannedType){
	try{
		$threaded[] = $bannedType;
	}catch(\Error $e){
		echo $e->getMessage() . PHP_EOL;
	}
}

$tsObj = new class extends \ThreadedBase{
	public $ref = 2;
};
$var = 1;
try{
	$tsObj->ref = &$var;
	echo "references should not work" . PHP_EOL;
}catch(\Error $e){
	echo $e->getMessage() . PHP_EOL;
}
?>
--EXPECT--
Cannot assign non-thread-safe value of type array to ThreadedArray
Cannot assign non-thread-safe value of type array to ThreadedArray
Cannot assign non-thread-safe value of type array to ThreadedArray
Cannot assign non-thread-safe value of type stdClass to ThreadedArray
Cannot assign non-thread-safe value of type resource to ThreadedArray
Indirect modification of non-ThreadedBase members of ThreadedBase@anonymous is not supported

