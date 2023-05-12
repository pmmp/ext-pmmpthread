<?php

/**
 * @generate-class-entries
 */

namespace pmmp\thread;

/**
 * Runnable class
 *
 * Runnable represents a unit of work. It provides methods to determine its execution state.
 *
 * @since 6.0.0
 */
abstract class Runnable extends ThreadSafe
{
    /**
     * Tell if the referenced object is executing
     *
     * @return bool A boolean indication of state
     */
    public function isRunning() : bool{}

    /**
     * Tell if the referenced object exited, suffered fatal errors, or threw uncaught exceptions during execution
     *
     * @return bool A boolean indication of state
     */
    public function isTerminated() : bool{}

    /**
     * The programmer should always implement the run method for objects that are intended for execution.
     *
     * @return void The methods return value, if used, will be ignored
     */
    abstract public function run() : void;
}
