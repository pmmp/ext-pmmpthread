
#include <src/globals.h>
#include <src/pmmpthread.h>

#if HAVE_PMMPTHREAD_EXT_SOCKETS_SUPPORT

static zend_object* (*socket_create_object_original)(zend_class_entry *class_type);

static void pmmpthread_socket_free_obj_hook(zend_object *object) {
	php_socket *socket = socket_from_obj(object);
	if (!IS_INVALID_SOCKET(socket)) {
		if (pmmpthread_globals_socket_shared(socket->bsd_socket)) {
			socket->bsd_socket = -1;
		}
	}
	PMMPTHREAD_ZG(original_socket_object_handlers)->free_obj(object);
}

static zend_object *pmmpthread_socket_create_object_hook(zend_class_entry *class_type) {
	zend_object *result = socket_create_object_original(class_type);
	if (PMMPTHREAD_ZG(original_socket_object_handlers) == NULL) {
		PMMPTHREAD_ZG(original_socket_object_handlers) = result->handlers;
		PMMPTHREAD_ZG(custom_socket_object_handlers) = *result->handlers;
		PMMPTHREAD_ZG(custom_socket_object_handlers).free_obj = pmmpthread_socket_free_obj_hook;
	}
	result->handlers = &PMMPTHREAD_ZG(custom_socket_object_handlers);
	return result;
}

void pmmpthread_ext_sockets_hacks_init() {
	socket_create_object_original = socket_ce->create_object;
	socket_ce->create_object = pmmpthread_socket_create_object_hook;
}

#endif
