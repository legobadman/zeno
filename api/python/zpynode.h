#pragma once

#ifndef __ZPYNODE_H__
#define __ZPYNODE_H__

#ifdef ZENO_PYAPI_STATICLIB
#include <Python.h>
#endif

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <zeno/core/NodeImpl.h>

class Zpy_Node {
public:
    Zpy_Node();
    Zpy_Node(zeno::NodeImpl* ptrNode);
    Zpy_Node(const Zpy_Node& rhs);
    ~Zpy_Node();

    void set_name(const std::string& name);
    std::string get_name() const;

    void set_view(bool bOn);
    bool is_view() const;

    void update_param(const std::string& name, py::object obj);
    py::object param_value(const std::string& name);

    void set_pos(const py::tuple pos);
    py::tuple get_pos() const;

private:
    std::string m_name;
    zeno::NodeImpl* m_ptrNode;
};

#endif