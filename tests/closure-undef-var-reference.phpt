--TEST--
Test that &$undefined works correctly in closures
--DESCRIPTION--
ZEND_HASH_FOREACH macros skip IS_UNDEF, which is problematic when copying closure statics in this particular case

This test verifies that bug #112 has not come back
--FILE--
<?php

declare(strict_types=1);

use pmmp\thread\Thread;

class A extends Thread {
	public function __construct(
		private Closure $x
	) {
	}

	public function run() : void {
		($this->x)()();
	}
}

$t = (new A(static fn() => static function() use (&$u) : void {
	echo "HI 2\n";
}));
$t->start(Thread::INHERIT_NONE);
$t->join();
echo "OK\n";
?>
--EXPECT--
HI 2
OK
