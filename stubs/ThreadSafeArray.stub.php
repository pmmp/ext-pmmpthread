<?php

/**
 * @generate-class-entries
 */

namespace pmmp\thread;

/**
 * ThreadSafeArray objects are similar to regular arrays, with the exception that they can be shared between threads.
 *
 * @since 6.0.0
 *
 * @strict-properties
 */
final class ThreadSafeArray extends ThreadSafe implements \Countable, \ArrayAccess
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
     * @throws NonThreadSafeValueError if the array contains any non-thread-safe values
     */
    public static function fromArray(array $array) : ThreadSafeArray{}

    /**
     * Merges data into the current ThreadSafeArray
     *
     * @param mixed $from The data to merge
     * @param bool $overwrite Overwrite existing keys flag
     *
     * @return bool A boolean indication of success
     * @throws NonThreadSafeValueError if $from contains any non-thread-safe values
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
