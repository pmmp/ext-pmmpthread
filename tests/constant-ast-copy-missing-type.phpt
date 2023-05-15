--TEST--
Test constant AST copy bug #564
--DESCRIPTION--
AST in op array literals caused corrupted heaps because it wasn't copied properly. 
--FILE--
<?php
namespace any\name\space {
	class Test extends \pmmp\thread\Thread {
		const KEY = '';

		public function run() : void{
			static $arrConst = [self::KEY => true];

			var_dump($arrConst);
		}
	}
}

namespace {
	use any\name\space\Test;

	$objTest = new Test();
	$objTest->start(\pmmp\thread\Thread::INHERIT_ALL);
	$objTest->join();
}
?>
--EXPECTF--
array(1) {
  [""]=>
  bool(true)
}
