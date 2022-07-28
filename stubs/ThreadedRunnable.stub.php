<?php

/**
 * ThreadedRunnable class
 *
 * ThreadedRunnable represents a unit of work. It provides methods to determine its execution state.
 * @generate-class-entries
 */
abstract class ThreadedRunnable extends ThreadedBase
{
    /**
     * Tell if the referenced object is executing
     *
     * @link http://www.php.net/manual/en/threaded.isrunning.php
     * @return bool A boolean indication of state
     */
    public function isRunning() : bool{}

    /**
     * Tell if the referenced object exited, suffered fatal errors, or threw uncaught exceptions during execution
     *
     * @link http://www.php.net/manual/en/threaded.isterminated.php
     * @return bool A boolean indication of state
     */
    public function isTerminated() : bool{}

    /**
     * The programmer should always implement the run method for objects that are intended for execution.
     *
     * @link http://www.php.net/manual/en/threaded.run.php
     * @return void The methods return value, if used, will be ignored
     */
    abstract public function run() : void{}
}
