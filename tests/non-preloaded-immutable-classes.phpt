--TEST--
Test that non-preloaded classes marked immutable by OPCache are correctly "copied"
--DESCRIPTION--
These classes aren't actually supposed to be copied at all because they are immutable, but they
should be available just like a regular class regardless.
--SKIPIF--
<?php
if(!extension_loaded("Zend OPcache")) die("skip: this test requires opcache");
--INI--
opcache.enable=1
opcache.enable_cli=1
--FILE--
<?php

require __DIR__ . '/assets/preloaded-stuff.php';

$w = new Worker;
$w->start(PTHREADS_INHERIT_ALL);
$w->stack(new class extends \Threaded{
	public function run() : void{
		var_dump(class_exists(SomeClass::class));
		iAmPreloaded();
	}
});
$w->shutdown();
--EXPECT--
bool(true)
string(3) "yes"
