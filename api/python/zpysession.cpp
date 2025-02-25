#ifdef ZENO_PYAPI_STATICLIB
#include <Python.h>
#endif
#include <pybind11/pybind11.h>
#include <string>
#include <zeno/core/Session.h>
#include <zeno/core/Graph.h>
#include "zpyobject.h"
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

    py::class_<Zpy_Camera>(m, "Camera")
        .def(py::init<py::list, py::list, py::list, float, float, float>(),
            py::arg("pos") = py::list(py::make_tuple(0, 0, 5)),
            py::arg("up") = py::list(py::make_tuple(0, 1, 0)),
            py::arg("view") = py::list(py::make_tuple(0, 0, -1)),
            py::arg("fov") = 45.f,
            py::arg("aperture") = 11.0f,
            py::arg("focalPlaneDistance") = 2.0f
            )
        .def("setPos", &Zpy_Camera::setPos)
        .def("getPos", &Zpy_Camera::getPos)
        .def_property("pos", &Zpy_Camera::getPos, &Zpy_Camera::setPos)
        /**/
        .def("setUp", &Zpy_Camera::setUp)
        .def("getUp", &Zpy_Camera::getUp)
        .def_property("up", &Zpy_Camera::getUp, &Zpy_Camera::setUp)
        /**/
        .def("setView", &Zpy_Camera::setView)
        .def("getView", &Zpy_Camera::getView)
        .def_property("view", &Zpy_Camera::getView, &Zpy_Camera::setView)
        /**/
        .def("setFar", &Zpy_Camera::setFar)
        .def("getFar", &Zpy_Camera::getFar)
        .def_property("far", &Zpy_Camera::getFar, &Zpy_Camera::setFar)
        /**/
        .def("setFov", &Zpy_Camera::setFov)
        .def("getFov", &Zpy_Camera::getFov)
        .def_property("fov", &Zpy_Camera::getFov, &Zpy_Camera::setFov)
        /**/
        .def("setAperture", &Zpy_Camera::setAperture)
        .def("getAperture", &Zpy_Camera::getAperture)
        .def_property("aperture", &Zpy_Camera::getAperture, &Zpy_Camera::setAperture)
        /**/
        .def("setFocalPlaneDistance", &Zpy_Camera::setFocalPlaneDistance)
        .def("getFocalPlaneDistance", &Zpy_Camera::getFocalPlaneDistance)
        .def_property("focalPlaneDistance", &Zpy_Camera::getFocalPlaneDistance, &Zpy_Camera::setFocalPlaneDistance)
        ;

    py::class_<Zpy_Node>(m, "ZNode")     //其实并不希望用户在python允许构造这个，毕竟他只是一个proxy
        .def("setName", &Zpy_Node::set_name)
        .def("getName", &Zpy_Node::get_name)
        .def_property("name", &Zpy_Node::get_name, &Zpy_Node::set_name)
        .def("setView", &Zpy_Node::set_view)
        .def("isView", &Zpy_Node::is_view)
        .def_property("view", &Zpy_Node::is_view, &Zpy_Node::set_view)
        .def("updateParam", &Zpy_Node::update_param)
        .def("paramValue", &Zpy_Node::param_value)
        .def("setPos", &Zpy_Node::set_pos)
        .def("getPos", &Zpy_Node::get_pos)
        .def_property("pos", &Zpy_Node::get_pos, &Zpy_Node::set_pos)
        ;

    py::class_<Zpy_Graph>(m, "ZGraph")
        .def("newNode", &Zpy_Graph::newNode,
            py::arg("nodecls"),
            py::arg("originalname") = "",
            py::arg("bAssets") = false)
        .def("getName", &Zpy_Graph::getName)
        .def_property_readonly("name", &Zpy_Graph::getName)
        .def("getNode", &Zpy_Graph::getNode)
        .def("removeNode", &Zpy_Graph::removeNode)
        .def("addEdge", &Zpy_Graph::addEdge)
        ;

    py::class_<Zpy_Session>(m, "Session")
        .def(py::init<>())
        .def("mainGraph", &Zpy_Session::mainGraph);
}
