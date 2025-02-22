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

// �� C++ �ൽ Python
namespace py = pybind11;

PYBIND11_MODULE(ze, m) {  // `my_module` �� Python ���ģ����
    py::class_<MyClass>(m, "MyClass")
        .def(py::init<const std::string&, int>())  // �󶨹��캯��
        .def("setValue", &MyClass::setValue)       // �󶨳�Ա����
        .def("getValue", &MyClass::getValue)
        .def("getName", &MyClass::testzeno);
}