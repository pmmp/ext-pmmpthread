--TEST--
Test that closure scope is properly restored when copying closures from dead child to parent thread
--DESCRIPTION--
User classes on a child thread won't be available to use as a scope when a child thread goes out of scope.
Therefore, we need to reference the scope classes in a safe way that won't be affected by origin thread death.
--XFAIL--
This bug has not been fixed yet
--XLEAK--
This bug has not been fixed yet
--FILE--
<?php

$t = new class extends \pmmp\thread\Thread{
	public $closure;

	public function run() : void{
		require __DIR__ . '/assets/ExternalClosureDefinitionChildToParent.php';

		$this->closure = ExternalClosureDefinitionChildToParent::getClosure();
	}
};

$t->start(\pmmp\thread\Thread::INHERIT_ALL) && $t->join();
($t->closure)();
echo "OK\n";
?>
--EXPECT--
string(38) "ExternalClosureDefinitionChildToParent"
string(38) "ExternalClosureDefinitionChildToParent"
OK

