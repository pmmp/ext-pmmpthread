--TEST--
Test constant AST copy bug #495
--DESCRIPTION--
AST in op array literals caused corrupted heaps because it wasn't copied properly.
--FILE--
<?php
class Test
{
    private static function getClassPropertyNames(
		string $classname, 
		$view = \ReflectionProperty::IS_PUBLIC | \ReflectionProperty::IS_PROTECTED) {
	}
}

$t = new class extends \pmmp\thread\Thread {
    public function run() : void{
        throw new \Exception();
    }
};

$t->start(\pmmp\thread\Thread::INHERIT_ALL) && $t->join();
?>
--EXPECTF--
Fatal error: Uncaught Exception in %s:12
Stack trace:
#0 [internal function]: %s@anonymous->run()
#1 {main}
  thrown in %s on line 12


