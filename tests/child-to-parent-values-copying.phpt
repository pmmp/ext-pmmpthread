--TEST--
Test that values are hard-copied when necessary
--DESCRIPTION--
Some values like interned strings and immutable arrays are not refcounted, but still have to be copied.
This test ensures that we still copy interned stuff when needed to avoid UAF.
--FILE--
<?php

class Test extends \Thread{

	public $permanentInternedString;

	public $requestInternedString;

	public $refcountedString;

	public $emptyArray;

	public $constArray;

	public $refcountedArray;

	public function run() : void{
		require __DIR__ . '/child-to-parent-values-copying.inc';
		assignStuff($this);
	}
}
$t = new Test();
$t->start() && $t->join();

var_dump($t);
?>
--EXPECT--
object(Test)#1 (6) {
  ["permanentInternedString"]=>
  string(6) "Thread"
  ["requestInternedString"]=>
  string(27) "i am a request-local string"
  ["refcountedString"]=>
  string(15) "hellohellohello"
  ["emptyArray"]=>
  array(0) {
  }
  ["constArray"]=>
  array(3) {
    [0]=>
    int(1)
    [1]=>
    int(2)
    [2]=>
    int(3)
  }
  ["refcountedArray"]=>
  array(3) {
    [0]=>
    int(1)
    [1]=>
    int(2)
    [2]=>
    int(3)
  }
}
