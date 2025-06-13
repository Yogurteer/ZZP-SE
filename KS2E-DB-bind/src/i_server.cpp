#include "./utils/encrypt.h"
#include "./utils/inter.h"
#include "./utils/utils.h"
#include "./include/KS2E.h"
#include "./include/DBOGaccess.hpp"

#define PY_SSIZE_T_CLEAN
#include<Python.h>
KS2EServer* server;
DBOGaccess db1;

static PyObject* py_init(PyObject* self, PyObject* args)
{
    server = new KS2EServer();
    server->setup();
    PyObject* ret;
    ret = Py_BuildValue("i", 0);
    return ret;
}

static PyObject* py_re_update(PyObject* self, PyObject* args)
{

}

static PyObject* py_search_w(PyObject* self, PyObject* args)
{

}

static PyObject* py_re_update(PyObject* self, PyObject* args)
{
    cout<< "re_update called"<< endl;

    if (server == nullptr) {
        PyErr_SetString(PyExc_RuntimeError, "server not initialized");
        return nullptr; // server not initialized
    }
    
    const char* str_w;
    const char* str_id;
    Py_ssize_t w_length;
    Py_ssize_t i_length;
    // 解析
    if (!PyArg_ParseTuple(args, "s#s#", &str_w, &w_length, &str_id, &i_length)) {
        return nullptr; // Error parsing arguments
    }
    cout<<"w: "<<str_w<<endl;
    
    // Call the server's update method
    vector<unsigned char> op = {'0'}; // op为0表示添加操作

    vector<unsigned char> id(str_id, str_id + i_length);
    vector<unsigned char> w(str_w, str_w + w_length);
    pair<vector<unsigned char>, vector<unsigned char>> w_id = make_pair(w, id);

    pair<vector<unsigned char>, cipher> Cwid;
    cout << "re_update called " << endl;
    server->re_o_update(Cwid, db1);

    // Convert vector<vector<unsigned char>> to Python list of bytes
    // Cw to vector<vector<unsigned char>>
    cout << "convert pylist..." << endl;
    vector<vector<unsigned char>> data = {Cwid.first, Cwid.second.Iw, Cwid.second.Rw, Cwid.second.Cw, Cwid.second.Iid, Cwid.second.Rid, Cwid.second.Cid};
    cout << "data size: " << data.size() << endl;
    PyObject* py_list = PyList_New(data.size());
    if (!py_list) {
        return NULL; // Error creating Python list
    }
    
    cout << "data convert to pylist" << endl;
    for (size_t i = 0; i < data.size(); i++) {
        const vector<uint8_t>& item = data[i];
        PyObject* py_bytes = PyBytes_FromStringAndSize(reinterpret_cast<const char*>(item.data()), item.size());
        if (!py_bytes) {
            Py_DECREF(py_list);
            return NULL; // Error creating Python bytes
        }
        PyList_SetItem(py_list, i, py_bytes); // 接管所有权
    }

    return py_list; // Return the list of bytes
}


static PyObject* py_free(PyObject* self,PyObject* args)
{
    PyObject* ret;
    if (server != NULL)
    {
        delete server;
        server = NULL;
    }
    ret = Py_BuildValue("i", 0);
    return ret;
}

static PyMethodDef KS2E_Methods[] = {
    {"init", py_init, METH_NOARGS, "Initialize server ."},
    {NULL, NULL, 0, NULL} // Sentinel
};

static struct PyModuleDef KS2Emodule = {
    PyModuleDef_HEAD_INIT,
    "KS2E_Server",   /* name of module */
    NULL,      /* module documentation, may be NULL */
    -1,        /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    KS2E_Methods
};

PyMODINIT_FUNC PyInit_KS2E_Client(void)
{
    return PyModule_Create(&KS2Emodule);
}