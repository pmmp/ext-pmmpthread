<?php

class ExternalClosureDefinitionChildToParent {
	public static function getClosure() {
		$func = function() {
			var_dump(static::class, self::class);
		};
		return $func;
	}
}
