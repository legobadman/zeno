#ifdef ZENO_PYAPI_STATICLIB
#include <Python.h>
#endif
#include <pybind11/pybind11.h>
#include <string>
#include <zeno/core/Session.h>
#include <zeno/core/Graph.h>
#include "zpygraph.h"
#include <memory>


#if 1
class MyClass {
public:
    MyClass(const std::string& name, int value)
        : name(name)
        , value(value)
        , m_pSess(std::make_unique<zeno::Session>())
    {
        m_pSess = std::make_unique<zeno::Session>();
    }

    void setValue(int v) { value = v; }
    int getValue() const { return value; }

    std::string getName() const { return name; }

    std::string testzeno() {
        return m_pSess->mainGraph->getName();
    }

private:
    std::string name;
    int value;
    std::unique_ptr<zeno::Session> m_pSess;
};
#endif

class Zpy_Session {
public:
    Zpy_Session() : sess(zeno::getSession()) {
    }

    Zpy_Graph mainGraph() const {
        return Zpy_Graph(sess.mainGraph);
    }
private:
    zeno::Session& sess;
};


namespace py = pybind11;

PYBIND11_MODULE(zen, m) {  // `ze` 是python里import的模块名
    py::class_<MyClass>(m, "MyClass")
        .def(py::init<const std::string&, int>())  // 绑定构造函数
        .def("setValue", &MyClass::setValue)       // 绑定成员函数
        .def("getValue", &MyClass::getValue)
        .def("getName", &MyClass::testzeno);

    py::class_<Zpy_Node>(m, "ZNode")     //其实并不希望用户在python允许构造这个，毕竟他只是一个proxy
        .def("setName", &Zpy_Node::set_name)
        .def("getName", &Zpy_Node::get_name)
        .def("setView", &Zpy_Node::set_view)
        .def("isView", &Zpy_Node::is_view)
        .def("updateParam", &Zpy_Node::update_param)
        .def("paramValue", &Zpy_Node::param_value)
        .def("setPos", &Zpy_Node::set_pos)
        .def("getPos", &Zpy_Node::get_pos)
        ;

    py::class_<Zpy_Graph>(m, "ZGraph")
        .def("newNode", &Zpy_Graph::newNode)
        .def("getName", &Zpy_Graph::getName)
        .def("getNode", &Zpy_Graph::getNode)
        .def("removeNode", &Zpy_Graph::removeNode)
        .def("addEdge", &Zpy_Graph::addEdge)
        ;

    py::class_<Zpy_Session>(m, "Session")
        .def(py::init<>())
        .def("mainGraph", &Zpy_Session::mainGraph);
}
