--TEST--
Test that inherited static properties don't get split up during class copying
--DESCRIPTION--
Class statics inherited from parent class were getting copied as if child and parent both had a definition for a static
property with the same name.
--FILE--
<?php

class A extends \ThreadedRunnable{
	public static $a = [];

	public function run() {}
}
class B extends A{

	public function run(){
		echo "---worker thread start---\n";
		doTest();
		echo "---worker thread end---\n";
	}
}

function doTest() : void{
	B::$a[] = 1;
	var_dump(B::$a);
	var_dump(A::$a);
}

$t = new \Worker();
$t->start();

$t->stack(new B);
$t->shutdown();


echo "---main thread start---\n";
doTest();
echo "---main thread end---\n";
?>
--EXPECT--
---worker thread start---
array(1) {
  [0]=>
  int(1)
}
array(1) {
  [0]=>
  int(1)
}
---worker thread end---
---main thread start---
array(1) {
  [0]=>
  int(1)
}
array(1) {
  [0]=>
  int(1)
}
---main thread end---
