<?php

    class ExternalClosureDefinition extends \ThreadedBase {
        public function load() {
            $sync = function () {
                var_dump('Hello World');
            };
            $sync();
        }
    }
