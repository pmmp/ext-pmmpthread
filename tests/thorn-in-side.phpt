--TEST--
Testing thorn in my side hasn't returned
--DESCRIPTION--
This test verifies my nemisis has not returned (bug #48)
--FILE--
<?php
class parentClass {
    private $var;

    public function __construct() {
        echo $this->var;
    }
}

class childClass extends parentClass {

}

class clientThread extends \pmmp\thread\Thread {

    public function run() : void{
        $objChild = new childClass();

    }               

}


$objClientThread = new clientThread();
$objClientThread->start(\pmmp\thread\Thread::INHERIT_ALL);
$objClientThread->join();
echo "OK\n";
--EXPECTF--
OK
