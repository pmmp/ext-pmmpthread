--TEST--
Test members (typeof object) with no other references
--DESCRIPTION--
In pthreads v2, you were charged with responsability of the objects you create, however in pthreads v3, a ThreadSafe object
is able to retain a reference to another ThreadSafe object, this makes programming Thread objects a lot simpler, 
the code in this test would fail with pthreads v2 and will work as expected with pthreads v3, which is a win for everyone ...
--FILE--
<?php

class T extends \pmmp\thread\Thread {

	public function __construct() {
		$this->t = new \pmmp\thread\ThreadSafe();
		$this->t->set = true;
	}

    public function run() : void{
		var_dump($this->t, $this);
	}
}

$t = new T();
var_dump($t);
$t->start();
$t->join();
?>
--EXPECTF--
object(T)#%d (%d) {
  ["t"]=>
  object(pmmp\thread\ThreadSafe)#%d (%d) {
    ["set"]=>
    bool(true)
  }
}
object(pmmp\thread\ThreadSafe)#2 (%d) {
  ["set"]=>
  bool(true)
}
object(T)#%d (%d) {
  ["t"]=>
  object(pmmp\thread\ThreadSafe)#%d (%d) {
    ["set"]=>
    bool(true)
  }
}
