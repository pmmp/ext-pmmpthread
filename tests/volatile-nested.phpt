--TEST--
Test nested volatile variables
--DESCRIPTION--
This test verifies the possibility to nest volatile variables
--FILE--
<?php
class Node extends Volatile {}

class TestNestedWrite extends Thread {
    private $shared;

    public function __construct(Volatile $shared) {
        $this->shared = $shared;
    }

    public function run() {
        var_dump($this->shared['queue'][0]);

        // link to new var
        $this->shared['queue'][1] = $this->shared['queue'][0];

        // unset old ref
        //unset($this->shared['queue'][0]);

        // or replace ref
        $this->shared['queue'][0] = new Volatile();

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

class TestNestedRead extends Thread {
    private $shared;

    public function __construct(Volatile $shared) {
        $this->shared = $shared;
    }

    public function run() {
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

class Test extends Thread {
    public function run() {
        $queue = new Volatile();
        $queue[0] = new Volatile();

        $shared = new Volatile();
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
object(Volatile)#4 (0) {
}
object(Volatile)#4 (0) {
}
object(Node)#4 (0) {
}

