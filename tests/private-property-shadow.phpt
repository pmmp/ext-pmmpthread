--TEST--
Test that private properties are correctly shadowed
--DESCRIPTION--
Child classes shouldn't be able to access parent private properties by defining a property with the same name.
Regardless of whether we verify visibility or not, private properties are supposed to be exclusive to a class and
multiple private properties in a hierarchy with the same name should not become one.
--FILE--
<?php

class A extends \pmmp\thread\ThreadSafe{
	private $test = 1;

	protected function dump() : void{
		var_dump($this->test);
	}
}

class B extends A{
	public $test = 3;

	public function run() : void{
		$this->test = 2;
		parent::dump();
		var_dump($this->test);
	}
}

$t = new \pmmp\thread\Worker();

$t->start(\pmmp\thread\Thread::INHERIT_ALL);
$t->stack(new class extends \pmmp\thread\Runnable{
	public function run() : void{
		$b = new B;
		$b->run();
	}
});
$t->shutdown();

$b = new B;
$b->run();
?>
--EXPECT--
int(1)
int(2)
int(1)
int(2)
