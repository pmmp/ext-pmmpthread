--TEST--
Test that ArrayAccess works properly when implemented by ThreadedBase
--FILE--
<?php

$t = new class extends \ThreadedBase implements ArrayAccess{
	public function offsetGet($offset) : mixed{
		var_dump("get: $offset");
		return null;
	}

	public function offsetSet($offset, $value) : void{
		var_dump("set: $offset, $value");
	}

	public function offsetExists($offset) : bool{
		var_dump("exists: $offset");
		return false;
	}

	public function offsetUnset($offset) : void{
		var_dump("unset: $offset");
	}
};
$t[] = "hi";
$t["hello"] = "hi 2";
var_dump(isset($t["hello"]));
var_dump($t["hello"]);
unset($t["hello"]);
var_dump($t["hello"]);
?>
--EXPECT--
string(9) "set: , hi"
string(16) "set: hello, hi 2"
string(13) "exists: hello"
bool(false)
string(10) "get: hello"
NULL
string(12) "unset: hello"
string(10) "get: hello"
NULL
