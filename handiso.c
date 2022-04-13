#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "src/hand_index.h"

static PyObject *HandisoError;

typedef struct {
	PyObject_HEAD
	hand_indexer_state_t state;
} StateObject;

typedef struct {
	PyObject_HEAD
	hand_indexer_t indexer;
} IndexerObject;

static int helper_parse_uint8s(PyObject* list, uint8_t* dst) {
	Py_ssize_t n = PyList_Size(list);
	int overflow = 0;
	for (Py_ssize_t i = 0; i < n; i++) {
		PyObject* item = PyList_GetItem(list, i);
		if (item == NULL) {
			return -1;
		}
		if (!PyLong_Check(item)) {
			return -1;
		}
		long value = PyLong_AsLongAndOverflow(item, &overflow);
		if (overflow || value < 1 || value > 255) {
			return -1;
		}
		dst[i] = (uint8_t)(value);
	}
	return n;
}

/**
 * Free a hand indexer state.
 */
static void py_hand_indexer_state_dealloc(IndexerObject *self) {
	Py_TYPE(self)->tp_free((PyObject *) self);
}

/**
 * Initialize a hand index state.  This is used for incrementally indexing a hand as
 * new rounds are dealt and determining if a hand is canonical.
 *
 * @param state
 */
static int py_hand_indexer_state_init(StateObject *self, PyObject *args, PyObject *kwds) {
	PyObject* pyindexer = NULL;
	if (!PyArg_ParseTuple(args, "O", &pyindexer)) {
		return -1;
	}
	IndexerObject* indexer = (IndexerObject*)(pyindexer);
	hand_indexer_state_init(&indexer->indexer, &self->state);
	return 0;
}


/**
 * Free a hand indexer.
 */
static void py_hand_indexer_dealloc(IndexerObject *self) {
	hand_indexer_free(&self->indexer);
	Py_TYPE(self)->tp_free((PyObject *) self);
}

/**
 * Initialize a hand indexer.  This generates a number of lookup tables and is relatively
 * expensive compared to indexing a hand.
 *
 * @param cards_per_round number of cards in each round
 */
static int py_hand_indexer_init(IndexerObject *self, PyObject *args, PyObject *kwds) {
	PyObject* cards = NULL;
	if (!PyArg_ParseTuple(args, "O", &cards)) {
		return -1;
	}
	if (!PyList_Check(cards)) {
		return -1;
	}
	uint8_t cards_per_round[MAX_ROUNDS];
	int rounds = helper_parse_uint8s(cards, cards_per_round);
	if (rounds < 1) {
		return -1;
	}
	if (!hand_indexer_init(rounds, cards_per_round, &self->indexer)) {
		return -1;
	}
	return 0;
}

/**
 * Get rounds of index
 *
 * @returns rounds of index
 */
static PyObject* py_hand_indexer_rounds(IndexerObject *self, PyObject *Py_UNUSED(ignored)) {
    return PyLong_FromLong(self->indexer.rounds);
}

/**
 * Get configurations at round
 *
 * @param round 
 * @returns configurations at round
 */
static PyObject* py_hand_indexer_configurations(IndexerObject *self, PyObject *args) {
	int round = 0;
	if (!PyArg_ParseTuple(args, "i", &round)) {
		PyErr_SetString(HandisoError, "Bad argument");
		return NULL;
	}
	if (round < 0 || (uint_fast32_t)(round) >= self->indexer.rounds) {
		return PyLong_FromLong(0);
	}
    return PyLong_FromLong(self->indexer.configurations[round]);
}

/**
 * Get permutations at round
 *
 * @param round 
 * @returns permutations at round
 */
static PyObject* py_hand_indexer_permutations(IndexerObject *self, PyObject *args) {
	int round = 0;
	if (!PyArg_ParseTuple(args, "i", &round)) {
		PyErr_SetString(HandisoError, "Bad argument");
		return NULL;
	}
	if (round < 0 || (uint_fast32_t)(round) >= self->indexer.rounds) {
		return PyLong_FromLong(0);
	}
    return PyLong_FromLong(self->indexer.permutations[round]);
}

/**
 * Get size of round
 *
 * @param round 
 * @returns size of round
 */
static PyObject* py_hand_indexer_round_size(IndexerObject *self, PyObject *args) {
	int round = 0;
	if (!PyArg_ParseTuple(args, "i", &round)) {
		PyErr_SetString(HandisoError, "Bad argument");
		return NULL;
	}
	if (round < 0 || (uint_fast32_t)(round) >= self->indexer.rounds) {
		return PyLong_FromLong(0);
	}
    return PyLong_FromLong(self->indexer.round_size[round]);
}

/**
 * Get cards number at round
 *
 * @param round 
 * @returns cards number at round
 */
static PyObject* py_hand_indexer_cards(IndexerObject *self, PyObject *args) {
	int round = 0;
	if (!PyArg_ParseTuple(args, "i", &round)) {
		PyErr_SetString(HandisoError, "Bad argument");
		return NULL;
	}
	if (round < 0 || (uint_fast32_t)(round) >= self->indexer.rounds) {
		return PyLong_FromLong(0);
	}
    return PyLong_FromLong(self->indexer.cards_per_round[round]);
}

/**
 * Get total size at specific round
 *
 * @param round 
 * @returns size of index for hands on round
 */
static PyObject* py_hand_indexer_size(IndexerObject *self, PyObject *args) {
	int round = 0;
	if (!PyArg_ParseTuple(args, "i", &round)) {
		PyErr_SetString(HandisoError, "Bad argument");
		return NULL;
	}
	return PyLong_FromLong(hand_indexer_size(&self->indexer, round));
}

/**
 * Index a hand on the last round.
 *
 * @param cards
 * @returns hand's index on the last round
 */
static PyObject* py_hand_index_last(IndexerObject *self, PyObject *args) {
	PyObject* pycards = NULL;
	if (!PyArg_ParseTuple(args, "O", &pycards)) {
		PyErr_SetString(HandisoError, "Bad argument");
		return NULL;
	}
	if (!PyList_Check(pycards)) {
		PyErr_SetString(HandisoError, "Bad argument at 0: list wanted");
		return NULL;
	}
	uint8_t cards[MAX_ROUNDS];
	int n = helper_parse_uint8s(pycards, cards);
	if (n < 1) {
		if (n < 0) {
			PyErr_SetString(HandisoError, "Bad argument at 0: list contains non-integer item");
		} else {
			PyErr_SetString(HandisoError, "Bad argument at 0: length of list must be greater than 0");
		}
		return NULL;
	}
	return PyLong_FromLongLong(hand_index_last(&self->indexer, cards));
}

/**
 * Incrementally index the next round.
 * 
 * @param cards the cards for the next round only!
 * @param state
 * @returns the hand's index at the latest round
 */
static PyObject* py_hand_index_next_round(IndexerObject *self, PyObject *args) {
	PyObject* pycards = NULL;
	PyObject* pystate = NULL;
	if (!PyArg_ParseTuple(args, "OO", &pycards, &pystate)) {
		PyErr_SetString(HandisoError, "Bad argument");
		return NULL;
	}
	if (!PyList_Check(pycards)) {
		PyErr_SetString(HandisoError, "Bad argument at 0: list wanted");
		return NULL;
	}
	uint8_t cards[MAX_ROUNDS];
	int n = helper_parse_uint8s(pycards, cards);
	if (n < 1) {
		if (n < 0) {
			PyErr_SetString(HandisoError, "Bad argument at 0: list contains non-integer item");
		} else {
			PyErr_SetString(HandisoError, "Bad argument at 0: length of list must be greater than 0");
		}
		return NULL;
	}
	StateObject* state = (StateObject*)(pystate);
	if (state->state.round+1 >= self->indexer.rounds) {
		PyErr_SetString(HandisoError, "Bad argument at 1: round out of range");
		return NULL;
	}
	return PyLong_FromLongLong(hand_index_next_round(&self->indexer, cards, &state->state));
}

/**
 * Recover the canonical hand from a particular index.
 *
 * @param round
 * @param index
 * @returns non-empty cards if successful
 */
static PyObject* py_hand_unindex(IndexerObject *self, PyObject *args) {
	int round = 0;
	long long index = 0;
	if (!PyArg_ParseTuple(args, "iL", &round, &index)) {
		PyErr_SetString(HandisoError, "Bad argument");
		return NULL;
	}
	uint8_t cards[MAX_ROUNDS];
	if (hand_unindex(&self->indexer, round, index, cards)) {
		PyObject* list = PyList_New(MAX_ROUNDS);
		for (int i = 0; i < MAX_ROUNDS; i++) {
			PyList_SetItem(list, i, PyLong_FromLong(cards[i]));
		}
		return list;
	}
	Py_RETURN_NONE;
}

static PyMethodDef indexerMethods[] = {
	{"rounds", (PyCFunction) py_hand_indexer_rounds, METH_VARARGS, "Get rounds of index"},
	{"configurations", (PyCFunction) py_hand_indexer_configurations, METH_VARARGS, "Get configurations at round"},
	{"permutations", (PyCFunction) py_hand_indexer_permutations, METH_VARARGS, "Get permutations at round"},
	{"round_size", (PyCFunction) py_hand_indexer_round_size, METH_VARARGS, "Get size of round"},
	{"cards", (PyCFunction) py_hand_indexer_cards, METH_VARARGS, "Get cards number at round"},
	{"size", (PyCFunction) py_hand_indexer_size, METH_VARARGS, "Get size of index for hands on round"},
	{"index_last", (PyCFunction) py_hand_index_last, METH_VARARGS, "Index a hand on last round"},
	{"index_next_round", (PyCFunction) py_hand_index_next_round, METH_VARARGS, "Incrementally index the next round"},
	{"unindex", (PyCFunction) py_hand_unindex, METH_VARARGS, "Recover the canonical hand from a particular index"},
	{NULL, NULL, 0, NULL}
};

/**
 * Wrap hand_indexer_t as a python object
 */
static PyTypeObject IndexerType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "handiso.Indexer",
    .tp_doc = "Indexer holds lookup table for index cards",
    .tp_basicsize = sizeof(IndexerObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc) py_hand_indexer_init,
    .tp_dealloc = (destructor) py_hand_indexer_dealloc,
    .tp_methods = indexerMethods,
};

/**
 * Wrap hand_indexer_state_t as a python object
 */
static PyTypeObject StateType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "handiso.State",
    .tp_doc = "State holds dynamic state information for indexer",
    .tp_basicsize = sizeof(StateObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc) py_hand_indexer_state_init,
    .tp_dealloc = (destructor) py_hand_indexer_state_dealloc,
};


static struct PyModuleDef handisomodule = {
	PyModuleDef_HEAD_INIT,
	.m_name = "handiso",
	.m_doc = "Poker hands are isomorphic with respect to "
		"permutations of the suits and ordering within a betting round. That is, "
		"AsKs, KdAd and KhAh all map to the same index preflop",
	.m_size = -1
};

/**
 * Initialize the module.
 */
PyMODINIT_FUNC PyInit_handiso(void) {
	PyObject *m;
    if (PyType_Ready(&IndexerType) < 0) {
        return NULL;
	}
    if (PyType_Ready(&StateType) < 0) {
        return NULL;
	}

    m = PyModule_Create(&handisomodule);
    if (m == NULL) {
        return NULL;
	}

	HandisoError = PyErr_NewException("handiso.Error", NULL, NULL);
    Py_XINCREF(HandisoError);
    if (PyModule_AddObject(m, "error", HandisoError) < 0) {
        Py_XDECREF(HandisoError);
        Py_CLEAR(HandisoError);
        Py_DECREF(m);
        return NULL;
    }

    Py_INCREF(&IndexerType);
    if (PyModule_AddObject(m, "Indexer", (PyObject *) &IndexerType) < 0) {
        Py_DECREF(&IndexerType);
        Py_DECREF(m);
        return NULL;
    }

    Py_INCREF(&StateType);
    if (PyModule_AddObject(m, "State", (PyObject *) &StateType) < 0) {
        Py_DECREF(&StateType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}
