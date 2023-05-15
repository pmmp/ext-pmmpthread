<?php

use pmmp\thread\Thread;

/**
* A synchronized Future for a Closure
*
*	This example takes a Closure and executes it in parallel, storing the result
*	and fetching the result are synchronized operations
*
*	This means that a call to getResult() will block the calling context until a result
*	is available
**/
class Future extends Thread {

    private function __construct(Closure $closure, array $args = []) {
        $this->closure = $closure;
        $this->args    = serialize($args);
    }

    public function run() : void {
        $this->synchronized(function() {
            $result = ($this->closure)(...unserialize($this->args));
			$this->result = serialize($result);
            $this->notify();
        });
    }

    public function getResult() {
        return $this->synchronized(function(){
            while (!$this->result)
                $this->wait();
            return unserialize($this->result);
        });
    }
    
    public static function of(Closure $closure, array $args = []) {
        $future = 
            new self($closure, $args);

        /*
         * You really, really don't want to use INHERIT_ALL in a production application - it's really slow and wastes lots of memory
         * Prefer INHERIT_NONE if you can autoload your code and don't set any INI entries
         * In this example code, it's used because we're in a single-file script and don't have an autoloader
         */
        $future->start(Thread::INHERIT_ALL);
        return $future;
    }
    
	protected $owner;
    protected $closure;
    protected $args;
    protected $result;
}

/* some data */
$test = ["Hello", "World"];

/* a closure to execute in background and foreground */
$closure = function($test) {
    return $test;
};

/* make call in background thread */
$future = Future::of($closure, [$test]);

/* get result of background and foreground call */
var_dump($future->getResult(), $closure($test));
?>
