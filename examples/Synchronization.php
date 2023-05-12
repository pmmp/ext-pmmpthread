<?php

use pmmp\thread\Thread;

/*
	Rules:
		1. only ever wait FOR something
		2. only ever wait FOR something
		3. only ever wait FOR something

	Only one thread may enter synchronized() on a ThreadSafe object at any given time, unless the synchronized closure calls wait().
	Other threads which try to use synchronized() at the same time will block until either:
		a) the currently-executing synchronized callable calls wait() on the ThreadSafe object
		b) the currently-executing synchronized callable returns.
	
	Threads are not guaranteed to execute in any particular order; therefore, the following code could execute in one of two ways:
		a) the synchronized callable inside the Thread executes first, causing the main thread to never call wait(), and exit gracefully
		b) the synchronized callable on the main thread executes first, calls wait(), allowing the child thread to wake it up using notify(), and exit gracefully.
*/

class BadCode extends Thread{
	public function run() : void{
		/* The following is BAD CODE
		 * The main thread might check `awake` here, and see that it's false: */
		$this->awake = true;
		$this->notify();
		/* and then call wait() here, after the notify() has already been called
		 * this leads to wait() on the main thread getting stuck forever (deadlock).
		 * To avoid this, we use synchronized() as shown below.
		 */
	}
}
class GoodCode extends Thread{
	public function run() : void{
		/* The following is GOOD CODE
		 * One of two things might happen:
		 *     1) this synchronized block executes first, awake becomes true, and the main thread will never go to sleep,
		 *        because it can't check `awake` until this synchronized block exits.
		 *     2) the main thread's synchronized block executes first, blocking this one until the main thread calls 
		 *        wait(), guaranteeing that the notify() call in here will wake it up.
		 * This prevents any possible deadlock.
		 */
		$this->synchronized(function(){
			$this->awake = true;
			$this->notify();
		});
	}
}
$thread = new GoodCode();

$thread->start();
/*
 * wait() can ONLY be reliably used inside a synchronized block on the SAME ThreadSafe object that you're synchronizing with.
 * The behaviour of wait() is undefined if used outside a synchronized() block on the same object that you're wait()ing on.
 * (This means it might or might not do what you expect).
 *
 * This block might do one of two things:
 *    1) it executes first, sees `awake` == false, and calls wait(), allowing the other thread to notify it
 *    2) it executes second, sees `awake` == true, and returns without waiting
 */
$thread->synchronized(function() use($thread) {
	while (!$thread->awake) {
		/*
			If there was no precondition above and the Thread
			managed to send notification before we entered this synchronized block
			we would wait forever!

			We check the precondition in a loop because a Thread can be awoken
			by signals other than the one you are waiting for.

			If we didn't use a synchronized block, it's possible that the other thread's code could **all** execute
			**in between** the previous line and the following wait() call, which would lead to a deadlock
			(see BadCode::run() comments).
		*/
		$thread->wait();
	}
});
$thread->join();
?>
