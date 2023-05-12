<?php

/**
 * @generate-class-entries
 */

namespace pmmp\thread;

/**
 * Thrown on failure to establish a connection to a ThreadSafe object from another thread.
 *
 * Since thread-safe objects don't directly track internal references (for simplicity reasons),
 * this means it's possible for an object to be destroyed before it's able to be dereferenced
 * by another thread. This is rare, and only usually happens when the chain of ownership of an
 * object is broken. See gone.phpt for an example case.
 */
class ConnectionException extends \RuntimeException{

}
