
#include <src/globals.h>
#include <src/pthreads.h>

#if HAVE_PTHREADS_EXT_SOCKETS_SUPPORT

static zend_object* (*socket_create_object_original)(zend_class_entry *class_type);

static void pthreads_socket_free_obj_hook(zend_object *object) {
	php_socket *socket = socket_from_obj(object);
	if (!IS_INVALID_SOCKET(socket)) {
		if (pthreads_globals_socket_shared(socket->bsd_socket)) {
			socket->bsd_socket = -1;
		}
	}
	PTHREADS_ZG(original_socket_object_handlers)->free_obj(object);
}

static zend_object *pthreads_socket_create_object_hook(zend_class_entry *class_type) {
	zend_object *result = socket_create_object_original(class_type);
	if (PTHREADS_ZG(original_socket_object_handlers) == NULL) {
		PTHREADS_ZG(original_socket_object_handlers) = result->handlers;
		PTHREADS_ZG(custom_socket_object_handlers) = *result->handlers;
		PTHREADS_ZG(custom_socket_object_handlers).free_obj = pthreads_socket_free_obj_hook;
	}
	result->handlers = &PTHREADS_ZG(custom_socket_object_handlers);
	return result;
}

void pthreads_ext_sockets_hacks_init() {
	socket_create_object_original = socket_ce->create_object;
	socket_ce->create_object = pthreads_socket_create_object_hook;
}

#endif
