#include <Python.h>

#ifndef Py_NewRef
static inline PyObject *Py_NewRef(PyObject *o) {
    Py_INCREF(o);
    return o;
}
#endif

PyDoc_STRVAR(atomicint__doc__,
"A simple implementation of atomic integers");

typedef struct {
    PyObject_HEAD
    PyLongObject *value;
} PyAtomicInt;

static void atomicint_dealloc(PyObject *o) {
    PyAtomicInt *self = (PyAtomicInt*)o;
    Py_XDECREF(self->value);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyTypeObject AtomicInt_type;

static int atomicint_init(PyObject *o, PyObject *args, PyObject *kwds) {
    assert(Py_IS_TYPE(o, &AtomicInt_type));
    PyAtomicInt *self = (PyAtomicInt*)o;
    self->value = NULL;

    if (!_PyArg_NoKeywords("AtomicInt", kwds)) {
        goto out;
    }

    PyObject *value;
    if (PyTuple_GET_SIZE(args) == 0) {
        value = PyLong_FromLong(0);
        if (!value) {
            return -1;
        }
    } else if (PyTuple_GET_SIZE(args) == 1) {
        value = PyTuple_GET_ITEM(args, 0);
        if (!PyLong_CheckExact(value)) {
            PyErr_SetString(PyExc_TypeError, "AtomicInt() takes an integer");
            return -1;
        }
        Py_INCREF(value);
        // okay to reuse the same object, since ints are immutable.
    } else {
        PyErr_SetString(PyExc_TypeError, "AtomicInt() takes 0 or 1 positional arguments");
        return -1;
    }

    assert(PyLong_CheckExact(value));
    self->value = (PyLongObject*)value;

    return 0;

out:
    return -1;
}

static PyObject *atomicint_int(PyObject *v) {
    assert(Py_IS_TYPE(v, &AtomicInt_type));
    return Py_NewRef(_PyObject_CAST(((PyAtomicInt*)v)->value));
}

static PyObject *atomicint_inplace_add(PyObject *o1, PyObject *o2) {
    assert(Py_IS_TYPE(o1, &AtomicInt_type));
    PyAtomicInt *v = (PyAtomicInt*)o1;

    PyObject *result = PyNumber_Add(_PyObject_CAST(v->value), o2);
    if (!result) {
        return NULL;
    }

    Py_SETREF(v->value, (PyLongObject*)Py_NewRef(result));
    return Py_NewRef(_PyObject_CAST(v));
}

static PyNumberMethods atomicint_as_number = {
    .nb_int = atomicint_int,
    .nb_inplace_add = atomicint_inplace_add,
};

static PyTypeObject AtomicInt_type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "atomicint.AtomicInt",
    sizeof(PyAtomicInt),
    .tp_dealloc = atomicint_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_as_number = &atomicint_as_number,
    .tp_init = atomicint_init,
    .tp_new = PyType_GenericNew,
};

static int atomicint_exec(PyObject* m) {
    if (PyType_Ready(&AtomicInt_type) < 0) {
        return -1;
    }

    Py_INCREF(&AtomicInt_type);
    if (PyModule_AddObject(m, "AtomicInt", (PyObject*)&AtomicInt_type) < 0) {
        return -1;
    }

    return 0;
}

static struct PyModuleDef_Slot atomicint_slots[] = {
    {Py_mod_exec, atomicint_exec},
    {0, NULL},
};

static struct PyModuleDef atomicint_module = {
    PyModuleDef_HEAD_INIT,
    "atomicint",
    atomicint__doc__,
    0,
    NULL,
    atomicint_slots,
    NULL,
    NULL,
    NULL,
};

PyMODINIT_FUNC PyInit_atomicint(void) {
    return PyModuleDef_Init(&atomicint_module);
}
