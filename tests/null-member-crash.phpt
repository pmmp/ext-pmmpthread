--TEST--
Null member crash
--DESCRIPTION--
This test verifies that null members do not crash php
--FILE--
<?php
class Test{
	public function run() : void{}
}

class Test2 extends \pmmp\thread\ThreadSafe{
	public function run() : void{}
}
$test = new Test();
@$test->{$undefined} = "what";
var_dump($test);

$test2 = new Test2();
@$test2->{$undefined} = "what";
var_dump($test2);
?>
--EXPECTF--
object(Test)#2 (1) {
  [""]=>
  string(4) "what"
}
object(Test2)#3 (1) {
  [""]=>
  string(4) "what"
}
