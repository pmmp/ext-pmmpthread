<?php

/**
 * ThreadedArray objects are similar to regular arrays, with the exception that they can be shared between threads.
 *
 * @generate-class-entries
 * @strict-properties
 */
final class ThreadedArray extends ThreadedBase implements Traversable, Countable
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
     * Converts the given array into a ThreadedArray object (recursively)
     * @param array $array
     *
     * @return ThreadedArray A ThreadedArray object created from the provided array
     */
    public static function fromArray(array $array) : ThreadedArray {}

    /**
     * Merges data into the current ThreadedArray
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
}
