<?php

/**
 * ThreadedBase class
 *
 * ThreadedBase exposes similar synchronization functionality to the old Threaded, but
 * with a less bloated interface which reduces undefined behaviour possibilities.
 *
 * Threaded objects form the basis of pthreads ability to execute user code in parallel;
 * they expose and include synchronization methods.
 *
 * Threaded objects, most importantly, provide implicit safety for the programmer;
 * all operations on the object scope are safe.
 * @generate-class-entries
 */
class ThreadedBase implements IteratorAggregate
{
    /**
     * Send notification to the referenced object
     *
     * @link http://www.php.net/manual/en/threaded.notify.php
     * @return bool A boolean indication of success
     */
    public function notify() : bool{}

    /**
     * Send notification to one context waiting on the Threaded
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
     * @link http://www.php.net/manual/en/threaded.synchronized.php
     * @return mixed The return value from the block
     */
    public function synchronized(\Closure $function, mixed ...$args) : mixed{}

    /**
     * Waits for notification from the Stackable
     *
     * @param int $timeout An optional timeout in microseconds
     *
     * @link http://www.php.net/manual/en/threaded.wait.php
     * @return bool A boolean indication of success
     */
    public function wait(int $timeout = 0) : bool{}

	public function getIterator() : Iterator{}
}
