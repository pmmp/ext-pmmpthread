--TEST--
Test that fake closure static variables are shared with the target function
--DESCRIPTION--
Calling a fake closure behaves exactly the same as if the target function was directly invoked.
This means that fake closures don't have their own unique static variables - they use the static variables of the original function.

This test verifies that this behaviour is preserved when first-class callables are copied to another thread.
--SKIPIF--
<?php if(PHP_VERSION_ID < 80100) die("skip broken on 8.0 due to broken behaviour in php-src (https://wiki.php.net/rfc/static_variable_inheritance)"); ?>
--FILE--
<?php

class A{
	public static function test() : void{
		static $a = 0;
		$a++;
		var_dump("a = $a");
	}
}

function b() : void{
	static $b = 0;
	$b++;
	var_dump("b = $b");
}

function test(\Closure $fcc) : void{
	$fcc();
	A::test();
}

function test2(\Closure $fcc) : void{
	$fcc();
	b();
}


$fcc = \Closure::fromCallable([A::class, 'test']);
test($fcc);
$fcc2 = \Closure::fromCallable('b');
test2($fcc2);
echo "end\n";
$t = new class($fcc, $fcc2) extends \pmmp\thread\Thread{
	public function __construct(
		private \Closure $fcc,
		private \Closure $fcc2
	){}

	public function run() : void{
		test($this->fcc);
		test2($this->fcc2);
	}
};
$t->start();
$t->join();

?>
--EXPECT--
string(5) "a = 1"
string(5) "a = 2"
string(5) "b = 1"
string(5) "b = 2"
end
string(5) "a = 1"
string(5) "a = 2"
string(5) "b = 1"
string(5) "b = 2"
