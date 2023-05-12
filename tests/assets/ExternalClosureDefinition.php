<?php

    class ExternalClosureDefinition extends \pmmp\thread\ThreadSafe {
        public function load() {
            $sync = function () {
                var_dump('Hello World');
            };
            $sync();
        }
    }
