#include <pybind11/pybind11.h>
#include <string>
#include <zeno/core/Session.h>
#include <zeno/core/Graph.h>
#include <memory>


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

// 绑定 C++ 类到 Python
namespace py = pybind11;

PYBIND11_MODULE(ze, m) {  // `my_module` 是 Python 里的模块名
    py::class_<MyClass>(m, "MyClass")
        .def(py::init<const std::string&, int>())  // 绑定构造函数
        .def("setValue", &MyClass::setValue)       // 绑定成员函数
        .def("getValue", &MyClass::getValue)
        .def("getName", &MyClass::testzeno);
}