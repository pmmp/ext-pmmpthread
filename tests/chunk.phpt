--TEST--
Check chunking
--DESCRIPTION--
This test verifies functionality of ::chunk
--FILE--
<?php
class S extends Threaded {
    public function run(){}
}

$s = new S();
$s->merge(array_fill(0, 10000, true));

var_dump($s->chunk(10));

$s = new S();
$s->merge(array_fill_keys(array_map(fn(int $i) => "key$i", range(0, 1000)), true));

var_dump($s->chunk(10));

?>
--EXPECT--
array(10) {
  [0]=>
  bool(true)
  [1]=>
  bool(true)
  [2]=>
  bool(true)
  [3]=>
  bool(true)
  [4]=>
  bool(true)
  [5]=>
  bool(true)
  [6]=>
  bool(true)
  [7]=>
  bool(true)
  [8]=>
  bool(true)
  [9]=>
  bool(true)
}
array(10) {
  ["key0"]=>
  bool(true)
  ["key1"]=>
  bool(true)
  ["key2"]=>
  bool(true)
  ["key3"]=>
  bool(true)
  ["key4"]=>
  bool(true)
  ["key5"]=>
  bool(true)
  ["key6"]=>
  bool(true)
  ["key7"]=>
  bool(true)
  ["key8"]=>
  bool(true)
  ["key9"]=>
  bool(true)
}

