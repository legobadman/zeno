#pragma once

#ifndef __ZPYOBJECT_H__
#define __ZPYOBJECT_H__

#ifdef ZENO_PYAPI_STATICLIB
#include <Python.h>
#endif

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <zeno/core/INode.h>
#include <zeno/types/CameraObject.h>

namespace py = pybind11;

class Zpy_Object {

};

class Zpy_Camera {
public:
    Zpy_Camera(
        py::list pos,
        py::list up,
        py::list view,
        float fov,
        float aperture,
        float focalPlaneDistance
    );
    Zpy_Camera(std::shared_ptr<zeno::CameraObject> pCamera);
    py::list getPos() const;
    void setPos(py::list v);
    py::list getUp() const;
    void setUp(py::list v);
    py::list getView() const;
    void setView(py::list v);
    float getNear() const;
    void setNear(float near);
    float getFar() const;
    void setFar(float far);
    float getFov() const;
    void setFov(float fov);
    float getAperture() const;
    void setAperture(float aperture);
    float getFocalPlaneDistance() const;
    void setFocalPlaneDistance(float focal);

private:
    void run();
    std::shared_ptr<zeno::CameraObject> getCamera() const;

    std::weak_ptr<zeno::INode> m_wpNode;
    std::weak_ptr<zeno::CameraObject> m_wpCamera;
};

class Zpy_Light {

};

#endif