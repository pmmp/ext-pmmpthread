--TEST--
Test that immutable functions on mutable classes are handled properly
--DESCRIPTION--
Inherited immutable functions might appear on classes that weren't preloaded.
These should not be copied, same as immutable classes.
--SKIPIF--
<?php
if(defined('PHP_WINDOWS_VERSION_MAJOR')) die("skip: preloading is not supported on Windows");
if(!extension_loaded("Zend OPcache")) die("skip: this test requires opcache");
--INI--
opcache.preload={PWD}/assets/preload.php
opcache.enable=1
opcache.enable_cli=1
--FILE--
--FILE--
<?php

class ChildClass extends SomeClass{

}

$w = new \Worker();
$w->start(PTHREADS_INHERIT_ALL);
$w->stack(new class extends \Threaded{
	public function run() : void{
		(new ChildClass())->inheritedFunc();
	}
});
$w->shutdown();
?>
--EXPECT--
string(2) "ok"
