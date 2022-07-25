--TEST--
Test attributes copying
--FILE--
<?php

class ClassAttribute{

	public function __construct(public int $param1, public string $param2){}
}

const TARGETS = [
	\Attribute::TARGET_CLASS => "class",
	\Attribute::TARGET_FUNCTION => "function",
	\Attribute::TARGET_METHOD => "method",
	\Attribute::TARGET_PROPERTY => "property",
	\Attribute::TARGET_CLASS_CONSTANT => "class constant",
	\Attribute::TARGET_PARAMETER => "parameter",
];
function dumpAttribute(\ReflectionAttribute $attribute) : void{
	echo "--- Attribute " . "\"" . $attribute->getName() . "\" ---\n";
	$targets = [];
	foreach(TARGETS as $flag => $name){
		if(($attribute->getTarget() & $flag) !== 0){
			$targets[] = $name;
		}
	}
	echo "\tTarget=" . implode(", ", $targets) . "\n";
	echo "\tRepeated=" . ($attribute->isRepeated() ? "yes" : "no") . "\n";
	echo "\tArguments=" . print_r($attribute->getArguments(), true) . "\n";
	echo "--- End of attribute ---\n";
}

function dumpAttributes(array $attributes) : void{
	foreach($attributes as $attribute){
		dumpAttribute($attribute);
	}
}

#[ClassAttribute(1, "hello"), ClassAttribute(2, "world"), NotRepeated]
class Test{
	#[AttributedConst]
	public const ATTRIBUTED_CONST = 1;

	public const NON_ATTRIBUTED_CONST = 2;

	#[AttributedField]
	public $attributedField = 1;

	public $nonAttributedField = 2;

	#[AttributedStaticField]
	public static $staticField = 1;

	public static $nonAttributedStaticField = 2;

	#[MethodAttribute1, MethodAttribute2("ho ho ho")]
	public function t1(#[ParameterAttribute("i", "don't", "know")] int $p1) : void{}

	public function t2(int $p1) : void{}

	#[StaticMethodAttribute]
	public static function staticMethod() : void{}

	public static function nonAttributedStaticMethod() : void{}
}

function test() : void{
	$c = new \ReflectionClass(Test::class);
	dumpAttributes($c->getAttributes());
	foreach($c->getReflectionConstants() as $const){
		dumpAttributes($const->getAttributes());
	}
	foreach($c->getProperties() as $property){
		dumpAttributes($property->getAttributes());
	}
	foreach($c->getMethods() as $method){
		dumpAttributes($method->getAttributes());	
		foreach($method->getParameters() as $parameter){
			dumpAttributes($parameter->getAttributes());
		}
	}

}

echo "--- main thread start ---\n";
test();
echo "--- main thread end ---\n";

$thread = new class extends \Thread{
	public function run() : void{
		echo "--- child thread start ---\n";
		test();
		echo "--- child thread end ---\n";
	}
};
$thread->start();
$thread->join();
echo "OK\n";
?>
--EXPECTF--
--- main thread start ---
--- Attribute "ClassAttribute" ---
	Target=class
	Repeated=yes
	Arguments=Array
(
    [0] => 1
    [1] => hello
)

--- End of attribute ---
--- Attribute "ClassAttribute" ---
	Target=class
	Repeated=yes
	Arguments=Array
(
    [0] => 2
    [1] => world
)

--- End of attribute ---
--- Attribute "NotRepeated" ---
	Target=class
	Repeated=no
	Arguments=Array
(
)

--- End of attribute ---
--- Attribute "AttributedConst" ---
	Target=class constant
	Repeated=no
	Arguments=Array
(
)

--- End of attribute ---
--- Attribute "AttributedField" ---
	Target=property
	Repeated=no
	Arguments=Array
(
)

--- End of attribute ---
--- Attribute "AttributedStaticField" ---
	Target=property
	Repeated=no
	Arguments=Array
(
)

--- End of attribute ---
--- Attribute "MethodAttribute1" ---
	Target=method
	Repeated=no
	Arguments=Array
(
)

--- End of attribute ---
--- Attribute "MethodAttribute2" ---
	Target=method
	Repeated=no
	Arguments=Array
(
    [0] => ho ho ho
)

--- End of attribute ---
--- Attribute "ParameterAttribute" ---
	Target=parameter
	Repeated=no
	Arguments=Array
(
    [0] => i
    [1] => don't
    [2] => know
)

--- End of attribute ---
--- Attribute "StaticMethodAttribute" ---
	Target=method
	Repeated=no
	Arguments=Array
(
)

--- End of attribute ---
--- main thread end ---
--- child thread start ---
--- Attribute "ClassAttribute" ---
	Target=class
	Repeated=yes
	Arguments=Array
(
    [0] => 1
    [1] => hello
)

--- End of attribute ---
--- Attribute "ClassAttribute" ---
	Target=class
	Repeated=yes
	Arguments=Array
(
    [0] => 2
    [1] => world
)

--- End of attribute ---
--- Attribute "NotRepeated" ---
	Target=class
	Repeated=no
	Arguments=Array
(
)

--- End of attribute ---
--- Attribute "AttributedConst" ---
	Target=class constant
	Repeated=no
	Arguments=Array
(
)

--- End of attribute ---
--- Attribute "AttributedField" ---
	Target=property
	Repeated=no
	Arguments=Array
(
)

--- End of attribute ---
--- Attribute "AttributedStaticField" ---
	Target=property
	Repeated=no
	Arguments=Array
(
)

--- End of attribute ---
--- Attribute "MethodAttribute1" ---
	Target=method
	Repeated=no
	Arguments=Array
(
)

--- End of attribute ---
--- Attribute "MethodAttribute2" ---
	Target=method
	Repeated=no
	Arguments=Array
(
    [0] => ho ho ho
)

--- End of attribute ---
--- Attribute "ParameterAttribute" ---
	Target=parameter
	Repeated=no
	Arguments=Array
(
    [0] => i
    [1] => don't
    [2] => know
)

--- End of attribute ---
--- Attribute "StaticMethodAttribute" ---
	Target=method
	Repeated=no
	Arguments=Array
(
)

--- End of attribute ---
--- child thread end ---
OK
