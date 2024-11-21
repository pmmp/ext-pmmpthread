--TEST--
Test Worker::unstack while a task is currently executing
--DESCRIPTION--
Unstacking a task would cause it to be freed from the worker stack, but a currently-executing task would not be told not to point to the destroyed task, resulting in an attempted double-free when collecting garbage from workers
--FILE--
<?php
$w = new \pmmp\thread\Worker();
$w->start(\pmmp\thread\Thread::INHERIT_ALL);

class Task extends \pmmp\thread\Runnable{
    public bool $running = false;
    public bool $continue = false;

    public function run() : void{
        $this->synchronized(function() : void{
            $this->running = true;
            $this->notify();
        });
        $this->synchronized(function() : void{
            while(!$this->continue){
                $this->wait();
            }
        });
    }
}

$task = new Task;
$w->stack($task);
$w->stack(new class extends Task{
    public function run() : void{
        //NOOP
    }
});

$task->synchronized(function() use ($task) : void{
    while(!$task->running){
        $task->wait();
    }
});
$w->unstack();
$task->synchronized(function() use ($task) : void{
    $task->continue = true;
    $task->notify();
});

$w->shutdown();

var_dump("ok");
?>
--EXPECTF--
string(2) "ok"
