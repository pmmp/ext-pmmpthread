--TEST--
Test Threaded behaviour when used as n array
--DESCRIPTION--
Arrays were difficult to use in pthreads, their behaviour was strange and inconsistent with Zend arrays.

pthreads will coerce arrays to Threaded objects when they are set as members of Threaded objects.

Threaded objects have been made consistent with PHP arrays, so there should be no noticable difference between an array
and a Threaded object.
--FILE--
<?php
$threaded = new Threaded();

$threaded->test = [
	"greeting" => "Hello World", 
	"child" => [
		"of" => "mine",
		"grandchild" => [
			"of" => "parents"
		]
	]
];

/*
 This looks strange, but needs to be consistent with zend, so we'll test here ... 
*/
$threaded["0"] = [];
$threaded[1] = [];
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
$threaded->test["child"]["of"] = "yours";
$threaded->test["child"]["grandchild"]["of"] = "devil";

var_dump($threaded);
?>
--EXPECTF--
object(Threaded)#%d (%d) {
  ["test"]=>
  object(Threaded)#%d (%d) {
    ["greeting"]=>
    string(11) "Hello World"
    ["child"]=>
    object(Threaded)#%d (%d) {
      ["of"]=>
      string(4) "mine"
      ["grandchild"]=>
      object(Threaded)#%d (%d) {
        ["of"]=>
        string(7) "parents"
      }
    }
  }
  [0]=>
  object(Threaded)#%d (%d) {
  }
  [1]=>
  object(Threaded)#%d (%d) {
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
object(Threaded)#%d (%d) {
  ["test"]=>
  object(Threaded)#%d (%d) {
    ["greeting"]=>
    string(11) "Hello World"
    ["child"]=>
    object(Threaded)#%d (%d) {
      ["of"]=>
      string(5) "yours"
      ["grandchild"]=>
      object(Threaded)#%d (%d) {
        ["of"]=>
        string(5) "devil"
      }
    }
  }
  [0]=>
  object(Threaded)#%d (%d) {
  }
  [1]=>
  object(Threaded)#%d (%d) {
  }
}

