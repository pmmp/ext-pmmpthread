<?php

use pmmp\thread\Runnable;
use pmmp\thread\Thread;
use pmmp\thread\ThreadSafeArray;
use pmmp\thread\Worker;

/*
 * This example takes a Closure and executes it on another thread. Calling result() will block until the task has
 * finished, and then return the result.
 *
 * This example uses Worker threads. Workers are reusable threads which execute Runnable tasks given to them.
 * This is significantly more efficient than creating a new Thread for each task, as Threads are extremely costly to
 * create.
 */

class Future{
	public function __construct(private Task $task){}

	public function result() : mixed{
		return $this->task->synchronized(function(){
			while(!$this->task->hasResult){
				$this->task->wait();
			}
			return $this->task->result;
		});
	}
}

class Task extends Runnable{
	private ThreadSafeArray $args;

	public bool $hasResult = false;
	public mixed $result;

	public function __construct(private \Closure $c, array $args){
		$this->args = ThreadSafeArray::fromArray($args); //this will ensure no non-thread-safe objects are passed
	}

	public function run() : void{
		$this->result = ($this->c)(...(array) $this->args);
		$this->hasResult = true;
		$this->synchronized(fn() => $this->notify());
	}
}

function runInThread(\Closure $closure, array $args = []) : Future{
	static $worker = null;
	if($worker === null){
		$worker = new Worker();

		/*
		 * You really, really don't want to use INHERIT_ALL in a production application - it's really slow and wastes lots of memory
		 * Prefer INHERIT_NONE if you can autoload your code and don't set any INI entries
		 * In this example code, it's used because we're in a single-file script and don't have an autoloader - if
		 * we used INHERIT_NONE the classes within this file wouldn't be available to the worker
		 */
		$worker->start(Thread::INHERIT_ALL);
	}
	$future = new Task($closure, $args);
	$worker->stack($future);

	return new Future($future);
}

/*
 * NOTE: If your function is this simple, it's probably not worth using threads to run it, as anything involving
 * threads has inherent overhead, even if workers are used (as in this example).
 *
 * In real code you would do some actual work in this function, like expensive computation, a slow web request, etc.
 */
$args = ["hello", "world"];
$closure = function(string $hello, string $world) : string{
	return $hello . " " . $world;
};

/*
 * Request the closure to be executed in the background
 */
$future = runInThread($closure, $args);

/*
 * result() will block until the task has finished, and then return the result
 *
 * This example is pretty pointless, as this call will block until the thread has done its work, making the thread
 * redundant.
 * Normally you would involve some kind of event loop to do other work while the thread is doing its work, and then
 * come back to the future later
 */
var_dump($future->result());

/* The result of running the closure on the main thread should be the same */
var_dump($closure(...$args));

/* You can also use first-class callables and Closure::fromCallable() */
$future = runInThread(Closure::fromCallable("phpversion"), ["pmmpthread"]);
var_dump($future->result());
$future = runInThread(phpversion(...), ["pmmpthread"]);
var_dump($future->result());
