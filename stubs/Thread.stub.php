<?php

/**
 * @generate-class-entries
 */

namespace pmmp\thread;

/**
 * Basic thread implementation
 *
 * An implementation of a Thread should extend this declaration, implementing the run method.
 * When the start method of that object is called, the run method code will be executed in separate Thread.
 */
abstract class Thread extends Runnable
{
	/**
	 * @var int
	 * @cvalue PMMPTHREAD_INHERIT_NONE
	 *
	 * The newly created thread will inherit nothing from its parent, as if starting a new request.
	 */
	public const INHERIT_NONE = UNKNOWN;

	/**
	 * @var int
	 * @cvalue PMMPTHREAD_INHERIT_INI
	 *
	 * The newly created thread will inherit INI overrides set by ini_set() in the parent thread(s).
	 * If not set, the settings defined in php.ini will be used.
	 */
	public const INHERIT_INI = UNKNOWN;

	/**
	 * @var int
	 * @cvalue PMMPTHREAD_INHERIT_CONSTANTS
	 *
	 * The newly created thread will copy all global constants with simple or thread-safe values from its parent.
	 * Note: Constants containing non-thread-safe objects or resources cannot be copied.
	 *
	 * Do not rely on this for production. Prefer relying on autoloading instead (e.g. Composer
	 * bootstrap files), which is more reliable (and takes less memory, when OPcache is used).
	 */
	public const INHERIT_CONSTANTS = UNKNOWN;

	/**
	 * @var int
	 * @cvalue PMMPTHREAD_INHERIT_FUNCTIONS
	 *
	 * The newly created thread will copy all global functions from its parent.
	 *
	 * Do not rely on this for production. Prefer relying on autoloading instead (e.g. Composer
	 * bootstrap files), which is more reliable (and takes less memory, when OPcache is used).
	 */
	public const INHERIT_FUNCTIONS = UNKNOWN;

	/**
	 * @var int
	 * @cvalue PMMPTHREAD_INHERIT_CLASSES
	 *
	 * The newly created thread will copy all classes from its parent.
	 *
	 * !!!!! WARNING !!!!! This has a significant performance cost in large applications with many
	 * classes. Avoid relying on this. Prefer relying on autoloading instead (e.g. Composer
	 * bootstrap files), which is more reliable (and takes less memory, when OPcache is used).
	 *
	 * Note: Disabling this flag only prevents class copying during thread start. Classes may still
	 * be copied at other times, such as when a new thread is started, since no autoloader would be
	 * present inside the new thread to load the thread's own class.
	 */
	public const INHERIT_CLASSES = UNKNOWN;

	/**
	 * @var int
	 * @cvalue PMMPTHREAD_INHERIT_INCLUDES
	 *
	 * The newly created thread will copy the list of included and required files from its parent.
	 */
	public const INHERIT_INCLUDES = UNKNOWN;

	/**
	 * @var int
	 * @cvalue PMMPTHREAD_INHERIT_COMMENTS
	 *
	 * The newly created thread will copy doc comments of any classes, functions or constants that
	 * it inherits from its parent.
	 */
	public const INHERIT_COMMENTS = UNKNOWN;

	/**
	 * @var int
	 * @cvalue PMMPTHREAD_INHERIT_ALL
	 *
	 * Everything (classes, functions, constants, includes, doc comments, ini settings) will be
	 * copied from the parent thread.
	 *
	 * !!!!! WARNING !!!!! This has a significant performance cost in large applications with many
	 * classes. Avoid relying on this. Prefer relying on autoloading instead (e.g. Composer
	 * bootstrap files), which is more reliable (and takes less memory, when OPcache is used).
	 */
	public const INHERIT_ALL = UNKNOWN;

	/**
	 * @var int
	 * @cvalue PMMPTHREAD_ALLOW_HEADERS
	 *
	 * Allows the new thread to emit HTTP headers.
	 */
	public const ALLOW_HEADERS = UNKNOWN;

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
	 * Returns a ThreadSafeArray of globals accessible to all threads
	 * Any modification made will be seen by all threads
	 *
	 * @return ThreadSafeArray
	 */
	public static function getSharedGlobals() : ThreadSafeArray{}

	/**
	 * Returns the total number of Threads and Workers which have been
	 * started but not yet successfully joined/shutdown.
	 *
	 * The following are **not** included:
	 * - Threads which have been created but not started
	 * - Threads which have already been joined/shutdown
	 * - Threads which are not managed by pmmpthread (e.g. created by other extensions)
	 * - The main process thread
	 *
	 * @return int
	 */
	public static function getRunningCount() : int{}

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
     * @param int $options An optional mask of inheritance constants, by default INHERIT_ALL
     *
     * @return bool A boolean indication of success
     */
    public function start(int $options) : bool{}
}
