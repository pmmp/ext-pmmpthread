--TEST--
Test Threaded::fromArray()
--FILE--
<?php

$array = [
	"greeting" => "Hello World", 
	"child" => [
		"of" => "mine",
		"grandchild" => [
			"of" => "parents"
		]
	]
];
var_dump($array);

$threaded = ThreadedArray::fromArray($array);
var_dump($threaded);
?>
--EXPECTF--
array(2) {
  ["greeting"]=>
  string(11) "Hello World"
  ["child"]=>
  array(2) {
    ["of"]=>
    string(4) "mine"
    ["grandchild"]=>
    array(1) {
      ["of"]=>
      string(7) "parents"
    }
  }
}
object(ThreadedArray)#1 (2) {
  ["greeting"]=>
  string(11) "Hello World"
  ["child"]=>
  object(ThreadedArray)#2 (2) {
    ["of"]=>
    string(4) "mine"
    ["grandchild"]=>
    object(ThreadedArray)#3 (1) {
      ["of"]=>
      string(7) "parents"
    }
  }
}
