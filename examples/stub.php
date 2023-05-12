<?php
/**
 * pthreads extension stub file for code completion purposes
 *
 * @author Lisachenko Alexander <lisachenko.it@gmail.com>
 * @version 3.0.0
 */

namespace {
/**
 * The default inheritance mask used when starting Threads and Workers
 */
define('PTHREADS_INHERIT_ALL', 0x111111);

/**
 * Nothing will be inherited by the new context
 */
define('PTHREADS_INHERIT_NONE', 0);

/**
 * Determines whether the ini entries are inherited by the new context
 */
define('PTHREADS_INHERIT_INI', 0x1);

/**
 * Determines whether the constants are inherited by the new context
 */
define('PTHREADS_INHERIT_CONSTANTS', 0x10);

/**
 * Determines whether the class table is inherited by the new context
 */
define('PTHREADS_INHERIT_CLASSES', 0x100);

/**
 * Determines whether the function table is inherited by the new context
 */
define('PTHREADS_INHERIT_FUNCTIONS', 0x100);

/**
 * Determines whether the included_files table is inherited by the new context
 */
define('PTHREADS_INHERIT_INCLUDES', 0x10000);

/**
 * Determines whether the comments are inherited by the new context
 */
define('PTHREADS_INHERIT_COMMENTS', 0x100000);

/**
 * Allow output headers from the threads
 */
define('PTHREADS_ALLOW_HEADERS', 0x1000000);

}

namespace pmmp\thread{

/**
 * ThreadSafe class
 *
 * ThreadSafe exposes similar synchronization functionality to the old Threaded, but
 * with a less bloated interface which reduces undefined behaviour possibilities.
 *
 * ThreadSafe objects form the basis of pthreads ability to execute user code in parallel;
 * they expose and include synchronization methods.
 *
 * ThreadSafe objects, most importantly, provide implicit safety for the programmer;
 * all operations on the object scope are safe.
 *
 * @since 6.0.0
 */
class ThreadSafe implements IteratorAggregate
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

	public function getIterator() : Iterator{}
}

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

/**
 * ThreadSafeArray objects are similar to regular arrays, with the exception that they can be shared between threads.
 *
 * @since 6.0.0
 */
final class ThreadSafeArray extends ThreadSafe implements Countable, ArrayAccess
{
    /**
     * Fetches a chunk of the objects properties table of the given size
     *
     * @param int $size The number of items to fetch
     * @param bool $preserve Preserve the keys of members
     *
     * @return array An array of items from the objects member table
     */
    public function chunk(int $size, bool $preserve = false) : array{}

    /**
     * {@inheritdoc}
     */
    public function count() : int{}

    /**
     * Converts the given array into a ThreadSafeArray object (recursively)
     * @param array $array
     *
     * @return ThreadSafeArray A ThreadSafeArray object created from the provided array
     */
    public static function fromArray(array $array) : ThreadSafeArray {}

    /**
     * Merges data into the current ThreadSafeArray
     *
     * @param mixed $from The data to merge
     * @param bool $overwrite Overwrite existing keys flag
     *
     * @return bool A boolean indication of success
     */
    public function merge(mixed $from, bool $overwrite = true) : bool{}

    /**
     * Pops an item from the array
     *
     * @return mixed The last item in the array
     */
    public function pop() : mixed{}

    /**
     * Shifts an item from the array
     *
     * @return mixed The first item in the array
     */
    public function shift() : mixed{}

	public function offsetGet(mixed $offset) : mixed{}

	public function offsetSet(mixed $offset, mixed $value) : void{}

	public function offsetExists(mixed $offset) : bool{}

	public function offsetUnset(mixed $offset) : void{}
}

/**
 * Basic thread implementation
 *
 * An implementation of a Thread should extend this declaration, implementing the run method.
 * When the start method of that object is called, the run method code will be executed in separate Thread.
 */
abstract class Thread extends Runnable
{
    /**
     * Will return the identity of the Thread that created the referenced Thread
     *
     * @return int A numeric identity
     */
    public function getCreatorId() : int{}

    /**
     * Will return the instance of currently executing thread
     *
     * @return Thread|null
     */
    public static function getCurrentThread() : ?Thread{}

    /**
     * Will return the identity of the currently executing thread
     *
     * @return int
     */
    public static function getCurrentThreadId() : int{}

    /**
     * Will return the identity of the referenced Thread
     *
     * @return int
     */
    public function getThreadId() : int{}

    /**
     * Tell if the referenced Thread has been joined by another context
     *
     * @return bool A boolean indication of state
     */
    public function isJoined() : bool{}

    /**
     * Tell if the referenced Thread has been started
     *
     * @return bool A boolean indication of state
     */
    public function isStarted() : bool{}

    /**
     * Causes the calling context to wait for the referenced Thread to finish executing
     *
     * @return bool A boolean indication of state
     */
    public function join() : bool{}

    /**
     * Will start a new Thread to execute the implemented run method
     *
     * @param int $options An optional mask of inheritance constants, by default PTHREADS_INHERIT_ALL
     *
     * @return bool A boolean indication of success
     */
    public function start(int $options = PTHREADS_INHERIT_ALL) : bool{}
}

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
     * @param Runnable $collectable The collectable object to run the collector on
     * @return bool Whether or not the object can be disposed of
     */
    public function collector(Runnable $collectable) : bool{}

    /**
     * Returns the number of tasks waiting to be executed by the referenced Worker
     *
     * @return int An integral value
     */
    public function getStacked() : int{}

    /**
     * Tell if the referenced Worker has been shutdown
     *
     * @return bool A boolean indication of state
	 * @alias Thread::isJoined
     */
    public function isShutdown() : bool{}

    /**
     * Shuts down the Worker after executing all tasks previously stacked
     *
     * @return bool A boolean indication of success
	 * @alias Thread::join
     */
    public function shutdown() : bool{}

    /**
     * Appends the referenced object to the stack of the referenced Worker
     *
     * @param Runnable $work object to be executed by the referenced Worker
     *
     * @return int The new length of the stack
     */
    public function stack(Runnable $work) : int{}

    /**
     * Removes the first task (the oldest one) in the stack.
     *
     * @return Runnable|null The item removed from the stack
     */
    public function unstack() : ?Runnable{}

    /**
     * Performs initialization actions when the Worker is started.
     * Override this to do actions on Worker start; an empty default implementation is provided.
     *
     * @return void
     */
    public function run() : void{}
}


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
