--TEST--
Test nested Threaded objects
--DESCRIPTION--
This test verifies the possibility to nest Threaded objects
--FILE--
<?php
class Node extends ThreadedBase {}

class TestNestedWrite extends Thread {
    private $shared;

    public function __construct(ThreadedArray $shared) {
        $this->shared = $shared;
    }

    public function run() {
        var_dump($this->shared['queue'][0]);

        // link to new var
        $this->shared['queue'][1] = $this->shared['queue'][0];

        // unset old ref
        //unset($this->shared['queue'][0]);

        // or replace ref
        $this->shared['queue'][0] = new ThreadedArray();

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

    public function __construct(ThreadedArray $shared) {
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
        $queue = new ThreadedArray();
        $queue[0] = new ThreadedArray();

        $shared = new ThreadedArray();
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
object(ThreadedArray)#4 (0) {
}
object(ThreadedArray)#4 (0) {
}
object(Node)#4 (0) {
}

