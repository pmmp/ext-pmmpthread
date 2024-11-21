--TEST--
Test that unsupported types are correctly rejected
--FILE--
<?php

$threaded = new \pmmp\thread\ThreadSafeArray;
foreach([
	[],
	[1, 2, 3],
	array_fill(0, 10, 0),
	new \stdClass,
	tmpfile()
] as $bannedType){
	try{
		$threaded[] = $bannedType;
	}catch(\pmmp\thread\NonThreadSafeValueError $e){
		echo $e->getMessage() . PHP_EOL;
	}
}

$tsObj = new class extends \pmmp\thread\ThreadSafe{
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
Cannot assign non-thread-safe value of type array to pmmp\thread\ThreadSafeArray
Cannot assign non-thread-safe value of type array to pmmp\thread\ThreadSafeArray
Cannot assign non-thread-safe value of type array to pmmp\thread\ThreadSafeArray
Cannot assign non-thread-safe value of type stdClass to pmmp\thread\ThreadSafeArray
Cannot assign non-thread-safe value of type resource to pmmp\thread\ThreadSafeArray
Indirect modification of non-ThreadSafe members of pmmp\thread\ThreadSafe@anonymous is not supported

