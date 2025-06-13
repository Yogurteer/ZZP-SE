#include "../utils/encrypt.h"
#include "../utils/inter.h"
#include "../utils/utils.h"
#include "../include/KS2E.h"
#include "../include/DBOGaccess.hpp"

#define PY_SSIZE_T_CLEAN
#include<Python.h>
KS2EUser* client;
DBOGaccess db1("dbname=zzpdb port=5432 host=222.20.126.206 user=admin password=hust@123");

// 待完善更新部分的接口函数

static void pylist_to_vecs(PyObject* args, vector<vector<unsigned char>>& datavecs){
    // Convert input from Python list of bytes
    PyObject* py_list1 = nullptr;
    if (!PyArg_ParseTuple(args, "O", &py_list1)) { // 解析两个参数: 字符串 + 列表
        PyErr_SetString(PyExc_TypeError, "Expected a string and a list");
        return;
    }// 检查第二个参数是否为列表
    if (!PyList_Check(py_list1)) {
        PyErr_SetString(PyExc_TypeError, "First argument must be a list");
        return;
    }
    Py_ssize_t list_size = PyList_Size(py_list1); // 获取列表长度
    vector<vector<unsigned char>> datavecs; // input Pid = {L, Jid, id, kid}
    for (Py_ssize_t i = 0; i < list_size; ++i) {
        PyObject* item = PyList_GetItem(py_list1, i);  // 不会增加引用计数
        if (!PyBytes_Check(item)) {
            PyErr_SetString(PyExc_TypeError, "List elements must be bytes");
            return;
        }
        const char* bytes_data = PyBytes_AsString(item);
        Py_ssize_t bytes_len = PyBytes_Size(item);
        datavecs.emplace_back(
            reinterpret_cast<const uint8_t*>(bytes_data),
            reinterpret_cast<const uint8_t*>(bytes_data) + bytes_len
        );
    }
}

static PyObject* py_init(PyObject* self, PyObject* args)
{
    client = new KS2EUser();
    client->setup();
    PyObject* ret;
    ret = Py_BuildValue("i", 0);
    return ret;
}

static PyObject* py_update(PyObject* self, PyObject* args)
{
    cout<< "py_update called"<< endl;

    if (client == nullptr) {
        PyErr_SetString(PyExc_RuntimeError, "Client not initialized");
        return nullptr; // client not initialized
    }
    
    const char* str_w;
    const char* str_id;
    Py_ssize_t w_length;
    Py_ssize_t i_length;
    // 解析
    if (!PyArg_ParseTuple(args, "s#s#", &str_w, &w_length, &str_id, &i_length)) {
        return nullptr; // Error parsing arguments
    }
    // Call the client's update method
    vector<unsigned char> op = {'0'}; // op为0表示添加操作
    vector<unsigned char> id(str_id, str_id + i_length);
    vector<unsigned char> w(str_w, str_w + w_length);
    pair<vector<unsigned char>, vector<unsigned char>> w_id = make_pair(w, id);
    pair<vector<unsigned char>, cipher> Cwid;

    client->update_1(w_id, op, Cwid);

    // Convert vector<vector<unsigned char>> to Python list of bytes
    // Cw to vector<vector<unsigned char>>
    vector<vector<unsigned char>> data = {Cwid.first, Cwid.second.Iw, Cwid.second.Rw, Cwid.second.Cw, Cwid.second.Iid, Cwid.second.Rid, Cwid.second.Cid};
    PyObject* py_list = PyList_New(data.size());
    if (!py_list) {
        return NULL; // Error creating Python list
    }
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

static PyObject* py_trapdoor_w(PyObject* self, PyObject* args)
{
    cout<< "py_trapdoor_w called" << endl;
    if (client == nullptr) {
        PyErr_SetString(PyExc_RuntimeError, "Client not initialized");
        return nullptr; // client not initialized
    }
    
    const char* str_w;
    Py_ssize_t w_length;

    // 解析
    if (!PyArg_ParseTuple(args, "s#", &str_w, &w_length)) {
        return nullptr; // Error parsing arguments
    }
    // Call the client's trapdoor_w method
    vector<unsigned char> w(str_w, str_w + w_length);
    vector<vector<unsigned char>> Tw;
    client->trapdoor_w(w, Tw); // Tw = {L, Jw}

    // Convert c++ data to Python list of bytes
    vector<vector<unsigned char>> data = Tw;
    PyObject* py_list = PyList_New(data.size());
    if (!py_list) {
        return NULL; // Error creating Python list
    }
    
    for (size_t i = 0; i < data.size(); i++) {
        const vector<unsigned char>& item = data[i];
        PyObject* py_bytes = PyBytes_FromStringAndSize(reinterpret_cast<const char*>(item.data()), item.size());
        if (!py_bytes) {
            Py_DECREF(py_list);
            return NULL; // Error creating Python bytes
        }
        PyList_SetItem(py_list, i, py_bytes); // 接管所有权
    }

    return py_list; // Return the list of bytes   
}

static PyObject* py_decresult_w(PyObject* self, PyObject* args)
{
    cout<< "py_decresult_w called" << endl;
    if (client == nullptr) {
        PyErr_SetString(PyExc_RuntimeError, "Client not initialized");
        return nullptr; // client not initialized
    }
    const char* str_w;
    Py_ssize_t w_length;
    PyObject* py_list = nullptr;
    // 解析两个参数: 字符串 + 列表
    if (!PyArg_ParseTuple(args, "s#O", &str_w, &w_length, &py_list)) {
        PyErr_SetString(PyExc_TypeError, "Expected a string and a list");
        return nullptr;
    }
    // 检查第二个参数是否为列表
    if (!PyList_Check(py_list)) {
        PyErr_SetString(PyExc_TypeError, "Second argument must be a list");
        return nullptr;
    }
    // 获取列表长度
    Py_ssize_t list_size = PyList_Size(py_list);
    std::vector<std::vector<unsigned char>> c_list;
    for (Py_ssize_t i = 0; i < list_size; ++i) {
        PyObject* item = PyList_GetItem(py_list, i);  // 不会增加引用计数
        if (!PyBytes_Check(item)) {
            PyErr_SetString(PyExc_TypeError, "List elements must be bytes");
            return nullptr;
        }
        const char* bytes_data = PyBytes_AsString(item);
        Py_ssize_t bytes_len = PyBytes_Size(item);
        c_list.emplace_back(
            reinterpret_cast<const uint8_t*>(bytes_data),
            reinterpret_cast<const uint8_t*>(bytes_data) + bytes_len
        );
    }
    // Call the client's decresult_w method
    vector<unsigned char> w(str_w, str_w + w_length);
    vector<vector<unsigned char>> S = c_list; // S = {Cw...}
    vector<vector<unsigned char>> getids;
    getids = client->dec_result_w(w, S); // Tw = {L, Jw}
    // Convert c++ data to Python list of bytes
    vector<vector<unsigned char>> data = getids;
    PyObject* py_list_return = PyList_New(data.size()); // py_list = {id...}
    if (!py_list_return) {
        return NULL; // Error creating Python list
    }
    for (size_t i = 0; i < data.size(); i++) {
        const vector<unsigned char>& item = data[i];
        PyObject* py_bytes = PyBytes_FromStringAndSize(reinterpret_cast<const char*>(item.data()), item.size());
        if (!py_bytes) {
            Py_DECREF(py_list_return);
            return NULL; // Error creating Python bytes
        }
        PyList_SetItem(py_list_return, i, py_bytes); // 接管所有权
    }

    return py_list_return; // Return the list of bytes 
}

static PyObject* py_trapdoor_id(PyObject* self, PyObject* args)
{
    cout<< "py_trapdoor_id called" << endl;
    const char* str_id;
    Py_ssize_t i_length;
    if (!PyArg_ParseTuple(args, "s#", &str_id, &i_length)) {
        return NULL; // Error parsing arguments
    }
    if (client == nullptr) {
        PyErr_SetString(PyExc_RuntimeError, "Client not initialized");
        return nullptr; // client not initialized
    }
    // Call the client's search method
    vector<unsigned char> id(str_id, str_id + i_length);
    vector<vector<unsigned char>> Tid;
    client->trapdoor_id(id, Tid);// send Tid to server
    // Convert vector<vector<unsigned char>> to Python list of bytes
    vector<vector<unsigned char>> data = Tid;
    PyObject* py_list = PyList_New(data.size());
    if (!py_list) {
        return NULL; // Error creating Python list
    }
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

static PyObject* py_decresult_id(PyObject* self, PyObject* args)
{
    cout<< "py_decresult_id called" << endl;
    if (client == nullptr) {
        PyErr_SetString(PyExc_RuntimeError, "Client not initialized");
        return nullptr; // client not initialized
    }
    const char* str_id;
    Py_ssize_t id_length;
    PyObject* py_list = nullptr;
    // 解析两个参数: 字符串 + 列表
    if (!PyArg_ParseTuple(args, "s#O", &str_id, &id_length, &py_list)) {
        PyErr_SetString(PyExc_TypeError, "Expected a string and a list");
        return nullptr;
    }
    // 检查第二个参数是否为列表
    if (!PyList_Check(py_list)) {
        PyErr_SetString(PyExc_TypeError, "Second argument must be a list");
        return nullptr;
    }
    // 获取列表长度
    Py_ssize_t list_size = PyList_Size(py_list);
    std::vector<std::vector<unsigned char>> c_list;
    for (Py_ssize_t i = 0; i < list_size; ++i) {
        PyObject* item = PyList_GetItem(py_list, i);  // 不会增加引用计数

        if (!PyBytes_Check(item)) {
            PyErr_SetString(PyExc_TypeError, "List elements must be bytes");
            return nullptr;
        }

        const char* bytes_data = PyBytes_AsString(item);
        Py_ssize_t bytes_len = PyBytes_Size(item);

        c_list.emplace_back(
            reinterpret_cast<const uint8_t*>(bytes_data),
            reinterpret_cast<const uint8_t*>(bytes_data) + bytes_len
        );
    }
    vector<unsigned char> id(str_id, str_id + id_length);
    vector<vector<unsigned char>> S = c_list; // S = {Cid...}
    vector<vector<unsigned char>> getws;
    getws = client->dec_result_id(id, S);
    // Convert c++ data to Python list of bytes
    vector<vector<unsigned char>> data = getws;
    PyObject* py_list_return = PyList_New(data.size()); // py_list = {w...}
    if (!py_list_return) {
        return NULL; // Error creating Python list
    }
    for (size_t i = 0; i < data.size(); i++) {
        const vector<unsigned char>& item = data[i];
        PyObject* py_bytes = PyBytes_FromStringAndSize(reinterpret_cast<const char*>(item.data()), item.size());
        if (!py_bytes) {
            Py_DECREF(py_list_return);
            return NULL; // Error creating Python bytes
        }
        PyList_SetItem(py_list_return, i, py_bytes); // 接管所有权
    }

    return py_list_return; // Return the list of bytes 
}

static PyObject* py_share_id_1(PyObject* self, PyObject* args){
    cout<< "py_share_id_1 called" << endl;
    // Convert input from Python list of bytes
    PyObject* py_list1 = nullptr;
    if (!PyArg_ParseTuple(args, "O", &py_list1)) { // 解析两个参数: 字符串 + 列表
        PyErr_SetString(PyExc_TypeError, "Expected a string and a list");
        return nullptr;
    }// 检查第二个参数是否为列表
    if (!PyList_Check(py_list1)) {
        PyErr_SetString(PyExc_TypeError, "First argument must be a list");
        return nullptr;
    }
    Py_ssize_t list_size = PyList_Size(py_list1); // 获取列表长度
    vector<vector<unsigned char>> Pid; // input Pid = {L, Jid, id, kid}
    for (Py_ssize_t i = 0; i < list_size; ++i) {
        PyObject* item = PyList_GetItem(py_list1, i);  // 不会增加引用计数
        if (!PyBytes_Check(item)) {
            PyErr_SetString(PyExc_TypeError, "List elements must be bytes");
            return nullptr;
        }
        const char* bytes_data = PyBytes_AsString(item);
        Py_ssize_t bytes_len = PyBytes_Size(item);
        Pid.emplace_back(
            reinterpret_cast<const uint8_t*>(bytes_data),
            reinterpret_cast<const uint8_t*>(bytes_data) + bytes_len
        );
    }
    // Call the client's search method
    vector<vector<unsigned char>> Did; 
    client->share_id_1(Pid, Did);
    // Convert output to Python list of bytes
    vector<vector<unsigned char>> data = Did; // Did = {L, Jid}
    PyObject* py_list = PyList_New(data.size());
    if (!py_list) {
        return NULL;
    }
    for (size_t i = 0; i < data.size(); i++) {
        const vector<uint8_t>& item = data[i];
        PyObject* py_bytes = PyBytes_FromStringAndSize(reinterpret_cast<const char*>(item.data()), item.size());
        if (!py_bytes) {
            Py_DECREF(py_list);
            return NULL;
        }
        PyList_SetItem(py_list, i, py_bytes); // 接管所有权
    }
    return py_list;
}

static PyObject* py_share_id_2(PyObject* self, PyObject* args){
    cout<< "py_trapdoor_id called" << endl;
    // Convert input from Python list of bytes
    
    PyObject* py_list1 = nullptr;
    PyObject* py_list2 = nullptr;
    if (!PyArg_ParseTuple(args, "OO", &py_list1, &py_list2)) { // 解析两个参数: 列表 + 列表
        PyErr_SetString(PyExc_TypeError, "Expected a string and a list");
        return nullptr;
    }// 检查第二个参数是否为列表
    if (!PyList_Check(py_list1)) {
        PyErr_SetString(PyExc_TypeError, "First argument must be a list");
        return nullptr;
    }
    if (!PyList_Check(py_list2)) {
        PyErr_SetString(PyExc_TypeError, "Second argument must be a list");
        return nullptr;
    }
    Py_ssize_t list_size1 = PyList_Size(py_list1); // 获取列表长度
    vector<vector<unsigned char>> Pid; // input Pid = {L, Jid, id, kid}
    for (Py_ssize_t i = 0; i < list_size1; ++i) {
        PyObject* item = PyList_GetItem(py_list1, i);  // 不会增加引用计数
        if (!PyBytes_Check(item)) {
            PyErr_SetString(PyExc_TypeError, "List elements must be bytes");
            return nullptr;
        }
        const char* bytes_data = PyBytes_AsString(item);
        Py_ssize_t bytes_len = PyBytes_Size(item);
        Pid.emplace_back(
            reinterpret_cast<const uint8_t*>(bytes_data),
            reinterpret_cast<const uint8_t*>(bytes_data) + bytes_len
        );
    }
    Py_ssize_t list_size2 = PyList_Size(py_list2); // 获取列表长度
    vector<vector<unsigned char>> S; // input S = {Cw...}
    for (Py_ssize_t i = 0; i < list_size2; ++i) {
        PyObject* item = PyList_GetItem(py_list2, i);  // 不会增加引用计数
        if (!PyBytes_Check(item)) {
            PyErr_SetString(PyExc_TypeError, "List elements must be bytes");
            return nullptr;
        }
        const char* bytes_data = PyBytes_AsString(item);
        Py_ssize_t bytes_len = PyBytes_Size(item);
        S.emplace_back(
            reinterpret_cast<const uint8_t*>(bytes_data),
            reinterpret_cast<const uint8_t*>(bytes_data) + bytes_len
        );
    }
    // Call the client's search method
    vector<vector<vector<unsigned char>>> S_1;
    map<vector<unsigned char>, vector<vector<unsigned char>>> u_share_id_result;
    client->share_id_2(Pid, S, S_1, u_share_id_result);
    search_output_vk(u_share_id_result, share_id_dir, db1);
    // Convert output to Python list of bytes
    vector<vector<unsigned char>> data ; 
    PyObject* py_list = PyList_New(data.size());
    if (!py_list) {
        return NULL;
    }
    for (size_t i = 0; i < data.size(); i++) {
        const vector<uint8_t>& item = data[i];
        PyObject* py_bytes = PyBytes_FromStringAndSize(reinterpret_cast<const char*>(item.data()), item.size());
        if (!py_bytes) {
            Py_DECREF(py_list);
            return NULL;
        }
        PyList_SetItem(py_list, i, py_bytes); // 接管所有权
    }
    return py_list;
}

static PyObject* py_free(PyObject* self,PyObject* args)
{
    PyObject* ret;
    if (client != NULL)
    {
        delete client;
        client = NULL;
    }
    ret = Py_BuildValue("i", 0);
    return ret;
}

static PyMethodDef KS2E_Methods[] = {
    {"init", py_init, METH_NOARGS, "Initialize client ."},
    {"update", py_update, METH_VARARGS, "Update the server with a new entry."},
    {"trapdoor_w", py_trapdoor_w, METH_VARARGS, "Generate trapdoor for w."},
    {"decresult_w", py_decresult_w, METH_VARARGS, "Decode result for w."},
    {"trapdoor_id", py_trapdoor_id, METH_VARARGS, "Generate trapdoor for id."},
    {"decresult_id", py_decresult_id, METH_VARARGS, "Decode result for id."},
    {"py_free", py_free, METH_NOARGS, "Free the client."},
    {NULL, NULL, 0, NULL} // Sentinel
};

static struct PyModuleDef KS2Emodule = {
    PyModuleDef_HEAD_INIT,
    "KS2EUser",   /* name of module */
    NULL,      /* module documentation, may be NULL */
    -1,        /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    KS2E_Methods
};

PyMODINIT_FUNC PyInit_KS2E_User(void)
{
    return PyModule_Create(&KS2Emodule);
}