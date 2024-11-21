--TEST--
Test return types
--DESCRIPTION--
This test verifies that functions with a typed return are copied and execute properly
--FILE--
<?php
function some() : string {
	return __FUNCTION__;
}

$thread = new class extends \pmmp\thread\Thread {
	public function run() : void{
		var_dump(some());
	}
};

$thread->start(\pmmp\thread\Thread::INHERIT_ALL);
$thread->join();
--EXPECT--
string(4) "some"
