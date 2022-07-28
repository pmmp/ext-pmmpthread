<?php

function assignStuff(Test $test) : void{
	$test->permanentInternedString = \Thread::class; //this probably isn't actually PERMANENT, but it should at least be PERSISTENT
	$test->requestInternedString = "i am a request-local string"; //only when OPcache disabled
	$test->refcountedString = str_repeat("hello", 3);
}
