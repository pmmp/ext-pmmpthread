<?php

/**
 * Worker
 *
 * Worker Threads have a persistent context, as such should be used over Threads in most cases.
 *
 * When a Worker is started, the run method will be executed, but the Thread will not leave until one
 * of the following conditions are met:
 *   - the Worker goes out of scope (no more references remain)
 *   - the programmer calls shutdown
 *   - the script dies
 * This means the programmer can reuse the context throughout execution; placing objects on the stack of
 * the Worker will cause the Worker to execute the stacked objects run method.
 *
 * @link http://www.php.net/manual/en/class.worker.php
 * @generate-class-entries
 */
class Worker extends Thread
{
    /**
     * Executes the optional collector on each of the tasks, removing the task if true is returned
     *
     * @param callable $function The collector to be executed upon each task
     * @return int The number of tasks left to be collected
     */
    public function collect(callable $function = null) : int{}

    /**
     * Default collection function called by collect(), if a collect callback wasn't given.
     *
     * @param ThreadedRunnable $collectable The collectable object to run the collector on
     * @return bool Whether or not the object can be disposed of
     */
    public function collector(ThreadedRunnable $collectable) : bool{}

    /**
     * Returns the number of threaded tasks waiting to be executed by the referenced Worker
     *
     * @link http://www.php.net/manual/en/worker.getstacked.php
     * @return int An integral value
     */
    public function getStacked() : int{}

    /**
     * Tell if the referenced Worker has been shutdown
     *
     * @link http://www.php.net/manual/en/worker.isshutdown.php
     * @return bool A boolean indication of state
	 * @alias Thread::isJoined
     */
    public function isShutdown() : bool{}

    /**
     * Shuts down the Worker after executing all the threaded tasks previously stacked
     *
     * @link http://www.php.net/manual/en/worker.shutdown.php
     * @return bool A boolean indication of success
	 * @alias Thread::join
     */
    public function shutdown() : bool{}

    /**
     * Appends the referenced object to the stack of the referenced Worker
     *
     * @param ThreadedRunnable $work Threaded object to be executed by the referenced Worker
     *
     * @link http://www.php.net/manual/en/worker.stack.php
     * @return int The new length of the stack
     */
    public function stack(ThreadedRunnable $work) : int{}

    /**
     * Removes the first task (the oldest one) in the stack.
     *
     * @link http://www.php.net/manual/en/worker.unstack.php
     * @return ThreadedRunnable|null The item removed from the stack
     */
    public function unstack() : ?ThreadedRunnable{}

    /**
     * Performs initialization actions when the Worker is started.
     * Override this to do actions on Worker start; an empty default implementation is provided.
     *
     * @return void
     */
    public function run() : void{}
}
