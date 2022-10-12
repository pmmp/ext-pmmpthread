--TEST--
Test iterating on Threaded
--DESCRIPTION--
Regression test for bugs introduced with Threaded iteration in PHP 7.3
--FILE--
<?php
$threaded = new Threaded();

var_dump($threaded->count());
foreach($threaded as $k => $prop){
	var_dump("should not happen");
}

for($i = 0; $i < 5; ++$i){
	$threaded[] = "value$i";
	$threaded["key$i"] = "string$i";
}

$threaded["threaded"] = new Threaded();

foreach($threaded as $i => $prop){
	var_dump($i, $prop);
}
?>
--EXPECTF--
int(0)
int(0)
string(6) "value0"
string(4) "key0"
string(7) "string0"
int(1)
string(6) "value1"
string(4) "key1"
string(7) "string1"
int(2)
string(6) "value2"
string(4) "key2"
string(7) "string2"
int(3)
string(6) "value3"
string(4) "key3"
string(7) "string3"
int(4)
string(6) "value4"
string(4) "key4"
string(7) "string4"
string(8) "threaded"
object(Threaded)#2 (0) {
}
