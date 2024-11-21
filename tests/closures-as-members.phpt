--TEST--
Testing closure members
--DESCRIPTION--
This test verifies that closures can be set as members and called from anywhere
--FILE--
<?php
$test = new class extends \pmmp\thread\ThreadSafe{
    public $some;
};

$test->some = function(){
    echo "Hello Some\n";
};

class T extends \pmmp\thread\Thread {
    private $test;
    public $used;
    public $set;

    public function __construct($test) {
        $this->test = $test;
    }
    
    public function run() : void{
        /* call original closure */
        $this->call($this->test->some);
        
        /* set new closure */
        $this->synchronized(function(){
            $this->set = true;
            $this->test->some = function() {
                echo "Hello World\n";
            };
            $this->notify();
        });
        
        /* wait for new closure execution */
        $this->synchronized(function(){
            while (!$this->used)
                $this->wait();
        });
    }
    
    public function call(Closure $closure) {
        $closure();
    }
}

/* start thread to call closure */
$t = new T($test);
$t->start(\pmmp\thread\Thread::INHERIT_ALL);

/* wait for new closure */
$t->synchronized(function() use($t) {
    while (!$t->set)
        $t->wait();
});

/* call new closure */
$some = $test->some;
$some();

/* allow new closure to be dtored with old context */
$t->synchronized(function() use($t) {
    $t->used = true;
    $t->notify();
});
$t->join();
unset($t);
--EXPECTF--
Hello Some
Hello World
