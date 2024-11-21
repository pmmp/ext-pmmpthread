--TEST--
Testing interfaces inheritance (#124)
--DESCRIPTION--
Test if interfaces are stored in thread
--FILE--
<?php
interface iMY{
    function getTest();
}

class MY implements iMY{
    private $test = "Hello World";

    public function getTest(){
            return $this->test;
    }
}

class TEST extends \pmmp\thread\Thread {
    public function __construct() {

    }

    public function run() : void{
	    $MY = new MY();
        var_dump(in_array("iMY",get_declared_interfaces()));
	    var_dump(in_array("iMY",class_implements($MY)));
    }
}

$test = new TEST();
$test->start(\pmmp\thread\Thread::INHERIT_ALL);
?>
--EXPECT--
bool(true)
bool(true)
