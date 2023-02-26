--TEST--
Test that closures with non-thread-safe $this cannot be assigned to thread-safe objects
--DESCRIPTION--
We weren't copying closure $this, causing them to suddenly become NULL on the destination thread.
We might be able to copy $this if $this is a thread-safe object, but in all other cases, this won't be possible.
The limitations of this must be made clear with clear errors.
--FILE--
<?php

class A{
	public function getClosure() : \Closure{
		return function() : void{
			var_dump($this);
		};
	}
}

$a = new A();
$closure = $a->getClosure();

$t = new \ThreadedArray();
try{
	$t["closure"] = $closure;
}catch(\Error $e){
	echo $e->getMessage() . PHP_EOL;
}

?>
--EXPECT--
Cannot assign non-thread-safe value of type object to ThreadedArray
