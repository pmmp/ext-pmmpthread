--TEST--
Test that unbound anonymous classes correctly inherit properties from parent classes
--DESCRIPTION--
Linking an anonymous class can cause new properties to become available. These should
be available on the copied versions of anonymous classes.
--FILE--
<?php
$worker = new \pmmp\thread\Worker();

$worker->start(\pmmp\thread\Thread::INHERIT_ALL);

class Base extends \pmmp\thread\Runnable {
	public static $staticProp = "staticProp";

	public $prop = "prop";

	public function run() : void{}
}

class Base2 extends Base {
	public static $staticProp2 = "staticProp2";

	public $prop2 = "prop2";
}

$collectable = new class extends Base2 {
	public function run() : void{
		var_dump($this->prop);
		var_dump($this->prop2);
		var_dump(self::$staticProp);
		var_dump(self::$staticProp2);
	}
};

$collectable->run();
$worker->stack($collectable);
$worker->shutdown();
--EXPECT--
string(4) "prop"
string(5) "prop2"
string(10) "staticProp"
string(11) "staticProp2"
string(4) "prop"
string(5) "prop2"
string(10) "staticProp"
string(11) "staticProp2"

