--TEST--
Test pool defaults
--DESCRIPTION--
This test verifies pool defaults
--FILE--
<?php
class Work extends \pmmp\thread\Runnable {
	public function run() : void{
		var_dump($this);
	}
}

$pool = new \pmmp\thread\Pool(1);
$pool->submit(new Work());
$pool->shutdown();

$pool->collect();

var_dump($pool);
?>
--EXPECTF--
object(Work)#%d (%d) {
}
object(pmmp\thread\Pool)#%d (%d) {
  ["size":protected]=>
  int(1)
  ["class":protected]=>
  string(18) "pmmp\thread\Worker"
  ["workers":protected]=>
  array(0) {
  }
  ["ctor":protected]=>
  NULL
  ["last":protected]=>
  int(1)
}



