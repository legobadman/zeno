#ifndef __FUNCTION_MANAGER_H__
#define __FUNCTION_MANAGER_H__

#include <vector>
#include <string>
#include <map>
#include <zeno/core/data.h>
#include <zeno/formula/syntax_tree.h>
#include <stack>

namespace zeno {

    struct ZfxVariable
    {
        std::vector<zfxvariant> value;  //��������Ա���(bAttr=true)������������Ĵ�С����runover�������棩��Ԫ�ظ������������size=1
        bool bAttr = false;     //�Ƿ������Թ���������ûʲô�ã�
        bool bAttrUpdated = false;      //ZfxVariableҲ��¼����ֵ������@P, @N @ptnum�ȣ����˱�Ǽ�¼��zfxִ���У�����ֵ�Ƿ��޸���
    };

    using VariableTable = std::map<std::string, ZfxVariable>;
    using ZfxVarRef = VariableTable::const_iterator;
    using ZfxElemFilter = std::vector<char>;

    struct ZfxStackEnv
    {
        VariableTable table;
        bool bAttrAddOrRemoved = false;
        size_t indexToCurrentElem = 0;
    };

    class FunctionManager
    {
    public:
        FunctionManager();
        std::vector<std::string> getCandidates(const std::string& prefix, bool bFunc) const;
        std::string getFuncTip(const std::string& funcName, bool& bExist) const;
        FUNC_INFO getFuncInfo(const std::string& funcName) const;
        void executeZfx(std::shared_ptr<ZfxASTNode> root, ZfxContext* ctx);
        zfxvariant calc(std::shared_ptr<ZfxASTNode> root, ZfxContext* pContext);
        ZENO_API void testExp();

    private:
        void init();
        float callRef(const std::string& ref, ZfxContext* pContext);
        ZfxVariable eval(const std::string& func, const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext);
        void pushStack();
        void popStack();
        bool hasTrue(const ZfxVariable& cond, const ZfxElemFilter& filter, ZfxElemFilter& newFilter) const;

        ZfxVariable& getVariableRef(const std::string& name, ZfxContext* pContext);
        bool declareVariable(const std::string& name);
        bool assignVariable(const std::string& name, ZfxVariable var, ZfxContext* pContext);
        void validateVar(operatorVals varType, ZfxVariable& newvar);
        ZfxVariable parseArray(std::shared_ptr<ZfxASTNode> pNode, ZfxElemFilter& filter, ZfxContext* pContext);
        ZfxVariable execute(std::shared_ptr<ZfxASTNode> root, ZfxElemFilter& filter, ZfxContext* pContext);
        std::vector<ZfxVariable> process_args(std::shared_ptr<ZfxASTNode> parent, ZfxElemFilter& filter, ZfxContext* pContext);

        ZfxVariable getAttrValue(const std::string& attrname, ZfxContext* pContext);
        void commitToPrim(const std::string& attrname, const ZfxVariable& val, ZfxElemFilter& filter, ZfxContext* pContext);
        bool removePoint(int pointnum, ZfxContext* pContext);
        void afterRemovePoint(int rempoint);

        VariableTable m_globalAttrCached;
        std::map<std::string, FUNC_INFO> m_funcs;
        std::vector<ZfxStackEnv> m_stacks;
    };
}

#endif