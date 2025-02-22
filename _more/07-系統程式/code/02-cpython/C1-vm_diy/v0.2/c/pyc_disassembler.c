#include <Python.h>
#include <marshal.h>
#include <opcode.h>  // 用於操作碼
// #include <internal/pycore_opcode_metadata.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "op.h"

#define HEADER_SIZE 16  // Python 3.12 header size

void print_code_object(PyCodeObject *code_obj) {
    printf("Disassembling code object: %s\n", PyUnicode_AsUTF8(code_obj->co_name));
    printf("Filename: %s\n", PyUnicode_AsUTF8(code_obj->co_filename));
    printf("First Line Number: %d\n", code_obj->co_firstlineno);
    
    // Print constants
    printf("Constants:\n");
    for (Py_ssize_t i = 0; i < PyTuple_Size(code_obj->co_consts); i++) {
        PyObject *const_item = PyTuple_GetItem(code_obj->co_consts, i);
        printf("  [%zd] %s\n", i, PyUnicode_Check(const_item) ? 
             PyUnicode_AsUTF8(const_item) : "Non-string constant");
        /*
        if (PyUnicode_Check(const_item))
            printf("  [%zd] %s\n", i,  PyUnicode_AsUTF8(const_item));
        else
            printf("  [%zd] %d\n", i,  *(int*)const_item);
        */
    }
    
    // Print names
    printf("Names:\n");
    for (Py_ssize_t i = 0; i < PyTuple_Size(code_obj->co_names); i++) {
        PyObject *name_item = PyTuple_GetItem(code_obj->co_names, i);
        printf("  [%zd] %s\n", i, PyUnicode_AsUTF8(name_item));
    }

    // Print variable names
    printf("Variable Names:\n");
    for (Py_ssize_t i = 0; i < PyTuple_Size(code_obj->co_names); i++) {
        PyObject *var_item = PyTuple_GetItem(code_obj->co_names, i);
        printf("  [%zd] %s\n", i, PyUnicode_AsUTF8(var_item));
    }
// 這段有錯
    // Print the bytecode
    printf("Bytecode:\n");
    PyObject *bytecode = PyCode_GetCode(code_obj); // (PyObject *) code_obj->co_code_adaptive;
    Py_ssize_t bytecode_size = PyBytes_Size(bytecode);
    unsigned char *code = (unsigned char *)PyBytes_AsString(bytecode);
    for (Py_ssize_t i = 0; i < bytecode_size; ) {
        Py_ssize_t addr = i;
        int opcode = code[i++];
        int arg_count = code[i++];
        printf("%4zd: %s %d", addr, op_names[opcode], arg_count);  // 使用 Python 的操作碼名稱 (這個在 internal/pycore_opcode_metadata 引用不到)
        if (opcode == CALL) i+=6; // CALL 指令占 8byte (不知為何？)

        printf("\n");
        // i++;
    }
}

PyCodeObject* load_code_object(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        perror("Error opening file");
        return NULL;
    }

    // Read header
    unsigned char header[HEADER_SIZE];
    fread(header, sizeof(unsigned char), HEADER_SIZE, f);

    // Read the code object using marshal
    PyObject *code_obj = PyMarshal_ReadObjectFromFile(f);
    fclose(f);
    
    if (!code_obj || !PyCode_Check(code_obj)) {
        fprintf(stderr, "Failed to read a code object.\n");
        return NULL;
    }

    return (PyCodeObject *)code_obj;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <pyc file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Initialize Python interpreter
    Py_Initialize();

    PyCodeObject *code_obj = load_code_object(argv[1]);
    if (code_obj) {
        print_code_object(code_obj);
        Py_DECREF(code_obj);  // Decrease reference count
    }

    // Finalize Python interpreter
    Py_Finalize();
    return EXIT_SUCCESS;
}
