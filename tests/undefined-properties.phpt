--TEST--
Test that ThreadSafe undefined properties have the correct behaviour
--FILE--
<?php

#[\AllowDynamicProperties]
class T extends \pmmp\thread\ThreadSafe{
	public static $iAmStatic = 1;
};

#[\AllowDynamicProperties]
class T2{
	public static $iAmStatic = 1;
}

function test(object $t) : void{
	var_dump(isset($t->doesntExist)); //false, no error
	var_dump($t->doesntExist); //NULL + warning
	unset($t->doesntExist); //no-op

	$t->exists = 1; //works
	var_dump(isset($t->exists)); //true, no error
	var_dump($t->exists); //works
	unset($t->exists); //works
	var_dump(isset($t->exists)); //false, no error

	var_dump(isset($t->iAmStatic)); //false, E_NOTICE
	var_dump(isset($t::$iAmStatic)); //true
	var_dump($t->iAmStatic); //NULL, E_NOTICE
	var_dump($t::$iAmStatic); //1

	unset($t->iAmStatic);
	var_dump($t::$iAmStatic); //1, still exists
	$t->iAmStatic = 2;
	var_dump($t->iAmStatic); //2
	var_dump($t::$iAmStatic); //still 1
}

echo "--- test normal object ---\n";
test(new T2);
echo "--- test thread-safe object ---\n";
test(new T);
echo "--- done ---\n";

?>
--EXPECTF--
--- test normal object ---
bool(false)

Warning: Undefined property: %s::$doesntExist in %s on line %d
NULL
bool(true)
int(1)
bool(false)
bool(false)
bool(true)

Notice: Accessing static property %s::$iAmStatic as non static in %s on line %d

Warning: Undefined property: %s::$iAmStatic in %s on line %d
NULL
int(1)

Notice: Accessing static property %s::$iAmStatic as non static in %s on line %d
int(1)

Notice: Accessing static property %s::$iAmStatic as non static in %s on line %d

Notice: Accessing static property %s::$iAmStatic as non static in %s on line %d
int(2)
int(1)
--- test thread-safe object ---
bool(false)

Warning: Undefined property: %s::$doesntExist in %s on line %d
NULL
bool(true)
int(1)
bool(false)
bool(false)
bool(true)

Notice: Accessing static property %s::$iAmStatic as non static in %s on line %d

Warning: Undefined property: %s::$iAmStatic in %s on line %d
NULL
int(1)

Notice: Accessing static property %s::$iAmStatic as non static in %s on line %d
int(1)

Notice: Accessing static property %s::$iAmStatic as non static in %s on line %d

Notice: Accessing static property %s::$iAmStatic as non static in %s on line %d
int(2)
int(1)
--- done ---
