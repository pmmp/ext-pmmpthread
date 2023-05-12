--TEST--
Tests that copied closures have the correct scope
--DESCRIPTION--
pthreads was incorrectly assigning the current class scope to closures when copying them, resulting in some broken behaviour.
In addition, the called scope and original scope were the wrong way round.
--FILE--
<?php

class A{
	public static function getClosure() : \Closure{
		return function() : void{
			var_dump(static::class);
			var_dump(self::class);
		};
	}
}

class B extends A{
	public static function getBoundClosure() : \Closure{
		$c = parent::getClosure();
		$c();
		return $c;
	}

}

$closure = B::getBoundClosure();

$t = new class extends \pmmp\thread\Thread{
	public $closure;

	public function run() : void{
		($this->closure)();
		B::getBoundClosure();
	}
};
$t->closure = $closure;
$t->start() && $t->join();
?>
--EXPECT--
string(1) "B"
string(1) "A"
string(1) "B"
string(1) "A"
string(1) "B"
string(1) "A"
