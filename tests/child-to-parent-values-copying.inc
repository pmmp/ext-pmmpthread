<?php

function assignStuff(Test $test) : void{
	$test->permanentInternedString = \Thread::class; //this probably isn't actually PERMANENT, but it should at least be PERSISTENT
	$test->requestInternedString = "i am a request-local string"; //only when OPcache disabled
	$test->refcountedString = str_repeat("hello", 3);

	$test->emptyArray = []; //zend_empty_array should be OK to share directly
	$test->constArray = [1, 2, 3]; //const array, request-local (unless OPcache is enabled)
	$test->refcountedArray = range(1, 3);

}
