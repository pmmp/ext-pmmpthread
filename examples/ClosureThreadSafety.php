<?php

use pmmp\thread\ThreadSafe;

require 'vendor/autoload.php';

/*
 * Closures are complicated beasts when it comes to threading. Some are safe to share, others are not.
 * This file demonstrates some of the things you can and cannot do with closures.
 *
 * All thread-safe closures should be one of the following:
 * - Declared outside any classes (no $this)
 * - Declared inside a ThreadSafe class ($this is a ThreadSafe object)
 * - Declared as static inside any class (no $this)
 *
 * Things NOT to do:
 * - Do not use() variables by-reference
 * - Do not use() any non-ThreadSafe objects, arrays or resources - (use()ing other Closures is OK if they also follow these rules)
 * - No local static variables
 *
 * Generally speaking, as long as your closure doesn't use any non-copyable non-ThreadSafe things (NTS objects,
 * references, arrays, resources, other non-thread-safe Closures) then it should work fine.
 *
 * If you try to set a non-thread-safe Closure to a ThreadSafe object's property, a NonThreadSafeValueError will be thrown.
 */

/**
 * This class showcases what types of closures can be used in a thread-safe manner from a non-thread-safe class.
 */
class NonThreadSafeClass{
	/**
	 * NOT THREAD SAFE
	 *
	 * This closure implicitly uses $this, and $this would be a non-ThreadSafe object (the current class does not extend
	 * ThreadSafe).
	 * Non-static closures always bind $this, regardless of whether it is needed or not. (The same is also true for
	 * arrow functions.)
	 */
	public function getDynamicClosure() : Closure{
		return function() : void{
			echo "Hello world!\n";
		};
	}

	/**
	 * THREAD SAFE
	 * 
	 * This closure is explicitly static, so it does not bind $this.
	 */
	public function getStaticClosure() : Closure{
		return static function() : void{
			echo "Hello world!\n";
		};
	}

	/**
	 * THREAD SAFE
	 * 
	 * This closure is declared inside a static method, so it's implicitly static and also does not bind $this.
	 */
	public static function getImplicitStaticClosure() : Closure{
		return function() : void{
			echo "Hello world!\n";
		};
	}

	/**
	 * THREAD SAFE
	 *
	 * This closure use()s variables by-value, but they are copyable, so it's OK.
	 */
	public static function getClosureWithUseVars() : Closure{
		$i = 1;
		return static function() use ($i) : void{
			$i++;
			var_dump($i);
		};
	}
}

class ThreadSafeClass extends ThreadSafe{
	/**
	 * THREAD SAFE
	 * 
	 * This closure binds $this, but it's OK in this context because $this would be an instanceof ThreadSafe.
	 */
	public function getDynamicClosure() : Closure{
		return function() : void{
			echo "Hello world!\n";
		};
	}

	/**
	 * THREAD SAFE
	 * 
	 * This closure is explicitly static, so it does not bind $this.
	 */
	public function getStaticClosure() : Closure{
		return static function() : void{
			echo "Hello world!\n";
		};
	}

	/**
	 * THREAD SAFE
	 * 
	 * This closure is declared inside a static method, so it's implicitly static and also does not bind $this.
	 */
	public static function getImplicitStaticClosure() : Closure{
		return function() : void{
			echo "Hello world!\n";
		};
	}

	/**
	 * THREAD SAFE
	 * 
	 * This closure use()s variables by-value, but they are copyable, so it's OK.
	 */
	public static function getClosureWithUseVars() : Closure{
		$i = 1;
		return static function() use ($i) : void{
			$i++;
			var_dump($i);
		};
	}

}

/**
 * The closures shown here are never thread-safe, regardless of the context in which they are declared.
 */
class NeverThreadSafe{
	/**
	 * NEVER THREAD SAFE
	 *
	 * Local static variables are not thread-safe, because they cannot be copied.
	 */
	public static function getClosureWithLocalStaticVars() : Closure{
		return static function() : void{
			static $i = 1;
			$i++;
			var_dump($i);
		};
	}

	/**
	 * NEVER THREAD SAFE
	 * 
	 * This closure use()s variables by-reference, which is not allowed.
	 * It also use()s an \stdClass, which is not thread-safe and cannot be copied.
	 */
	public static function getClosureWithNonThreadSafeUse() : Closure{
		$object = new \stdClass(); //non-thread-safe object
		$i = 1;
		return static function() use ($object, &$i) : void{
			$i++;
			var_dump($i, $object);
		};
	}
}


