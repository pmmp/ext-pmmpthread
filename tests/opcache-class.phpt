--TEST--
Test that OPCache persisted classes are used when available
--SKIPIF--
<?php if(!extension_loaded("Zend OPcache")) die("skip: this test requires opcache");
--INI--
opcache.preload={PWD}/assets/preload.php
opcache.enable=1
opcache.enable_cli=1
--FILE--
<?php

SomeClass::$var = 2;

$w = new Worker;
$w->start(PTHREADS_INHERIT_NONE);
$w->stack(new class extends \Threaded{
	public function run() : void{
		var_dump(SomeClass::$var);
	}
});
$w->shutdown();
--EXPECT--
int(1)
