<?php

/**
 * @generate-class-entries
 */

namespace pmmp\thread;

/**
 * ThreadSafe class
 *
 * ThreadSafe exposes similar synchronization functionality to the old Threaded, but
 * with a less bloated interface which reduces undefined behaviour possibilities.
 *
 * ThreadSafe objects expose synchronization methods and wait/notify functionality.
 *
 * ThreadSafe objects, most importantly, provide implicit safety for the programmer;
 * all operations on the object scope are safe.
 *
 * @since 6.0.0
 */
class ThreadSafe implements \IteratorAggregate
{
    /**
     * Send notification to the referenced object
     *
     * @return bool A boolean indication of success
     */
    public function notify() : bool{}

    /**
     * Send notification to one context waiting on the ThreadSafe
     *
     * @return bool A boolean indication of success
     */
    public function notifyOne() : bool{}

    /**
     * Executes the block while retaining the synchronization lock for the current context.
     *
     * @param \Closure $function The block of code to execute
     * @param mixed $args... Variable length list of arguments to use as function arguments to the block
     *
     * @return mixed The return value from the block
     */
    public function synchronized(\Closure $function, mixed ...$args) : mixed{}

    /**
     * Waits for notification from the Stackable
     *
     * @param int $timeout An optional timeout in microseconds
     *
     * @return bool A boolean indication of success
     */
    public function wait(int $timeout = 0) : bool{}

	public function getIterator() : \Iterator{}
}
