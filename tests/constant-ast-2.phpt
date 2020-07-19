--TEST--
Test that constants and field defaults which refer to other constants have their values copied properly
--DESCRIPTION--
Test for IS_CONSTANT_AST const values and field defaults not being handled correctly, a la #564 but on a bigger scale.
--FILE--
<?php

const TEST = 1;

class C{
	public const TEST4 = TEST;

	public static $FIELD = TEST;

	public $field2 = TEST;
}

$thread = new class extends Thread{
	public function run(){
		var_dump(TEST);
		var_dump(C::TEST4, C::$FIELD, (new C)->field2);
	}
};
$thread->start(PTHREADS_INHERIT_ALL);
$thread->join();

//it seems like these are copied correctly if they are accessed before the thread is started, so we avoid that
var_dump(TEST);
var_dump(C::TEST4, C::$FIELD, (new C)->field2);
?>
--EXPECT--
int(1)
int(1)
int(1)
int(1)
int(1)
int(1)
int(1)
int(1)
