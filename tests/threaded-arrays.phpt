--TEST--
Test ThreadSafeArray behaviour
--DESCRIPTION--
Arrays were difficult to use in pthreads, their behaviour was strange and inconsistent with Zend arrays.

ThreadSafeArray objects can mostly serve as a drop-in replacement for an array, with similar behaviour.
--FILE--
<?php
$threaded = new \pmmp\thread\ThreadSafeArray();

$threaded["test"] = \pmmp\thread\ThreadSafeArray::fromArray([
	"greeting" => "Hello World", 
	"child" => [
		"of" => "mine",
		"grandchild" => [
			"of" => "parents"
		]
	]
]);

/*
 This looks strange, but needs to be consistent with zend, so we'll test here ... 
*/
$threaded["0"] = new \pmmp\thread\ThreadSafeArray;
$threaded[1] = new \pmmp\thread\ThreadSafeArray;
$threaded[2] = 'foo';
$threaded['3'] = 'bar';
$threaded[null] = 'null key';
$threaded[] = 'appended';

var_dump($threaded);
var_dump(isset($threaded["0"]));
var_dump(isset($threaded[0]));
var_dump($threaded[null]);
var_dump($threaded[4]);

unset($threaded["2"], $threaded[3], $threaded[null], $threaded[4]);

/*
 This kind of thing would simply fail before, creating really unexpected results
*/
$threaded["test"]["child"]["of"] = "yours";
$threaded["test"]["child"]["grandchild"]["of"] = "devil";

var_dump($threaded);
?>
--EXPECTF--
object(pmmp\thread\ThreadSafeArray)#%d (%d) {
  ["test"]=>
  object(pmmp\thread\ThreadSafeArray)#%d (%d) {
    ["greeting"]=>
    string(11) "Hello World"
    ["child"]=>
    object(pmmp\thread\ThreadSafeArray)#%d (%d) {
      ["of"]=>
      string(4) "mine"
      ["grandchild"]=>
      object(pmmp\thread\ThreadSafeArray)#%d (%d) {
        ["of"]=>
        string(7) "parents"
      }
    }
  }
  [0]=>
  object(pmmp\thread\ThreadSafeArray)#%d (0) {
  }
  [1]=>
  object(pmmp\thread\ThreadSafeArray)#%d (0) {
  }
  [2]=>
  string(3) "foo"
  [3]=>
  string(3) "bar"
  [""]=>
  string(8) "null key"
  [4]=>
  string(8) "appended"
}
bool(true)
bool(true)
string(8) "null key"
string(8) "appended"
object(pmmp\thread\ThreadSafeArray)#%d (%d) {
  ["test"]=>
  object(pmmp\thread\ThreadSafeArray)#%d (%d) {
    ["greeting"]=>
    string(11) "Hello World"
    ["child"]=>
    object(pmmp\thread\ThreadSafeArray)#%d (%d) {
      ["of"]=>
      string(5) "yours"
      ["grandchild"]=>
      object(pmmp\thread\ThreadSafeArray)#%d (%d) {
        ["of"]=>
        string(5) "devil"
      }
    }
  }
  [0]=>
  object(pmmp\thread\ThreadSafeArray)#%d (0) {
  }
  [1]=>
  object(pmmp\thread\ThreadSafeArray)#%d (0) {
  }
}

