--TEST--
Test that fake closure static variables are shared with the target function
--DESCRIPTION--
Calling a fake closure behaves exactly the same as if the target function was directly invoked.
This means that fake closures don't have their own unique static variables - they use the static variables of the original function.

This test verifies that this behaviour is preserved when first-class callables are copied to another thread.
--FILE--
<?php

class A{
	public static function test() : void{
		static $a = 0;
		$a++;
		var_dump("a = $a");
	}
}

function test(\Closure $fcc) : void{
	$fcc();
	A::test();
}


$fcc = \Closure::fromCallable([A::class, 'test']);
test($fcc);
echo "end\n";
$t = new class($fcc) extends \Thread{
	private \Closure $fcc;

	public function __construct(\Closure $fcc){
		$this->fcc = $fcc;
	}

	public function run() : void{
		test($this->fcc);
	}
};
$t->start();
$t->join();

?>
--EXPECT--
string(5) "a = 1"
string(5) "a = 2"
end
string(5) "a = 1"
string(5) "a = 2"
