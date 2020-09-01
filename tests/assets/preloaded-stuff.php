<?php

class SomeClass{
	public static $var = 1;

	public function inheritedFunc() : void{
		var_dump("ok");
	}
}

function iAmPreloaded() : void{
	var_dump("yes");
}
