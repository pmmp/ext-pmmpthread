--TEST--
Test that OPCache persisted classes and functions are used when available
--SKIPIF--
<?php
if(defined('PHP_WINDOWS_VERSION_MAJOR')) die("skip: preloading is not supported on Windows");
if(!extension_loaded("Zend OPcache")) die("skip: this test requires opcache");
--INI--
opcache.preload={PWD}/assets/preload.php
opcache.enable=1
opcache.enable_cli=1
--FILE--
<?php

$w = new \pmmp\thread\Worker;
$w->start(\pmmp\thread\Thread::INHERIT_NONE);
$w->stack(new class extends \pmmp\thread\Runnable{
	public function run() : void{
		var_dump(class_exists(SomeClass::class));
		iAmPreloaded();
	}
});
$w->shutdown();
--EXPECT--
bool(true)
string(3) "yes"
