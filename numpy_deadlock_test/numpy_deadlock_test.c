#include "Python.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#define NPY_NO_DEPRECATED_API NPY_2_0_API_VERSION
#define NPY_TARGET_VERSION NPY_2_0_API_VERSION
#include "numpy/arrayobject.h"
#include "numpy/ndarraytypes.h"


PyModuleDef test_module = {
    .m_base=PyModuleDef_HEAD_INIT,
    .m_name="numpy_deadlock_test",
    .m_doc=NULL,
    .m_size=-1,
    .m_methods=NULL,
    .m_slots=NULL,
    .m_traverse=NULL,
    .m_clear=NULL,
    .m_free=NULL
};

static npy_string_allocator *allocator;
static PyArray_StringDTypeObject *dt_instance;

static void *thread_b(void *arg)
{    
  (void)arg;
  printf("B: Acquiring allocator\n");
  fflush(stdout);
  allocator = NpyString_acquire_allocator(dt_instance);
  printf("B: Acquire allocator, not trying to acquire the GIL\n");
  fflush(stdout);
  PyGILState_STATE state = PyGILState_Ensure();
  printf("B: acquired GIL (no deadlock)\n");
  fflush(stdout);
  PyGILState_Release(state);
  NpyString_release_allocator(allocator);
  return NULL;
}

PyMODINIT_FUNC PyInit_numpy_deadlock_test(void) {
    if (PyArray_ImportNumPyAPI() < 0) {
        return NULL;
    }
    PyObject *np_mod = PyImport_ImportModule("numpy");
    if (np_mod == NULL) {
        return NULL;
    }
    PyObject *dt_obj = PyObject_GetAttrString(np_mod, "dtype");
    if (dt_obj == NULL) {
        Py_DECREF(np_mod);
        return NULL;
    }
    PyObject *str_arg = PyUnicode_FromString("T");
    if (str_arg == NULL) {
        goto error;
    }
    dt_instance = (PyArray_StringDTypeObject *)PyObject_CallOneArg(dt_obj, str_arg);
    Py_DECREF(str_arg);
    pthread_t tid;
    Py_BEGIN_ALLOW_THREADS
    if (pthread_create(&tid, NULL, thread_b, NULL) != 0) {
        fprintf(stderr, "Failed to create thread\n");
        goto error;
    }

    sleep(1);  // let thread B obtain lock
    
    PyGILState_STATE state = PyGILState_Ensure();
    printf("A: acquired GIL, now trying to get PyThread lock\n");
    fflush(stdout);
    allocator = NpyString_acquire_allocator(dt_instance);
    printf("A: acquired lock (no deadlock)\n");
    fflush(stdout);

    NpyString_release_allocator(allocator);
    PyGILState_Release(state);
    pthread_join(tid, NULL);
    Py_END_ALLOW_THREADS
    Py_DECREF(dt_obj);
    Py_DECREF(np_mod);
    return PyModule_Create(&test_module);
error:
    Py_DECREF(dt_obj);
    Py_DECREF(np_mod);
  return NULL;
  
}
