#include <zeno/formula/zfxexecute.h>
#include <zeno/core/Session.h>
#include <zeno/extra/GlobalState.h>
#include <zeno/core/Graph.h>
#include <zeno/extra/GraphException.h>
#include "zfxscanner.h"
#include "zfxparser.hpp"
#include <regex>
#include <zeno/core/FunctionManager.h>


namespace zeno
{

ZENO_API ZfxExecute::ZfxExecute(const std::string& code, ZfxContext* ctx)
    : m_location(0)
    , m_code(code)
    , m_context(ctx)
{
}

ZENO_API ZfxExecute::~ZfxExecute()
{
}

ZENO_API int ZfxExecute::parse() {
    std::stringstream inStream;
    std::stringstream outStream;
    ZfxScanner scanner(inStream, outStream, *this);
    ZfxParser parser(scanner, *this);
    m_location = 0;
    inStream << m_code << std::endl;
    int ret = parser.parse();
    return ret;
}

ZENO_API int ZfxExecute::execute() {
    int ret = parse();
    if (ret != 0) {
        throw makeError<ZfxParseError>();
        return ret;
    }
    //TODO: error exception catch.
    if (m_root) {
        auto& funcMgr = zeno::getSession().funcManager;
        funcMgr->executeZfx(m_root, m_context);
    }
    else {

    }
    return ret;
}

std::shared_ptr<ZfxASTNode> ZfxExecute::makeNewNode(nodeType type, operatorVals op, std::vector<std::shared_ptr<ZfxASTNode>> children) {
    auto pNode = newNode(type, op, children);
    return pNode;
}

std::shared_ptr<ZfxASTNode> ZfxExecute::makeNewNumberNode(float value) {
    auto pNode = newNumberNode(value);
    return pNode;
}

std::shared_ptr<ZfxASTNode> ZfxExecute::makeBoolNode(bool bVal) {
    std::shared_ptr<ZfxASTNode> n = std::make_shared<ZfxASTNode>();
    if (!n)
    {
        exit(0);
    }
    n->type = BOOLTYPE;
    n->opVal = UNDEFINE_OP;
    n->value = bVal;
    return n;
}

std::shared_ptr<ZfxASTNode> ZfxExecute::makeStringNode(std::string text) {
    std::shared_ptr<ZfxASTNode> spNode = std::make_shared<ZfxASTNode>();
    spNode->type = STRING;
    spNode->opVal = UNDEFINE_OP;
    spNode->value = text.substr(1, text.length() - 2);
    return spNode;
}

std::shared_ptr<ZfxASTNode> ZfxExecute::makeZfxVarNode(std::string text, operatorVals op) {
    std::shared_ptr<ZfxASTNode> spNode = std::make_shared<ZfxASTNode>();
    spNode->type = ZENVAR;
    spNode->opVal = op;
    spNode->value = text;
    return spNode;
}

std::shared_ptr<ZfxASTNode> ZfxExecute::makeZfxVarNode(std::shared_ptr<ZfxASTNode> func_call) {
    std::shared_ptr<ZfxASTNode> spNode = std::make_shared<ZfxASTNode>();
    spNode->type = ZENVAR;
    spNode->value = "";
    spNode->children.push_back(func_call);
    return spNode;
}

void ZfxExecute::markZfxAttr(std::shared_ptr<ZfxASTNode> pVarNode) {
    pVarNode->bAttr = true;
    std::string& varname = std::get<std::string>(pVarNode->value);
    if (varname.empty()) {
        return;
    }
    //属性变量必须要带上@。
    if (varname.at(0) != '@') {
        varname.insert(varname.begin(), '@');
    }
}

std::shared_ptr<ZfxASTNode> ZfxExecute::makeTypeNode(std::string text, bool bArray) {
    std::shared_ptr<ZfxASTNode> spNode = std::make_shared<ZfxASTNode>();
    spNode->type = VARIABLETYPE;
    spNode->value = text;
    if (text == "int" || text == "bool") {
        spNode->opVal = bArray ? TYPE_INT_ARR : TYPE_INT;
    }
    else if (text == "float") {
        spNode->opVal = bArray ? TYPE_FLOAT_ARR : TYPE_FLOAT;
    }
    else if (text == "string") {
        spNode->opVal = bArray ? TYPE_STRING_ARR : TYPE_STRING;
    }
    else if (text == "vector2") {
        spNode->opVal = bArray ? TYPE_VECTOR2_ARR : TYPE_VECTOR2;
    }
    else if (text == "vector3" || text == "vec3" || text == "vector") {
        spNode->opVal = bArray ? TYPE_VECTOR3_ARR : TYPE_VECTOR3;
    }
    else if (text == "vector4") {
        spNode->opVal = bArray ? TYPE_VECTOR4_ARR : TYPE_VECTOR4;
    }
    else if (text == "matrix2") {
        spNode->opVal = TYPE_MATRIX2;
    }
    else if (text == "matrix3") {
        spNode->opVal = TYPE_MATRIX3;
    }
    else if (text == "matrix4") {
        spNode->opVal = TYPE_MATRIX4;
    }
    return spNode;
}

std::shared_ptr<ZfxASTNode> ZfxExecute::makeComponentVisit(std::shared_ptr<ZfxASTNode> pExpression, std::string component) {
    if (pExpression->type == ZENVAR && pExpression->bAttr) {
        std::shared_ptr<ZfxASTNode> childNode = std::make_shared<ZfxASTNode>();
        childNode->value = component;
        pExpression->opVal = COMPVISIT;
        pExpression->children.push_back(childNode);
        return pExpression;
    }

    std::shared_ptr<ZfxASTNode> childNode = std::make_shared<ZfxASTNode>();
    childNode->type = ATTR_VAR;
    childNode->value = component;

    std::shared_ptr<ZfxASTNode> visitNode = std::make_shared<ZfxASTNode>();
    visitNode->type = ATTR_VISIT;
    visitNode->children.push_back(pExpression);
    visitNode->children.push_back(childNode);
    return visitNode;
}

std::shared_ptr<ZfxASTNode> ZfxExecute::makeQuoteStringNode(std::string text) {
    std::shared_ptr<ZfxASTNode> spNode = std::make_shared<ZfxASTNode>();
    spNode->type = STRING;
    spNode->opVal = UNDEFINE_OP;
    spNode->value = text.substr(1);
    return spNode;
}

void ZfxExecute::setASTResult(std::shared_ptr<ZfxASTNode> pNode) {
    m_root = pNode;
}

ZENO_API void ZfxExecute::printSyntaxTree()
{
    std::string printContent = "\noriginal code: " + m_code + '\n';
    if (!m_root) {
        printContent += "parser failed";
    }
    else {
        print_syntax_tree(m_root, 0, printContent);
    }
    zeno::log_info(printContent);
}

std::shared_ptr<ZfxASTNode> ZfxExecute::makeEmptyNode() {
    std::shared_ptr<ZfxASTNode> n = std::make_shared<ZfxASTNode>();
    if (!n)
    {
        exit(0);
    }
    n->type = PLACEHOLDER;
    n->value = 0;
    return n;
}

ZENO_API std::shared_ptr<ZfxASTNode> ZfxExecute::getASTResult() const {
    return m_root;
}

unsigned int ZfxExecute::location() const {
    return m_location;
}

void ZfxExecute::increaseLocation(unsigned int loc, char* txt) {
    m_location += loc;
}

}