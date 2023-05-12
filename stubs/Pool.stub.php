<?php

/**
 * @generate-class-entries
 */

namespace pmmp\thread;

/**
 * Pool class
 *
 * A Pool is a container for, and controller of, a number of Worker threads, the number of threads can be adjusted
 * during execution, additionally the Pool provides an easy mechanism to maintain and collect references in the
 * proper way.
 */
class Pool{
    /**
     * The maximum number of Worker threads allowed in this Pool
     *
     * @var int
     */
    protected $size;

    /**
     * The name of the Worker class for this Pool
     *
     * @var string
     */
    protected $class;

    /**
     * The array of Worker threads for this Pool
     *
     * @var Worker[]
     */
    protected $workers;

    /**
     * The constructor arguments to be passed by this Pool to new Workers upon construction
     *
     * @var array
     */
    protected $ctor;

    /**
     * The numeric identifier for the last Worker used by this Pool
     *
     * @var int
     */
    protected $last = 0;

    /**
     * Construct a new Pool of Workers
     *
     * @param integer $size The maximum number of Workers this Pool can create
     * @param string $class The class for new Workers
     * @param array $ctor An array of arguments to be passed to new Workers
     */
    public function __construct(int $size, string $class = Worker::class, array $ctor = []) {}

    /**
     * Collect references to completed tasks
     *
     * Allows the Pool to collect references determined to be garbage by the given collector
     *
     * @param callable|null $collector
     * @return int the number of tasks collected from the pool
     */
    public function collect(callable $collector = null) : int{}

    /**
     * Resize the Pool
     *
     * @param integer $size The maximum number of Workers this Pool can create
     */
    public function resize(int $size) : void{}

    /**
     * Shutdown all Workers in this Pool
     */
    public function shutdown() : void{}

    /**
     * Submit the task to the next Worker in the Pool
     *
     * @param Runnable $task The task for execution
     *
     * @return int the identifier of the Worker executing the object
     */
    public function submit(Runnable $task) : int{}

    /**
     * Submit the task to the specific Worker in the Pool
     *
     * @param int $worker The worker for execution
     * @param Runnable $task The task for execution
     *
     * @return int the identifier of the Worker that accepted the object
     */
    public function submitTo(int $worker, Runnable $task) : int{}
}
