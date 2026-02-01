#include <zeno/zeno.h>
#include <zcommon.h>
#include <iobject2.h>
#include <zeno/types/GeometryObject.h>

extern "C" __declspec(dllexport)
int __cdecl c_func() {
	return 233;
}

//std::vector<int> aaa;

extern "C" __declspec(dllexport)
bool __cdecl registerNode(zeno::INode2* (*ctor)(), void (*dtor)(zeno::INode2*), const char* name, const ZNodeDescriptor& desc) {
	//INode2* pNode = ctor();
	std::string nodecls(name);
	auto& nodeReg = zeno::getNodeRegister();
	nodeReg.registerNodeClass2(ctor, dtor, nodecls, desc);
	return true;
}

extern "C" __declspec(dllexport)
bool __cdecl unRegisterNode(const char* name) {
	auto& nodeReg = zeno::getNodeRegister();
	nodeReg.unregisterNodeClass(std::string(name));
	return true;
}

//extern "C" __declspec(dllexport)
//zeno::IGeometryObject* createEmptyGeometry(zeno::GeomTopoType type) {
//	//TODO
//	return nullptr;
//}
//
//extern "C" __declspec(dllexport)
//zeno::IGeometryObject* __cdecl createGeometry(zeno::GeomTopoType type, bool bTriangle, int nPoints, int nFaces, bool bInitFaces = false) {
//
//	return zeno::create_GeometryObject(type, bTriangle, nPoints, nFaces, bInitFaces).release();
//}
//
//extern "C" __declspec(dllexport)
//zeno::IGeometryObject* __cdecl createGeometryByPointFace(
//	zeno::GeomTopoType type,
//	bool bTriangle,
//	const zeno::Vec3f* points,
//	size_t num_of_points,
//	const ZIntArray* faces_points,
//	size_t num_of_facespts)
//{
//	return nullptr;
//}

//TODO: can export factory function, to create object