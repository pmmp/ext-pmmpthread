--TEST--
Test that values are hard-copied when necessary
--DESCRIPTION--
Some values like interned strings and immutable arrays are not refcounted, but still have to be copied.
This test ensures that we still copy interned stuff when needed to avoid UAF.
--FILE--
<?php

class Test extends \pmmp\thread\Thread{

	public $permanentInternedString;

	public $requestInternedString;

	public $refcountedString;

	public function run() : void{
		require __DIR__ . '/child-to-parent-values-copying.inc';
		assignStuff($this);
	}
}
$t = new Test();
$t->start(\pmmp\thread\Thread::INHERIT_ALL) && $t->join();

var_dump($t);
?>
--EXPECT--
object(Test)#1 (3) {
  ["permanentInternedString"]=>
  string(18) "pmmp\thread\Thread"
  ["requestInternedString"]=>
  string(27) "i am a request-local string"
  ["refcountedString"]=>
  string(15) "hellohellohello"
}
