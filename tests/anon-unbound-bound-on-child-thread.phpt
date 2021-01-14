--TEST--
Test that all the stuff needed to independently link an anonymous class gets copied
--DESCRIPTION--
Anonymous classes which are copied while unbound need to be properly linkable by the child thread,
because it's possible the child thread may use them without them being completed by the main thread.
--FILE--
<?php

trait Trait1{
	public function dummyTraitFunc(){
		return __METHOD__;
	}

	public function aliasedDummyTraitFunc(){
		return __METHOD__;
	}
}

trait Trait2{
	public function anotherTraitFunc(){
		return __METHOD__;
	}

	public function anotherAliasedTraitFunc(){
		return __METHOD__;
	}
}

interface Interfaz{
	public const A_CONSTANT = 1;
}

interface Interfaz2{
	public const ANOTHER_CONST = 2;
}

class FirstParent{

	public const FIRST_CONST = 1;
	public static $firstParentProp = 1;

	public function firstFunc(){
		return __METHOD__;
	}
}

class SecondParent extends FirstParent implements Interfaz2{
	use Trait1 {
		aliasedDummyTraitFunc as dummyTraitFuncAliased;
	}

	public const SECOND_CONST = 2;
	public static $secondParentProp = 2;

	public function secondFunc(){
		return __METHOD__;
	}
}

function returnsAnonymousClass(){
	return new class extends SecondParent implements Interfaz{
		use Trait2 {
			anotherAliasedTraitFunc as aliasedTraitFuncAnother;
		}

		public const MY_OWN_CONST = 3;
		public static $myOwnStaticProp = 3;

		public function myOwnFunc(){
			return __METHOD__;
		}
	};
}

function test(){
	$anon = returnsAnonymousClass();
	var_dump($anon instanceof Interfaz);
	var_dump($anon instanceof Interfaz2);
	var_dump($anon instanceof FirstParent);
	var_dump($anon instanceof SecondParent);

	var_dump($anon->dummyTraitFunc());
	var_dump($anon->dummyTraitFuncAliased());
	var_dump($anon->anotherTraitFunc());
	var_dump($anon->aliasedTraitFuncAnother());
	var_dump($anon->myOwnFunc());

	var_dump($anon::A_CONSTANT);
	var_dump($anon::ANOTHER_CONST);
	var_dump($anon::FIRST_CONST);
	var_dump($anon::SECOND_CONST);
	var_dump($anon::MY_OWN_CONST);

	var_dump($anon::$firstParentProp);
	var_dump($anon::$secondParentProp);
	var_dump($anon::$myOwnStaticProp);
}
$worker = new Worker();
$worker->start();
$worker->stack(new class extends \Threaded{
	public function run() : void{
		echo "--- worker thread start ---\n";
		test();
		echo "--- worker thread end ---\n";
	}
});
$worker->shutdown();
echo "--- main thread start ---\n";
test();
echo "--- main thread end ---\n";
--EXPECTF--
--- worker thread start ---
bool(true)
bool(true)
bool(true)
bool(true)
string(22) "Trait1::dummyTraitFunc"
string(29) "Trait1::aliasedDummyTraitFunc"
string(24) "Trait2::anotherTraitFunc"
string(31) "Trait2::anotherAliasedTraitFunc"
string(%d) "%s@anonymous%s::myOwnFunc"
int(1)
int(2)
int(1)
int(2)
int(3)
int(1)
int(2)
int(3)
--- worker thread end ---
--- main thread start ---
bool(true)
bool(true)
bool(true)
bool(true)
string(22) "Trait1::dummyTraitFunc"
string(29) "Trait1::aliasedDummyTraitFunc"
string(24) "Trait2::anotherTraitFunc"
string(31) "Trait2::anotherAliasedTraitFunc"
string(%d) "%s@anonymous%s::myOwnFunc"
int(1)
int(2)
int(1)
int(2)
int(3)
int(1)
int(2)
int(3)
--- main thread end ---
