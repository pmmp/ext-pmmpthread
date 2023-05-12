--TEST--
Test nested ThreadSafe objects
--DESCRIPTION--
This test verifies the possibility to nest ThreadSafe objects
--FILE--
<?php
class Node extends \pmmp\thread\ThreadSafe {}

class TestNestedWrite extends \pmmp\thread\Thread {
    private $shared;

    public function __construct(\pmmp\thread\ThreadSafeArray $shared) {
        $this->shared = $shared;
    }

    public function run() : void{
        var_dump($this->shared['queue'][0]);

        // link to new var
        $this->shared['queue'][1] = $this->shared['queue'][0];

        // unset old ref
        //unset($this->shared['queue'][0]);

        // or replace ref
        $this->shared['queue'][0] = new \pmmp\thread\ThreadSafeArray();

        $this->shared['lock'] = true;
        $this->shared->synchronized(function() : void{
            $this->shared->notify();
        });

        $this->shared->synchronized(function() : void{
            while(!isset($this->shared['lock2'])) {
                $this->shared->wait();
            }
        });

        var_dump($this->shared['queue'][1]);

        $this->shared['lock3'] = true;
        $this->shared->synchronized(function() : void{
            $this->shared->notify();
        });
    }
}

class TestNestedRead extends \pmmp\thread\Thread {
    private $shared;

    public function __construct(\pmmp\thread\ThreadSafeArray $shared) {
        $this->shared = $shared;
    }

    public function run() : void{
        $this->shared->synchronized(function() : void{
            while(!isset($this->shared['lock'])){
                $this->shared->wait();
            }
        });

        var_dump($this->shared['queue'][1]);

        $this->shared['queue'][1] = new Node();
        $this->shared['lock2'] = true;
        $this->shared->synchronized(function() : void{
            $this->shared->notify();
        });

        $this->shared->synchronized(function() : void{
            while(!isset($this->shared['quit'])){
                $this->shared->wait();
            }
        });
    }
}

class Test extends \pmmp\thread\Thread {
    public function run() : void{
        $queue = new \pmmp\thread\ThreadSafeArray();
        $queue[0] = new \pmmp\thread\ThreadSafeArray();

        $shared = new \pmmp\thread\ThreadSafeArray();
        $shared['queue'] = $queue;

        $thread = new TestNestedWrite($shared);
        $thread->start();

        $thread2 = new TestNestedRead($shared);
        $thread2->start();

        $shared->synchronized(function() use ($shared) : void{
            while(!isset($shared['lock3'])){
                $shared->wait();
            }
        });

        $shared['quit'] = true;
        $shared->synchronized(function() use ($shared) : void{
            $shared->notify();
        });

        $thread2->join();
        $thread->join();
    }
}
$thread = new Test();
$thread->start();
$thread->join();
?>
--EXPECT--
object(pmmp\thread\ThreadSafeArray)#4 (0) {
}
object(pmmp\thread\ThreadSafeArray)#4 (0) {
}
object(Node)#4 (0) {
}

