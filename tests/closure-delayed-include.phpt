--TEST--
Closure copying of an included closure
--DESCRIPTION--
We need to copy closures which are defined and bound by included classes
--FILE--
<?php

class Foo extends \pmmp\thread\Thread {
    /** @var bool */
    public $running;

    /** @var \pmmp\thread\ThreadSafeArray */
    private $shared;

    public function __construct(\pmmp\thread\ThreadSafeArray $shared) {
        $this->shared = $shared;
    }

    public function run() : void{
        $this->running = true;

        require __DIR__ .'/assets/ExternalClosureDefinition.php';

        $this->shared['loader'] = new ExternalClosureDefinition();
        $this->synchronized(function() : void{
            $this->notify();
        });
        $this->synchronized(function () {
            while($this->running) {
                $this->wait();
            }
        });
    }
}

$shared = new \pmmp\thread\ThreadSafeArray();

$foo = new Foo($shared);
$foo->start(\pmmp\thread\Thread::INHERIT_ALL);

$foo->synchronized(function() use ($foo, $shared) : void{
    while(!isset($shared['loader'])) {
        $foo->wait();
    }
});

$closureDefinition = $shared['loader'];
$closureDefinition->load();
$foo->running = false;
$foo->synchronized(function() use ($foo) : void{
    $foo->notify();
});
$foo->join();
--EXPECT--
string(11) "Hello World"
