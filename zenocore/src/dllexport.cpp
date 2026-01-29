#include <zeno/zeno.h>
#include <zcommon.h>

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

//TODO: can export factory function, to create object