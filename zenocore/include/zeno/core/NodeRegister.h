#pragma once

#include <memory>
#include <map>
#include <string>
#include <zeno/core/data.h>
#include <zeno/core/Descriptor.h>
#include <zeno/utils/api.h>
#include <zcommon.h>

namespace zeno {

	struct INodeClass;
	struct INode;

	struct NodeRegister {
		struct _ObjUIInfo
		{
			std::string_view name;
			std::string_view color;
		};

	public:
		static NodeRegister& instance();
		ZENO_API int registerNodeClass(INode* (*ctor)(), std::string const& clsname, Descriptor const& desc);
		ZENO_API int registerNodeClass(INode* (*ctor)(), std::string const& nodecls, CustomUI const& customui);
		ZENO_API int registerNodeClass2(INode2* (*ctor)(), void (*dtor)(INode2*), std::string const& nodecls, const ZNodeDescriptor& desc);
		ZENO_API void registerObjUIInfo(size_t hashcode, std::string_view color, std::string_view nametip);
		ZENO_API int unregisterNodeClass(std::string const& nodecls);
		ZENO_API bool getObjUIInfo(size_t hashcode, std::string_view& color, std::string_view& nametip);
		ZENO_API INodeClass* getNodeClassPtr(const std::string& name);
		ZENO_API void beginLoadModule(const std::string& module_name);
		ZENO_API void uninstallModule(const std::string& module_name);
		ZENO_API void endLoadModule();
		ZENO_API zeno::CustomUI getOfficalUIDesc(const std::string& clsname, bool& bExist);
		ZENO_API void clear();
		ZENO_API std::vector<NodeInfo> dumpCoreCates() const;

	private:
		NodeRegister();

		std::map<std::string, std::unique_ptr<INodeClass>> nodeClasses;
		std::vector<NodeInfo> m_cates;
		std::string m_current_loading_module;
		std::map<size_t, _ObjUIInfo> m_objsUIInfo;
	};

	ZENO_API NodeRegister& getNodeRegister();
}