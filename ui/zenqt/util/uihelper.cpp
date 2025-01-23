#include "uihelper.h"
#include <zeno/utils/logger.h>
#include "uicommon.h"
#include <zeno/core/data.h>
#include "zassert.h"
#include "model/curvemodel.h"
#include "variantptr.h"
#include "jsonhelper.h"
#include "model/graphmodel.h"
#include "util/curveutil.h"
#include "model/parammodel.h"
#include <QUuid>
#include <zeno/funcs/ParseObjectFromUi.h>
#include "util/globalcontrolmgr.h"
#include "zenoapplication.h"
#include "zenomainwindow.h"
#include "model/graphsmanager.h"
#include "model/assetsmodel.h"
#include <zeno/utils/helper.h>
#include "zeno_types/reflect/reflection.generated.hpp"
#include <regex>
#include "declmetatype.h"
#include <unordered_map>


const char* g_setKey = "setKey";

using namespace zenoio::iotags::curve;

VarToggleScope::VarToggleScope(bool* pbVar)
    : m_pbVar(pbVar)
{
    Q_ASSERT(m_pbVar);
    *m_pbVar = true;
}

VarToggleScope::~VarToggleScope()
{
    *m_pbVar = false;
}


BlockSignalScope::BlockSignalScope(QObject* pObj)
    : m_pObject(pObj)
{
    if (m_pObject)
        m_pObject->blockSignals(true);
}

BlockSignalScope::~BlockSignalScope()
{
	if (m_pObject)
		m_pObject->blockSignals(false);
}

QString UiHelper::createNewNode(GraphModel* subgraph, const QString& descName, const QPointF& pt)
{
    if (!subgraph)
        return "";

    zeno::NodeData node;
    //NODE_DATA node = newNodeData(pModel, descName, pt);
    node = subgraph->createNode(descName, "", pt);
    return QString::fromStdString(node.name);
}

QVariant UiHelper::parseTextValue(const zeno::ParamType& type, const QString& textValue)
{
    //TODO
    return QVariant();
}

zeno::reflect::Any UiHelper::qvarToAnyByType(const QVariant& var, const zeno::ParamType type, bool is_prim_var)
{
    QVariant::Type varType = var.type();
    bool bok = false;
    switch (type)
    {
    case gParamType_Float: {
        if (varType == QVariant::Double || varType == QMetaType::Float) {
            float fVal = var.toFloat(&bok);
            if (!bok)
                return zeno::reflect::Any();
            if (is_prim_var)
                return zeno::PrimVar(fVal);
            else
                return fVal;
        }
        else if (varType == QVariant::Int) {
            float fVal = var.toInt(&bok);
            if (!bok)
                return zeno::reflect::Any();
            if (is_prim_var)
                return zeno::PrimVar(fVal);
            else
                return fVal;
        }
        else if (varType == QVariant::String) {
            QString sVal = var.toString();
            float fVal = sVal.toFloat(&bok);
            if (bok) {
                if (is_prim_var)
                    return zeno::PrimVar(fVal);
                else
                    return fVal;
            }
            else {
                if (is_prim_var)
                    return zeno::PrimVar(sVal.toStdString());
                else
                    return sVal.toStdString();
            }
        }
        else {
            return zeno::reflect::Any();
        }
        break;
    }
    case gParamType_Int: {
        if (varType == QVariant::Double || varType == QMetaType::Float) {
            int iVal = var.toFloat(&bok);
            if (!bok)
                return zeno::reflect::Any();
            if (is_prim_var)
                return zeno::PrimVar(iVal);
            else
                return iVal;
        }
        else if (varType == QVariant::Int) {
            int iVal = var.toInt(&bok);
            if (!bok)
                return zeno::reflect::Any();
            if (is_prim_var)
                return zeno::PrimVar(iVal);
            else
                return iVal;
        }
        else if (varType == QVariant::String) {
            QString sVal = var.toString();
            int iVal = sVal.toInt(&bok);
            if (bok) {
                if (is_prim_var) {
                    return zeno::PrimVar(iVal);
                }
                else {
                    return iVal;
                }
            }
            else {
                //再试试能不能转float
                iVal = sVal.toFloat(&bok);
                if (bok) {
                    if (is_prim_var)
                        return zeno::PrimVar(iVal);
                    else
                        return iVal;
                }
                else {
                    if (is_prim_var)
                        return zeno::PrimVar(sVal.toStdString());
                    else
                        return sVal.toStdString();
                }
            }
        }
        else {
            return zeno::reflect::Any();
        }
        break;
    }
    case gParamType_String: {
        if (varType == QVariant::String) {
            QString sVal = var.toString();
            if (is_prim_var) {
                return zeno::PrimVar(sVal.toStdString());
            }
            else {
                return sVal.toStdString();
            }
        }
        break;
    }
    case gParamType_Bool: {
        if (varType == QVariant::Bool) {
            bool bVal = var.toBool();
            return bVal;
        }
        else if (varType == QVariant::Int) {
            bool bVal = var.toInt();
            return bVal;
        }
        break;
    }
    case gParamType_Vec2i:
    case gParamType_Vec2f:
    case gParamType_Vec3i:
    case gParamType_Vec3f:
    case gParamType_Vec4i:
    case gParamType_Vec4f:
    {
        bool bFloat = (gParamType_Vec2f == type || gParamType_Vec3f == type || gParamType_Vec4f == type);
        int nSize = 0;
        if (gParamType_Vec2i == type || gParamType_Vec2f == type) {
            nSize = 2;
        }
        else if (gParamType_Vec3i == type || gParamType_Vec3f == type) {
            nSize = 3;
        }
        else if (gParamType_Vec4i == type || gParamType_Vec4f == type) {
            nSize = 4;
        }

        if (var.userType() == QMetaTypeId<UI_VECTYPE>::qt_metatype_id())
        {
            UI_VECTYPE qvec = var.value<UI_VECTYPE>();
            //默认都是转为VecEdit
            if (qvec.size() == nSize) {
                zeno::vecvar vec(nSize);
                for (int i = 0; i < nSize; i++) {
                    if (bFloat) {
                        vec[i] = zeno::PrimVar((float)qvec[i]);
                    }
                    else {
                        vec[i] = zeno::PrimVar((int)qvec[i]);
                    }
                }
                return vec;
            }
        }
        else if (varType == QVariant::StringList)
        {
            QStringList qvec = var.toStringList();
            if (qvec.size() == nSize) {
                zeno::vecvar vec(nSize);
                for (int i = 0; i < nSize; i++) {
                    const auto& anyPrimvar = qvarToAnyByType(qvec[i], bFloat ? gParamType_Float : gParamType_Int, true);
                    if (anyPrimvar.type().hash_code() == zeno::types::gParamType_PrimVariant)
                    {
                        const auto& primvar = zeno::reflect::any_cast<zeno::PrimVar>(anyPrimvar);
                        vec[i] = primvar;
                    }
                }
                return vec;
            }
        }
        else if (varType == QVariant::UserType)
        {
            int usrType = var.userType();
            if (usrType == QMetaTypeId<UI_VECSTRING>::qt_metatype_id())
            {
                //TODO:
            }
            if (usrType == QMetaTypeId<QJSValue>::qt_metatype_id())
            {
                const QJSValue& jsval = var.value<QJSValue>();
                if (jsval.isArray())
                {
                    QVariant var2 = jsval.toVariant();
                    if (var2.canConvert<QVariantList>())
                    {
                        const QVariantList& lst = var2.toList();
                        if (lst.size() == nSize) {
                            zeno::vecvar vec(nSize);
                            for (int i = 0; i < nSize; i++) {
                                const auto& anyPrimvar = qvarToAnyByType(lst[i], bFloat ? gParamType_Float : gParamType_Int, true);
                                if (anyPrimvar.type().hash_code() == zeno::types::gParamType_PrimVariant)
                                {
                                    const auto& primvar =zeno::reflect::any_cast<zeno::PrimVar>(anyPrimvar);
                                    vec[i] = primvar;
                                }
                            }
                            return vec;
                        }
                    }
                }
            }
        }
        break;
    }
    case gParamType_List: {

    }
    case gParamType_Dict: {

    }
    case gParamType_Heatmap: {

    }
    case gParamType_IObject: {

    }
    case gParamType_Primitive: {

    }
    case Param_Null:
    default:
        return zeno::reflect::Any();
    }
    return zeno::reflect::Any();
}

zeno::reflect::Any UiHelper::qvarToAny(const QVariant& var, const zeno::ParamType type, bool is_prim_var)
{
    if (var.type() == QVariant::String)
    {
        auto val = var.toString().toStdString();
        if (is_prim_var) {
            return zeno::PrimVar(val);
        }
        else {
            return val;
        }
    }
    else if (var.type() == QVariant::Double || var.type() == QMetaType::Float)
    {
        auto val = var.toFloat();
        if (is_prim_var) {
            return zeno::PrimVar(val);
        }
        else {
            return val;
        }
    }
    else if (var.type() == QVariant::Int)
    {
        auto val = var.toInt();
        if (is_prim_var) {
            return zeno::PrimVar(val);
        }
        else {
            return val;
        }
    }
    else if (var.type() == QVariant::Bool)
    {
        return var.toBool() ? 1 : 0;
    }
    else if (var.type() == QVariant::Invalid)
    {
        return zeno::reflect::Any();
    }
    else if (var.type() == QVariant::UserType)
    {
        if (var.userType() == QMetaTypeId<zeno::reflect::Any>::qt_metatype_id())
        {
            return var.value<zeno::reflect::Any>();
        }
        else if (var.userType() == QMetaTypeId<UI_VECTYPE>::qt_metatype_id())
        {
            UI_VECTYPE vec = var.value<UI_VECTYPE>();
            if (vec.isEmpty()) {
                zeno::log_warn("unexpected qt variant {}", var.typeName());
                return zeno::reflect::Any();
            }
            else {
                if (vec.size() == 2) {
                    if (type == zeno::types::gParamType_Vec2i) {
                        return zeno::vec2i((int)vec[0], (int)vec[1]);
                    }
                    else {
                        return zeno::vec2f(vec[0], vec[1]);
                    }
                }
                if (vec.size() == 3) {
                    if (type == zeno::types::gParamType_Vec3i) {
                        return zeno::vec3i((int)vec[0], (int)vec[1], (int)vec[2]);
                    }
                    else {
                        return zeno::vec3f(vec[0], vec[1], vec[2]);
                    }
                }
                if (vec.size() == 4) {
                    if (type == zeno::types::gParamType_Vec4i) {
                        return zeno::vec4i((int)vec[0], (int)vec[1], (int)vec[2], (int)vec[3]);
                    }
                    else {
                        return zeno::vec4f(vec[0], vec[1], vec[2], vec[3]);
                    }
                }
            }
        }
        else if (var.userType() == QMetaTypeId<UI_VECSTRING>::qt_metatype_id()) {
            UI_VECSTRING vec = var.value<UI_VECSTRING>();
            if (vec.size() == 2) {
                return zeno::vec2s(vec[0].toStdString(), vec[1].toStdString());
            }
            else if (vec.size() == 3) {
                return zeno::vec3s(vec[0].toStdString(), vec[1].toStdString(),
                    vec[2].toStdString());
            }
            else if (vec.size() == 4) {
                return zeno::vec4s(vec[0].toStdString(), vec[1].toStdString(),
                    vec[2].toStdString(), vec[3].toStdString());
            }
            else {
                return zeno::reflect::Any();
            }
        }
    }
    else
    {
        zeno::log_warn("bad qt variant {}", var.typeName());
    }
    return zeno::reflect::Any();
}



QVariant UiHelper::anyToQvar(zeno::reflect::Any var)
{
    if (!var.has_value()) {
        return QVariant();
    }
    if (zeno::reflect::get_type<int>() == var.type()) {
        return zeno::reflect::any_cast<int>(var);
    }
    else if (zeno::reflect::get_type<float>() == var.type()) {
        return zeno::reflect::any_cast<float>(var);
    }
    else if (zeno::reflect::get_type<std::string>() == var.type()) {
        return QString::fromStdString(zeno::reflect::any_cast<std::string>(var));
    }
    else if (zeno::reflect::get_type<zeno::vec2i>() == var.type()) {
        zeno::vec2i vec2i = zeno::reflect::any_cast<zeno::vec2i>(var);
        UI_VECTYPE vec;
        vec.push_back(vec2i[0]);
        vec.push_back(vec2i[1]);
        return QVariant::fromValue(vec);
    }
    else if (zeno::reflect::get_type<zeno::vec3i>() == var.type()) {
        zeno::vec3i vec3i = zeno::reflect::any_cast<zeno::vec3i>(var);
        UI_VECTYPE vec;
        vec.push_back(vec3i[0]);
        vec.push_back(vec3i[1]);
        vec.push_back(vec3i[2]);
        return QVariant::fromValue(vec);
    }
    else if (zeno::reflect::get_type<zeno::vec4i>() == var.type()) {
        zeno::vec4i vec4i = zeno::reflect::any_cast<zeno::vec4i>(var);
        UI_VECTYPE vec;
        vec.push_back(vec4i[0]);
        vec.push_back(vec4i[1]);
        vec.push_back(vec4i[2]);
        vec.push_back(vec4i[3]);
        return QVariant::fromValue(vec);
    }
    else if (zeno::reflect::get_type<zeno::vec2f>() == var.type()) {
        zeno::vec2f vec2f = zeno::reflect::any_cast<zeno::vec2f>(var);
        UI_VECTYPE vec;
        vec.push_back(vec2f[0]);
        vec.push_back(vec2f[1]);
        return QVariant::fromValue(vec);
    }
    else if (zeno::reflect::get_type<zeno::vec3f>() == var.type()) {
        zeno::vec3f vec3f = zeno::reflect::any_cast<zeno::vec3f>(var);
        UI_VECTYPE vec;
        vec.push_back(vec3f[0]);
        vec.push_back(vec3f[1]);
        vec.push_back(vec3f[2]);
        return QVariant::fromValue(vec);
    }
    else if (zeno::reflect::get_type<zeno::vec4f>() == var.type()) {
        zeno::vec4f vec4f = zeno::reflect::any_cast<zeno::vec4f>(var);
        UI_VECTYPE vec;
        vec.push_back(vec4f[0]);
        vec.push_back(vec4f[1]);
        vec.push_back(vec4f[2]);
        vec.push_back(vec4f[3]);
        return QVariant::fromValue(vec);
    }
    else if (zeno::reflect::get_type<zeno::vec2s>() == var.type()) {
        zeno::vec2s vec2s = zeno::reflect::any_cast<zeno::vec2s>(var);
        UI_VECSTRING vec;
        vec.push_back(QString::fromStdString(vec2s[0]));
        vec.push_back(QString::fromStdString(vec2s[1]));
        return QVariant::fromValue(vec);
    }
    else if (zeno::reflect::get_type<zeno::vec3s>() == var.type()) {
        zeno::vec3s vec3s = zeno::reflect::any_cast<zeno::vec3s>(var);
        UI_VECSTRING vec;
        vec.push_back(QString::fromStdString(vec3s[0]));
        vec.push_back(QString::fromStdString(vec3s[1]));
        vec.push_back(QString::fromStdString(vec3s[2]));
        return QVariant::fromValue(vec);
    }
    else if (zeno::reflect::get_type<zeno::vec4s>() == var.type()) {
        zeno::vec4s vec4s = zeno::reflect::any_cast<zeno::vec4s>(var);
        UI_VECSTRING vec;
        vec.push_back(QString::fromStdString(vec4s[0]));
        vec.push_back(QString::fromStdString(vec4s[1]));
        vec.push_back(QString::fromStdString(vec4s[2]));
        vec.push_back(QString::fromStdString(vec4s[3]));
        return QVariant::fromValue(vec);
    }
    return QVariant();
}

zeno::zvariant UiHelper::qvarToZVar(const QVariant& var, const zeno::ParamType type)
{
    if (var.type() == QVariant::String)
    {
        return var.toString().toStdString();
    }
    else if (var.type() == QVariant::Double || var.type() == QMetaType::Float)
    {
        return var.toFloat();
    }
    else if (var.type() == QVariant::Int)
    {
        return var.toInt();
    }
    else if (var.type() == QVariant::Bool)
    {
        return var.toBool();
    }
    else if (var.type() == QVariant::Invalid)
    {
        return zeno::zvariant();
    }
    else if (var.type() == QVariant::UserType)
    {
        if (var.userType() == QMetaTypeId<UI_VECTYPE>::qt_metatype_id())
        {
            UI_VECTYPE vec = var.value<UI_VECTYPE>();
            if (vec.isEmpty()) {
                zeno::log_warn("unexpected qt variant {}", var.typeName());
                return zeno::zvariant();
            }
            else {
                if (vec.size() == 2) {
                    if (type == zeno::types::gParamType_Vec2f) {
                        return zeno::vec2f(vec[0], vec[1]);
                    }
                    else if (type == zeno::types::gParamType_Vec2i) {
                        return zeno::vec2i((int)vec[0], (int)vec[1]);
                    }
                }
                if (vec.size() == 3) {
                    if (type == zeno::types::gParamType_Vec3f) {
                        return zeno::vec3f(vec[0], vec[1], vec[2]);
                    }
                    else if (type == zeno::types::gParamType_Vec3i) {
                        return zeno::vec3i((int)vec[0], (int)vec[1], (int)vec[2]);
                    }
                }
                if (vec.size() == 4) {
                    if (type == zeno::types::gParamType_Vec4f) {
                        return zeno::vec4f(vec[0], vec[1], vec[2], vec[3]);
                    }
                    else if (type == zeno::types::gParamType_Vec4i) {
                        return zeno::vec4i((int)vec[0], (int)vec[1], (int)vec[2], (int)vec[3]);
                    }
                }
            }
        }
        else if (var.userType() == QMetaTypeId<UI_VECSTRING>::qt_metatype_id()) {
            UI_VECSTRING vec = var.value<UI_VECSTRING>();
            if (vec.size() == 2) {
                return zeno::vec2s(vec[0].toStdString(), vec[1].toStdString());
            }
            if (vec.size() == 3) {
                return zeno::vec3s(vec[0].toStdString(), vec[1].toStdString(),
                    vec[2].toStdString());
            }
            if (vec.size() == 4) {
                return zeno::vec4s(vec[0].toStdString(), vec[1].toStdString(),
                    vec[2].toStdString(), vec[3].toStdString());
            }
        }
    }
    else
    {
        zeno::log_warn("bad qt variant {}", var.typeName());
    }
    return zeno::zvariant();
}

QVariant UiHelper::zvarToQVar(const zeno::zvariant& var)
{
    QVariant qVar;
    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int>) {
            qVar = arg;
        }
        else if constexpr (std::is_same_v<T, float>) {
            qVar = arg;
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            qVar = QString::fromStdString(arg);
        }
        else if constexpr (std::is_same_v<T, zeno::vec2i> || std::is_same_v<T, zeno::vec2f>)
        {
            UI_VECTYPE vec;
            vec.push_back(arg[0]);
            vec.push_back(arg[1]);
            qVar = QVariant::fromValue(vec);
        }
        else if constexpr (std::is_same_v<T, zeno::vec3i> || std::is_same_v<T, zeno::vec3f>)
        {
            UI_VECTYPE vec;
            vec.push_back(arg[0]);
            vec.push_back(arg[1]);
            vec.push_back(arg[2]);
            qVar = QVariant::fromValue(vec);
        }
        else if constexpr (std::is_same_v<T, zeno::vec4i> || std::is_same_v<T, zeno::vec4f>)
        {
            UI_VECTYPE vec;
            vec.push_back(arg[0]);
            vec.push_back(arg[1]);
            vec.push_back(arg[2]);
            vec.push_back(arg[3]);
            qVar = QVariant::fromValue(vec);
        }
        else if constexpr (std::is_same_v<T, zeno::vec2s>)
        {
            UI_VECSTRING vec;
            vec.push_back(QString::fromStdString(arg[0]));
            vec.push_back(QString::fromStdString(arg[1]));
            qVar = QVariant::fromValue(vec);
        }
        else if constexpr (std::is_same_v<T, zeno::vec3s>)
        {
            UI_VECSTRING vec;
            vec.push_back(QString::fromStdString(arg[0]));
            vec.push_back(QString::fromStdString(arg[1]));
            vec.push_back(QString::fromStdString(arg[2]));
            qVar = QVariant::fromValue(vec);
        }
        else if constexpr (std::is_same_v<T, zeno::vec4s>)
        {
            UI_VECSTRING vec;
            vec.push_back(QString::fromStdString(arg[0]));
            vec.push_back(QString::fromStdString(arg[1]));
            vec.push_back(QString::fromStdString(arg[2]));
            vec.push_back(QString::fromStdString(arg[3]));
            qVar = QVariant::fromValue(vec);
        }
        else 
        {
            //TODO
        }
    }, var);
    return qVar;
}

QVariant UiHelper::initDefaultValue(const zeno::ParamType& type)
{
    if (type == zeno::types::gParamType_String) {
        return "";
    }
    else if (type == zeno::types::gParamType_Float)
    {
        return QVariant((float)0.);
    }
    else if (type == zeno::types::gParamType_Int)
    {
        return QVariant((int)0);
    }
    else if (type == zeno::types::gParamType_Bool)
    {
        return QVariant(false);
    }
    else if (type == zeno::types::gParamType_Vec2i || type == zeno::types::gParamType_Vec2f)
    {
        UI_VECTYPE vec(2);
        return QVariant::fromValue(vec);
    }
    else if (type == zeno::types::gParamType_Vec3i || type == zeno::types::gParamType_Vec3f)
    {
        UI_VECTYPE vec(3);
        return QVariant::fromValue(vec);
    }
    else if (type == zeno::types::gParamType_Vec4i || type == zeno::types::gParamType_Vec4f)
    {
        UI_VECTYPE vec(4);
        return QVariant::fromValue(vec);
    }
    else if (type == zeno::types::gParamType_Curve)
    {
        zeno::CurvesData curves = curve_util::deflCurves();
        auto& anyVal = zeno::reflect::make_any<zeno::CurvesData>(curves);
        return QVariant::fromValue(anyVal);
    }
    else if (type == zeno::types::gParamType_Heatmap)
    {
        return JsonHelper::dumpHeatmap(1024, "");
    }

    /*
    else if (type.startsWith("vec"))
    {
        int dim = 0;
        bool bFloat = false;
        if (UiHelper::parseVecType(type, dim, bFloat))
        {
            return QVariant::fromValue(UI_VECTYPE(dim, 0));
        }
    }
    */
    return QVariant();
}

QSizeF UiHelper::viewItemTextLayout(QTextLayout& textLayout, int lineWidth, int maxHeight, int* lastVisibleLine)
{
	if (lastVisibleLine)
		*lastVisibleLine = -1;
	qreal height = 0;
	qreal widthUsed = 0;
	textLayout.beginLayout();
	int i = 0;
	while (true) {
		QTextLine line = textLayout.createLine();
		if (!line.isValid())
			break;
		line.setLineWidth(lineWidth);
		line.setPosition(QPointF(0, height));
		height += line.height();
		widthUsed = qMax(widthUsed, line.naturalTextWidth());
		// we assume that the height of the next line is the same as the current one
		if (maxHeight > 0 && lastVisibleLine && height + line.height() > maxHeight) {
			const QTextLine nextLine = textLayout.createLine();
			*lastVisibleLine = nextLine.isValid() ? i : -1;
			break;
		}
		++i;
	}
	textLayout.endLayout();
	return QSizeF(widthUsed, height);
}

QString UiHelper::generateUuid(const QString& name)
{
    QUuid uuid = QUuid::createUuid();
    return QString::number(uuid.data1, 16) + "-" + name;
}

uint UiHelper::generateUuidInt()
{
    QUuid uuid = QUuid::createUuid();
    return uuid.data1;
}

bool UiHelper::parseVecType(const QString& type, int& dim, bool& bFloat)
{
    static QRegExp rx("vec(2|3|4)(i|f)?");
    bool ret = rx.exactMatch(type);
    if (!ret) return false;

    rx.indexIn(type);
    QStringList list = rx.capturedTexts();
    if (list.length() == 3)
    {
        dim = list[1].toInt();
        bFloat = list[2] != 'i';
        return true;
    }
    else
    {
        return false;
    }
}

#if 0
QString UiHelper::getControlDesc(zeno::ParamControl ctrl)
{
    switch (ctrl)
    {
    case CONTROL_INT:               return "Integer";
    case CONTROL_FLOAT:             return "Float";
    case CONTROL_STRING:            return "String";
    case CONTROL_BOOL:              return "Boolean";
    case CONTROL_MULTILINE_STRING:  return "Multiline String";
    case CONTROL_READPATH:          return "read path";
    case CONTROL_WRITEPATH:         return "write path";
    case CONTROL_ENUM:              return "Enum";
    case CONTROL_VEC4_FLOAT:        return "Float Vector 4";
    case CONTROL_VEC3_FLOAT:        return "Float Vector 3";
    case CONTROL_VEC2_FLOAT:        return "Float Vector 2";
    case CONTROL_VEC4_INT:          return "Integer Vector 4";
    case CONTROL_VEC3_INT:          return "Integer Vector 3";
    case CONTROL_VEC2_INT:          return "Integer Vector 2";
    case CONTROL_COLOR:             return "Color";
    case CONTROL_COLOR_VEC3F:       return "Color Vec3f";
    case CONTROL_CURVE:             return "Curve";
    case CONTROL_HSPINBOX:          return "SpinBox";
    case CONTROL_HDOUBLESPINBOX: return "DoubleSpinBox";
    case CONTROL_HSLIDER:           return "Slider";
    case CONTROL_SPINBOX_SLIDER:    return "SpinBoxSlider";
    case CONTROL_DICTPANEL:         return "Dict Panel";
    case CONTROL_GROUP_LINE:             return "group-line";
    case CONTROL_PYTHON_EDITOR: return "PythonEditor";
    default:
        return "";
    }
}
#endif

QString UiHelper::getControlDesc(zeno::ParamControl ctrl, zeno::ParamType type)
{
    switch (ctrl)
    {
    case zeno::Lineedit:
    {
        switch (type) {
        case zeno::types::gParamType_Float:   return "Float";
        case zeno::types::gParamType_Int:     return "Integer";
        case zeno::types::gParamType_String:  return "String";
        }
        return "";
    }
    case zeno::Checkbox:
    {
        return "Boolean";
    }
    case zeno::Multiline:           return "Multiline String";
    case zeno::ReadPathEdit:            return "read path";
    case zeno::WritePathEdit:            return "write path";
    case zeno::DirectoryPathEdit:            return "directory";
    case zeno::Combobox:            return "Enum";
    case zeno::Vec4edit:
    {
        return type == zeno::types::gParamType_Int ? "Integer Vector 4" : "Float Vector 4";
    }
    case zeno::Vec3edit:
    {
        return type == zeno::types::gParamType_Int ? "Integer Vector 3" : "Float Vector 3";
    }
    case zeno::Vec2edit:
    {
        return type == zeno::types::gParamType_Int ? "Integer Vector 2" : "Float Vector 2";
    }
    case zeno::Heatmap:             return "Color";
    case zeno::ColorVec:            return "Color Vec3f";
    case zeno::CurveEditor:         return "Curve";
    case zeno::SpinBox:             return "SpinBox";
    case zeno::DoubleSpinBox:       return "DoubleSpinBox";
    case zeno::Slider:              return "Slider";
    case zeno::SpinBoxSlider:       return "SpinBoxSlider";
    case zeno::Seperator:           return "group-line";
    case zeno::PythonEditor:        return "PythonEditor";
    case zeno::CodeEditor:        return "CodeEditor";
    default:
        return "";
    }
}

zeno::ParamControl UiHelper::getControlByDesc(const QString& descName)
{
    //compatible with zsg2
    if (descName == "Integer")
    {
        return zeno::Lineedit;
    }
    else if (descName == "Float")
    {
        return zeno::Lineedit;
    }
    else if (descName == "String")
    {
        return zeno::Lineedit;
    }
    else if (descName == "Boolean")
    {
        return zeno::Checkbox;
    }
    else if (descName == "Multiline String")
    {
        return zeno::Multiline;
    }
    else if (descName == "read path")
    {
        return zeno::ReadPathEdit;
    }
    else if (descName == "write path")
    {
        return zeno::WritePathEdit;
    }
    else if (descName == "directory")
    {
        return zeno::DirectoryPathEdit;
    }
    else if (descName == "Enum")
    {
        return zeno::Combobox;
    }
    else if (descName == "Float Vector 4")
    {
        return zeno::Vec4edit;
    }
    else if (descName == "Float Vector 3")
    {
        return zeno::Vec3edit;
    }
    else if (descName == "Float Vector 2")
    {
        return zeno::Vec2edit;
    }
    else if (descName == "Integer Vector 4")
    {
        return zeno::Vec4edit;
    }
    else if (descName == "Integer Vector 3")
    {
        return zeno::Vec3edit;
    }
    else if (descName == "Integer Vector 2")
    {
        return zeno::Vec2edit;
    }
    else if (descName == "Color")
    {
        return zeno::Heatmap;
    } 
    else if (descName == "Color Vec3f")
    {
        return zeno::ColorVec;
    }
    else if (descName == "Curve")
    {
        return zeno::CurveEditor;
    }
    else if (descName == "SpinBox")
    {
        return zeno::SpinBox;
    } 
    else if (descName == "DoubleSpinBox") 
    {
        return zeno::DoubleSpinBox;
    }
    else if (descName == "Slider")
    {
        return zeno::Slider;
    }
    else if (descName == "SpinBoxSlider")
    {
        return zeno::SpinBoxSlider;
    }
    else if (descName == "Dict Panel")
    {
        return zeno::NullControl;
    }
    else if (descName == "group-line")
    {
        return zeno::NullControl;
    }
    else if (descName == "PythonEditor")
    {
        return zeno::PythonEditor;
    }
    else if (descName == "CodeEditor")
    {
        return zeno::CodeEditor;
    }
    else
    {
        return zeno::NullControl;
    }
}

bool UiHelper::isFloatType(zeno::ParamType type)
{
    return type == zeno::types::gParamType_Float || type == zeno::types::gParamType_Vec2f || type == zeno::types::gParamType_Vec3f || type == zeno::types::gParamType_Vec4f;
}

bool UiHelper::qIndexSetData(const QModelIndex& index, const QVariant& value, int role)
{
    QAbstractItemModel* pModel = const_cast<QAbstractItemModel*>(index.model());
    if (!pModel)
        return false;
    return pModel->setData(index, value, role);
}

QStringList UiHelper::getCoreTypeList()
{
    static QStringList types = {
        "",
        "int",
        "bool",
        "float",
        "string",
        "vec2f",
        "vec2i",
        "vec3f",
        "vec3i",
        "vec4f",
        "vec4i",
        //"writepath",
        //"readpath",
        "color",
        "curve",
        "list",
        "dict"
    };
    return types;
}

QStringList UiHelper::getAllControls()
{
    return { "Integer", "Float", "String", "Boolean", "Multiline String", "read path", "write path", "Enum",
        "Float Vector 4", "Float Vector 3", "Float Vector 2","Integer Vector 4", "Integer Vector 3",
        "Integer Vector 2", "Color", "Curve", "SpinBox", "DoubleSpinBox", "Slider", "SpinBoxSlider" };
}

QString UiHelper::getTypeDesc(zeno::ParamType type)
{
    //���������ͨ����Щhash�룬�õ���Ӧ��type_info����type_info���¼��
    //����metadata��������1.�Ƿ�Ϊobject 2.���Ƽ�� ������Դ����������Ȼ�ķ�ʽ�ˡ�
    switch (type)
    {
    case zeno::types::gParamType_String:    return "string";
    case zeno::types::gParamType_Bool:      return "bool";
    case zeno::types::gParamType_Int:       return "int";
    case zeno::types::gParamType_Float:     return "float";
    case zeno::types::gParamType_Vec2i:     return "vec2i";
    case zeno::types::gParamType_Vec3i:     return "vec3i";
    case zeno::types::gParamType_Vec4i:     return "vec4i";
    case zeno::types::gParamType_Vec2f:     return "vec2f";
    case zeno::types::gParamType_Vec3f:     return "vec3f";
    case zeno::types::gParamType_Vec4f:     return "vec4f";
    case gParamType_List:      return "list";
    case gParamType_Dict:      return "dict";
    case zeno::types::gParamType_Heatmap:       return "color";
    case gParamType_IObject: return "object";
    case gParamType_Primitive:      return "prim";
    case Param_Null:
    default:
        return "";
    }
}

zeno::ParamControl UiHelper::getControlByType(const QString &type)
{
    if (type.isEmpty()) {
        return zeno::NullControl;
    } else if (type == "int") {
        return zeno::Lineedit;
    } else if (type == "bool") {
        return zeno::Checkbox;
    } else if (type == "float") {
        return zeno::Lineedit;
    } else if (type == "string") {
        return zeno::Lineedit;
    } else if (type.startsWith("vec")) {
        // support legacy type "vec3"
        int dim = 0;
        bool bFloat = false;
        if (parseVecType(type, dim, bFloat)) {
            switch (dim)
            {
            case 2: return zeno::Vec2edit;
            case 3: return zeno::Vec3edit;
            case 4: return zeno::Vec4edit;
            default:
                return zeno::NullControl;
            }
        }
        else {
            return zeno::NullControl;
        }
    } else if (type == "writepath") {
        return zeno::WritePathEdit;
    } else if (type == "readpath") {
        return zeno::ReadPathEdit;
    } else if (type == "directory") {
        return zeno::DirectoryPathEdit;
    }
    else if (type == "multiline_string") {
        return zeno::Multiline;
    } else if (type == "color") {   //color is more general than heatmap.
        return zeno::Heatmap;
    } else if (type == "colorvec3f") {   //colorvec3f is for coloreditor, color is heatmap? ^^^^
        return zeno::ColorVec;
    } else if (type == "curve") {
        return zeno::CurveEditor;
    } else if (type.startsWith("enum ")) {
        return zeno::Combobox;
    } else if (type == "NumericObject") {
        return zeno::Lineedit;
    } else if (type.isEmpty()) {
        return zeno::NullControl;
    }
    else if (type == "dict")
    {
        //control by multilink socket property. see SOCKET_PROPERTY
        return zeno::NullControl;
    } else if (type == "group-line") {
        return zeno::NullControl;
    }
    else {
        zeno::log_trace("parse got undefined control type {}", type.toStdString());
        return zeno::NullControl;
    }
}

CONTROL_INFO UiHelper::getControlByType(const QString &nodeCls, bool bInput, const QString &socketName, const QString &socketType)
{
    return GlobalControlMgr::instance().controlInfo(nodeCls, bInput, socketName, socketType);
}

void UiHelper::getSocketInfo(const QString& objPath,
                             QString& subgName,
                             QString& nodeIdent,
                             QString& paramPath)
{
    //see GraphsModel::indexFromPath
    QStringList lst = objPath.split(cPathSeperator, QtSkipEmptyParts);
    //format like: [subgraph-name]:[node-ident]:[node|panel]/[param-layer-path]/[dict-key]
    //example: main:xxxxx-wrangle:[node]inputs/params/key1
    if (lst.size() >= 3)
    {
        subgName = lst[0];
        nodeIdent = lst[1];
        paramPath = lst[2];
    }
}

QString UiHelper::constructObjPath(const QString& subgraph, const QString& node, const QString& group, const QString& sockName)
{
    QStringList seq = {subgraph, node, group + sockName};
    return seq.join(cPathSeperator);
}

QString UiHelper::constructObjPath(const QString& subgraph, const QString& node, const QString& paramPath)
{
    QStringList seq = {subgraph, node, paramPath};
    return seq.join(cPathSeperator);
}

QString UiHelper::getSockNode(const QString& sockPath)
{
    QStringList lst = sockPath.split(cPathSeperator, QtSkipEmptyParts);
    if (lst.size() > 1)
        return lst[1];
    return "";
}

QString UiHelper::getParamPath(const QString& sockPath)
{
    QStringList lst = sockPath.split(cPathSeperator, QtSkipEmptyParts);
    if (lst.size() > 2)
        return lst[2];
    return "";
}

QString UiHelper::getSockName(const QString& sockPath)
{
    QStringList lst = sockPath.split(cPathSeperator, QtSkipEmptyParts);
    if (lst.size() > 2)
    {
        lst = lst[2].split("/", QtSkipEmptyParts);
        if (!lst.isEmpty())
        {
            //format: main:xxxxx-wrangle:[node]inputs/params/key1
            if (lst.size() == 4)
            {
                return lst[2] + "/" + lst[3];
            }
            else
            {
                return lst.last();
            }
        }
    }
    return "";
}

QString UiHelper::getSockSubgraph(const QString& sockPath)
{
    QStringList lst = sockPath.split(cPathSeperator, QtSkipEmptyParts);
    if (lst.size() > 0)
        return lst[0];
    return "";
}

QString UiHelper::anyToString(const zeno::reflect::Any& any)
{
    if (!any.has_value()) {
        return "";
    }
    if (zeno::reflect::get_type<int>() == any.type()) {
        return QString::number(zeno::reflect::any_cast<int>(any));
    }
    else if (zeno::reflect::get_type<float>() == any.type()) {
        return QString::number(zeno::reflect::any_cast<float>(any));
    }
    else if (gParamType_PrimVariant == any.type().hash_code()) {
        zeno::PrimVar var = zeno::reflect::any_cast<zeno::PrimVar>(any);
        return std::visit([](auto&& val)->QString {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<int, T> || std::is_same_v<float, T>) {
                return QString::number(val);
            }
            else if constexpr (std::is_same_v<std::string, T>) {
                return QString::fromStdString(val);
            }
            else {
                return "";
            }
        }, var);
    }
    else if (zeno::reflect::get_type<std::string>() == any.type()) {
        return QString::fromStdString(zeno::reflect::any_cast<std::string>(any));
    }
    else if (zeno::reflect::get_type<zeno::vec2i>() == any.type()) {
        zeno::vec2i vec2i = zeno::reflect::any_cast<zeno::vec2i>(any);
        QString res;
        res += QString::number(vec2i[0]);
        res += ",";
        res += QString::number(vec2i[1]);
        return res;
    }
    else if (zeno::reflect::get_type<zeno::vec3i>() == any.type()) {
        zeno::vec3i vec3i = zeno::reflect::any_cast<zeno::vec3i>(any);
        QString res;
        res += QString::number(vec3i[0]);
        res += ",";
        res += QString::number(vec3i[1]);
        res += ",";
        res += QString::number(vec3i[2]);
        return res;
    }
    else if (zeno::reflect::get_type<zeno::vec4i>() == any.type()) {
        zeno::vec4i vec4i = zeno::reflect::any_cast<zeno::vec4i>(any);
        QString res;
        res += QString::number(vec4i[0]);
        res += ",";
        res += QString::number(vec4i[1]);
        res += ",";
        res += QString::number(vec4i[2]);
        res += ",";
        res += QString::number(vec4i[3]);
        return res;
    }
    else if (zeno::reflect::get_type<zeno::vec2f>() == any.type()) {
        zeno::vec2f vec2f = zeno::reflect::any_cast<zeno::vec2f>(any);
        QString res;
        res += QString::number(vec2f[0]);
        res += ",";
        res += QString::number(vec2f[1]);
        return res;
    }
    else if (zeno::reflect::get_type<zeno::vec3f>() == any.type()) {
        zeno::vec3f vec3f = zeno::reflect::any_cast<zeno::vec3f>(any);
        QString res;
        res += QString::number(vec3f[0]);
        res += ",";
        res += QString::number(vec3f[1]);
        res += ",";
        res += QString::number(vec3f[2]);
        return res;
    }
    else if (zeno::reflect::get_type<zeno::vec4f>() == any.type()) {
        zeno::vec4f vec4f = zeno::reflect::any_cast<zeno::vec4f>(any);
        QString res;
        res += QString::number(vec4f[0]);
        res += ",";
        res += QString::number(vec4f[1]);
        res += ",";
        res += QString::number(vec4f[2]);
        res += ",";
        res += QString::number(vec4f[3]);
        return res;
    }
    else if (zeno::reflect::get_type<zeno::vec2s>() == any.type()) {
        zeno::vec2s vec2s = zeno::reflect::any_cast<zeno::vec2s>(any);
        QString res;
        res += QString::fromStdString(vec2s[0]);
        res += ",";
        res += QString::fromStdString(vec2s[1]);
        return res;
    }
    else if (zeno::reflect::get_type<zeno::vec3s>() == any.type()) {
        zeno::vec3s vec3s = zeno::reflect::any_cast<zeno::vec3s>(any);
        QString res;
        res += QString::fromStdString(vec3s[0]);
        res += ",";
        res += QString::fromStdString(vec3s[1]);
        res += ",";
        res += QString::fromStdString(vec3s[2]);
        return res;
    }
    else if (zeno::reflect::get_type<zeno::vec4s>() == any.type()) {
        zeno::vec4s vec4s = zeno::reflect::any_cast<zeno::vec4s>(any);
        QString res;
        res += QString::fromStdString(vec4s[0]);
        res += ",";
        res += QString::fromStdString(vec4s[1]);
        res += ",";
        res += QString::fromStdString(vec4s[2]);
        res += ",";
        res += QString::fromStdString(vec4s[3]);
        return res;
    }
    return "";
}

QString UiHelper::variantToString(const QVariant& var)
{
	QString value;
	if (var.type() == QVariant::String)
	{
		value = var.toString();
	}
	else if (var.type() == QVariant::Double)
	{
		value = QString::number(var.toDouble());
	}
    else if (var.type() == QMetaType::Float)
    {
        value = QString::number(var.toFloat());
    }
	else if (var.type() == QVariant::Int)
	{
		value = QString::number(var.toInt());
	}
	else if (var.type() == QVariant::Bool)
	{
		value = var.toBool() ? "true" : "false";
	}
	else if (var.type() == QVariant::Invalid)
	{
		zeno::log_debug("got null qt variant");
		value = "";
	}
	else if (var.type() == QVariant::Bool)
	{
		value = var.toBool() ? "true" : "false";
	}
	else if (var.type() == QVariant::UserType)
    {
        if (var.userType() == QMetaTypeId<UI_VECTYPE>::qt_metatype_id())
        {
            UI_VECTYPE vec = var.value<UI_VECTYPE>();
            if (vec.isEmpty()) {
                zeno::log_warn("unexpected qt variant {}", var.typeName());
            }
            else {
                QString res;
                for (int i = 0; i < vec.size(); i++) {
                    res.append(QString::number(vec[i]));
                    if (i < vec.size() - 1)
                        res.append(",");
                }
                return res;
            }
        }
        else if (var.userType() == QMetaTypeId<UI_VECSTRING>::qt_metatype_id())
        {
            UI_VECSTRING vec = var.value<UI_VECSTRING>();
            QString res;
            for (int i = 0; i < vec.size(); i++) {
                res.append(vec[i]);
                if (i < vec.size() - 1)
                    res.append(",");
            }
            return res;
        }
        else if (var.userType() == QMetaTypeId<zeno::reflect::Any>::qt_metatype_id())
        {
            const auto& anyVal = var.value<zeno::reflect::Any>();
            if (zeno::reflect::get_type<int>() == anyVal.type()) {
                value = QString::number(zeno::reflect::any_cast<int>(anyVal));
            }
            else if (zeno::reflect::get_type<float>() == anyVal.type()) {
                value = QString::number(zeno::reflect::any_cast<float>(anyVal));
            }
            else if (zeno::reflect::get_type<std::string>() == anyVal.type()) {
                value = QString::fromStdString(zeno::reflect::any_cast<std::string>(anyVal));
            }
            else if (zeno::reflect::get_type<bool>() == anyVal.type()) {
                bool bVal = zeno::reflect::any_cast<float>(anyVal);
                value = bVal ? "true" : "false";
            }
        }
    }
	else
    {
        zeno::log_warn("bad qt variant {}", var.typeName());
    }

    return value;
}

QString UiHelper::editVariantToQString(const zeno::PrimVar& var)
{
    return std::visit([](auto&& val) -> QString {
        using T = std::decay_t<decltype(val)>;
    if constexpr (std::is_same_v<T, int> || std::is_same_v<T, float>) {
        return QString::number(val);
    }
    else if constexpr (std::is_same_v<T, std::string>) {
        return QString::fromStdString(val);
    }
    else {
        return "";
    }
        }, var);
}

QVariant UiHelper::primvarToQVariant(const zeno::PrimVar& var)
{
    return std::visit([&](auto&& val)->QVariant {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, int>) {
            return val;
        }
        else if constexpr (std::is_same_v<T, float>) {
            return val;
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            return QString::fromStdString(val);
        }
        else {
            return QVariant();
        }
        }, var);
}

float UiHelper::parseNumeric(const QVariant& val, bool castStr, bool& bSucceed)
{
    float num = 0;
    QVariant::Type type = val.type();
    if (type == QMetaType::Float || type == QVariant::Double || type == QVariant::Int)
    {
        num = val.toFloat(&bSucceed);
    }
    else if (castStr && type == QVariant::String)
    {
        num = val.toString().toFloat(&bSucceed);
    }
    return num;
}

float UiHelper::parseJsonNumeric(const rapidjson::Value& val, bool castStr, bool& bSucceed)
{
    float num = 0;
    if (val.IsFloat())
    {
        num = val.GetFloat();
        bSucceed = true;
    }
    else if (val.IsDouble())
    {
        num = val.GetDouble();
        bSucceed = true;
    }
    else if (val.IsInt())
    {
        num = val.GetInt();
        bSucceed = true;
    }
    else if (val.IsString() && castStr)
    {
        QString numStr(val.GetString());
        num = numStr.toFloat(&bSucceed);    //may be empty string, no need to assert.
    }
    else
    {
        ZASSERT_EXIT(false, 0.0);
        bSucceed = false;
    }
    return num;
}

QPointF UiHelper::parsePoint(const rapidjson::Value& ptObj, bool& bSucceed)
{
    QPointF pt;

    RAPIDJSON_ASSERT(ptObj.IsArray());
    const auto &arr_ = ptObj.GetArray();
    RAPIDJSON_ASSERT(arr_.Size() == 2);

    const auto &xObj = arr_[0];
    pt.setX(UiHelper::parseJsonNumeric(xObj, false, bSucceed));
    RAPIDJSON_ASSERT(bSucceed);
    if (!bSucceed)
        return pt;

    const auto &yObj = arr_[1];
    pt.setY(UiHelper::parseJsonNumeric(yObj, false, bSucceed));
    RAPIDJSON_ASSERT(bSucceed);
    if (!bSucceed)
        return pt;

    return pt;
}

int UiHelper::getMaxObjId(const QList<QString> &lst)
{
    int maxObjId = -1;
    for (QString key : lst)
    {
        QRegExp rx("obj(\\d+)");
        if (rx.indexIn(key) != -1)
        {
            auto caps = rx.capturedTexts();
            if (caps.length() == 2) {
                int id = caps[1].toInt();
                maxObjId = qMax(maxObjId, id);
            }
        }
    }
    return maxObjId;
}

QString UiHelper::getUniqueName(const QList<QString>& existNames, const QString& prefix, bool bWithBrackets)
{
    int n = 0;
    QString name;
    do
    {
        if (bWithBrackets)
            name = prefix + "(" + QString::number(n++) + ")";
        else
            name = prefix + QString::number(n++);
    } while (existNames.contains(name));
    return name;
}

std::pair<qreal, qreal> UiHelper::getRxx2(QRectF r, qreal xRadius, qreal yRadius, bool AbsoluteSize)
{
    if (AbsoluteSize) {
        qreal w = r.width() / 2;
        qreal h = r.height() / 2;

        if (w == 0) {
            xRadius = 0;
        } else {
            xRadius = 100 * qMin(xRadius, w) / w;
        }
        if (h == 0) {
            yRadius = 0;
        } else {
            yRadius = 100 * qMin(yRadius, h) / h;
        }
    } else {
        if (xRadius > 100)// fix ranges
            xRadius = 100;

        if (yRadius > 100)
            yRadius = 100;
    }

    qreal w = r.width();
    qreal h = r.height();
    qreal rxx2 = w * xRadius / 100;
    qreal ryy2 = h * yRadius / 100;
    return std::make_pair(rxx2, ryy2);
}

QPainterPath UiHelper::getRoundPath(QRectF r, int lt_radius, int rt_radius, int lb_radius, int rb_radius, bool bFixRadius) {
    QPainterPath path;
    if (r.isNull())
        return path;

    if (lt_radius <= 0 && rt_radius <= 0 && lb_radius <= 0 && rb_radius <= 0) {
        path.addRect(r);
        return path;
    }

    qreal x = r.x();
    qreal y = r.y();
    qreal w = r.width();
    qreal h = r.height();

    auto pair = getRxx2(r, lt_radius, lt_radius, bFixRadius);
    qreal rxx2 = pair.first, ryy2 = pair.second;
    if (rxx2 <= 0) {
        path.moveTo(x, y);
    } else {
        path.arcMoveTo(x, y, rxx2, ryy2, 180);
        path.arcTo(x, y, rxx2, ryy2, 180, -90);
    }

    pair = getRxx2(r, rt_radius, rt_radius, bFixRadius);
    rxx2 = pair.first, ryy2 = pair.second;
    if (rxx2 <= 0) {
        path.lineTo(x + w, y);
    } else {
        path.arcTo(x + w - rxx2, y, rxx2, ryy2, 90, -90);
    }

    pair = getRxx2(r, rb_radius, rb_radius, bFixRadius);
    rxx2 = pair.first, ryy2 = pair.second;
    if (rxx2 <= 0) {
        path.lineTo(x + w, y + h);
    } else {
        path.arcTo(x + w - rxx2, y + h - rxx2, rxx2, ryy2, 0, -90);
    }

    pair = getRxx2(r, lb_radius, lb_radius, bFixRadius);
    rxx2 = pair.first, ryy2 = pair.second;
    if (rxx2 <= 0) {
        path.lineTo(x, y + h);
    } else {
        path.arcTo(x, y + h - rxx2, rxx2, ryy2, 270, -90);
    }

    path.closeSubpath();
    return path;
}

QVector<qreal> UiHelper::getSlideStep(const QString& name, zeno::ParamType type)
{
    QVector<qreal> steps;
    if (type == zeno::types::gParamType_Int)
    {
        steps = { 1, 10, 100 };
    }
    else if (type == zeno::types::gParamType_Float)
    {
        steps = { .0001, .001, .01, .1, 1, 10, 100 };
    }
    return steps;
}



QString UiHelper::nthSerialNumName(QString name)
{
    QRegExp rx("\\((\\d+)\\)");
    int idx = rx.lastIndexIn(name);
    if (idx == -1) {
        return name + "(1)";
    }
    else {
        name = name.mid(0, idx);
        QStringList lst = rx.capturedTexts();
        ZASSERT_EXIT(lst.size() == 2, "");
        bool bConvert = false;
        int ith = lst[1].toInt(&bConvert);
        ZASSERT_EXIT(bConvert, "");
        return name + "(" + QString::number(ith + 1) + ")";
    }
}

QVariant UiHelper::parseStringByType(const QString& defaultValue, zeno::ParamType type)
{
    switch (type) {
    case Param_Null:  return QVariant();
    case zeno::types::gParamType_Bool:  return (bool)defaultValue.toInt();
    case zeno::types::gParamType_Int:
    {
        bool bOk = false;
        int val = defaultValue.toInt(&bOk);
        if (bOk) {
            return val;
        }
        else {
            //type dismatch, try to convert to float.
            //disable it now because the sync problem is complicated and trivival.
            //float fVal = defaultValue.toFloat(&bOk);
            //if (bOk)
            //{
            //    val = fVal;
            //    return val;
            //}
            return defaultValue;
        }
    }
    case zeno::types::gParamType_String:    return defaultValue;
    case zeno::types::gParamType_Float:
    {
        bool bOk = false;
        float fVal = defaultValue.toFloat(&bOk);
        if (bOk)
            return fVal;
        else
            return defaultValue;
    }
    case zeno::types::gParamType_Vec2i:
    case zeno::types::gParamType_Vec3i:
    case zeno::types::gParamType_Vec4i:
    case zeno::types::gParamType_Vec2f:
    case zeno::types::gParamType_Vec3f:
    case zeno::types::gParamType_Vec4f:
    {
        UI_VECTYPE vec;
        if (!defaultValue.isEmpty())
        {
            QStringList L = defaultValue.split(",");
            vec.resize(L.size());
            bool bOK = false;
            for (int i = 0; i < L.size(); i++)
            {
                vec[i] = L[i].toFloat(&bOK);
                Q_ASSERT(bOK);
            }
        }
        else
        {
            vec.resize(3);
        }
        return QVariant::fromValue(vec);
    }
    case zeno::types::gParamType_Curve:
    {
        //TODO
        break;
    }
    case gParamType_Primitive:
    case gParamType_Dict:
    case gParamType_List:
            //Param_Color,  //need this?
        break;
    }
    return QVariant();
}

QVariant UiHelper::parseVarByType(const QString& descType, const QVariant& var, QObject* parentRef)
{
    const QVariant::Type varType = var.type();
    if (descType == "int" ||
        descType == "float" ||
        descType == "NumericObject" ||
        descType == "numeric:float" ||
        descType == "floatslider")
    {
        switch (varType)
        {
            case QVariant::Int:
            case QMetaType::Float:
            case QVariant::Double:
            case QVariant::UInt:
            case QVariant::LongLong:
            case QVariant::ULongLong: return var;
            case QVariant::String:
            {
                //string numeric, try to parse to numeric.
                bool bOk = false;
                float fVal = var.toString().toFloat(&bOk);
                if (bOk)
                    return QVariant(fVal);
            }
            default:
                return QVariant();
        }
    }
    else if ((descType == "string" ||
              descType == "writepath" ||
              descType == "readpath" ||
              descType == "multiline_string" ||
              descType.startsWith("enum ")))
    {
        if (varType == QVariant::String)
            return var;
        else
            return QVariant();
    }
    else if (descType == "bool")
    {
        if (varType == QVariant::Int)
        {
            return var.toInt() != 0;
        }
        else if (varType == QMetaType::Float)
        {
            return var.toFloat() != 0;
        }
        else if (varType == QVariant::Double)
        {
            return var.toDouble() != 0;
        }
        else if (varType == QVariant::Bool)
        {
            return var;
        }
        else if (varType == QVariant::String)
        {
            QString boolStr = var.toString();
            if (boolStr == "true")
                return true;
            if (boolStr == "false")
                return false;
        }
        return QVariant();
    }
    else if (descType.startsWith("vec") && varType == QVariant::UserType &&
             var.userType() == QMetaTypeId<UI_VECTYPE>::qt_metatype_id())
    {
        if (varType == QVariant::UserType && var.userType() == QMetaTypeId<UI_VECTYPE>::qt_metatype_id())
        {
            return var;
        }
        else if (varType == QVariant::String)
        {
            auto lst = var.toString().split(",");
            if (lst.isEmpty())
                return QVariant();
            UI_VECTYPE vec;
            for (int i = 0; i < lst.size(); i++)
            {
                QString str = lst[i];
                bool bOk = false;
                float fVal = str.toFloat(&bOk);
                if (!bOk)
                    return QVariant();
                vec.append(fVal);
            }
            return QVariant::fromValue(vec);
        }
        return QVariant();
    }
    else if (descType == "curve")
    {
        if (varType == QMetaType::User)
        {
            return var;
        }
        //legacy curve is expressed as string, and no desc type associated with it.
        return QVariant();
    }

    //string:
    if (varType == QVariant::String)
    {
        QString str = var.toString();
        if (str.isEmpty()) {
            // the default value of many types, for example primitive, are empty string,
            // skip it and return a invalid variant.
            return QVariant();
        }
        //try to convert to numeric.
        bool bOk = false;
        float fVal = str.toFloat(&bOk);
        if (bOk)
            return fVal;
    }
    //unregister type or unknown data, return itself.
    return var;
}

QVariant UiHelper::parseJson(const rapidjson::Value& val, QObject* parentRef)
{
    if (val.GetType() == rapidjson::kStringType)
    {
        bool bSucc = false;
        float fVal = parseJsonNumeric(val, true, bSucc);
        if (bSucc)
            return fVal;
        return val.GetString();
    }
    else if (val.GetType() == rapidjson::kNumberType)
    {
        if (val.IsDouble())
            return val.GetDouble();
        else if (val.IsInt())
            return val.GetInt();
        else {
            zeno::log_warn("bad rapidjson number type {}", val.GetType());
            return QVariant();
        }
    }
    else if (val.GetType() == rapidjson::kTrueType)
    {
        return val.GetBool();
    }
    else if (val.GetType() == rapidjson::kFalseType)
    {
        return val.GetBool();
    }
    else if (val.GetType() == rapidjson::kNullType)
    {
        return QVariant();
    }
    else if (val.GetType() == rapidjson::kArrayType)
    {
        //detect whether it is a numeric vector.
        auto values = val.GetArray();
        bool bNumeric = true;
        for (int i = 0; i < values.Size(); i++)
        {
            if (!values[i].IsNumber())
            {
                bNumeric = false;
                break;
            }
        }
        if (bNumeric)
        {
            UI_VECTYPE vec;
            for (int i = 0; i < values.Size(); i++)
            {
                const auto& numObj = values[i];
                if (numObj.IsInt() || numObj.IsInt64() || numObj.IsUint() || numObj.IsUint64())
                    vec.append(values[i].GetInt());
                else if (numObj.IsFloat() || numObj.IsDouble())
                    vec.append(values[i].GetFloat());
            }
            return QVariant::fromValue(vec);
        }
        else
        {
            QStringList lst;
            for (int i = 0; i < values.Size(); i++)
            {
                const auto& obj = values[i];
                if (obj.IsNumber()) {
                    lst.append(QString::number(obj.GetFloat()));
                }
                else if (obj.IsString()) {
                    lst.append(QString::fromUtf8(obj.GetString()));
                }
            }
            return lst;
        }
    }
    else if (val.GetType() == rapidjson::kObjectType)
    {
        //if (type == "curve")
        //{
        //    CurveModel* pModel = JsonHelper::_parseCurveModel(val, parentRef);
        //    return QVariantPtr<CurveModel>::asVariant(pModel);
        //}
    }

    zeno::log_warn("bad rapidjson value type {}", val.GetType());
    return QVariant();
}

QString UiHelper::gradient2colorString(const QLinearGradient& grad)
{
    QString colorStr;
    const QGradientStops &stops = grad.stops();
    colorStr += QString::number(stops.size());
    colorStr += "\n";
    for (QGradientStop grad : stops) {
        colorStr += QString::number(grad.first);
        colorStr += " ";
        colorStr += QString::number(grad.second.redF());
        colorStr += " ";
        colorStr += QString::number(grad.second.greenF());
        colorStr += " ";
        colorStr += QString::number(grad.second.blueF());
        colorStr += "\n";
    }
    return colorStr;
}

QLinearGradient UiHelper::colorString2Grad(const QString& colorStr)
{
    QLinearGradient grad;
    if (colorStr.isEmpty())
        return grad;
    QStringList L = colorStr.split("\n", QtSkipEmptyParts);
    ZASSERT_EXIT(!L.isEmpty(), grad);

    bool bOk = false;
    int n = L[0].toInt(&bOk);
    ZASSERT_EXIT(bOk && n == L.size() - 1, grad);
    for (int i = 1; i <= n; i++)
    {
        QStringList color_info = L[i].split(" ", QtSkipEmptyParts);
        ZASSERT_EXIT(color_info.size() == 4, grad);

        float f = color_info[0].toFloat(&bOk);
        ZASSERT_EXIT(bOk, grad);
        float r = color_info[1].toFloat(&bOk);
        ZASSERT_EXIT(bOk, grad);
        float g = color_info[2].toFloat(&bOk);
        ZASSERT_EXIT(bOk, grad);
        float b = color_info[3].toFloat(&bOk);
        ZASSERT_EXIT(bOk, grad);

        QColor clr;
        clr.setRgbF(r, g, b);
        grad.setColorAt(f, clr);
    }
    return grad;
}

int UiHelper::tabIndexOfName(const QTabWidget* pTabWidget, const QString& name)
{
    if (!pTabWidget)
        return -1;
    for (int i = 0; i < pTabWidget->count(); i++)
    {
        if (pTabWidget->tabText(i) == name)
        {
            return i;
        }
    }
    return -1;
}

QVector<qreal> UiHelper::scaleFactors()
{
    static QVector<qreal> lst({0.01, 0.025, 0.05, .1, .15, .2, .25, .5, .75, 1.0, 1.25, 1.5, 2.0, 3.0, 4.0, 5.0});
    return lst;
}

zeno::CurvesData UiHelper::getCurvesFromQVar(const QVariant& qvar, bool* bValid) {
    zeno::CurvesData curves;
    if (qvar.canConvert<zeno::reflect::Any>()) {
        const auto& anyVal = qvar.value<zeno::reflect::Any>();
        if (anyVal.has_value() && zeno::reflect::get_type<zeno::PrimVar>() == anyVal.type()) {  //ֻ��һ��curve
            zeno::PrimVar primvar = zeno::reflect::any_cast<zeno::PrimVar>(anyVal);
            if (std::holds_alternative<zeno::CurveData>(primvar)) {
                if (bValid) *bValid = true;
                curves.keys.insert({"x", std::get<zeno::CurveData>(primvar)});
                return zeno::reflect::any_cast<zeno::CurvesData>(curves);
            }
        } else if (anyVal.has_value() && zeno::reflect::get_type<zeno::vecvar>() == anyVal.type()) {
            zeno::vecvar vvar = zeno::reflect::any_cast<zeno::vecvar>(anyVal);
            for (int i = 0; i < vvar.size(); i++) {
                if (std::holds_alternative<zeno::CurveData>(vvar[i])) {
                    curves.keys.insert({ curve_util::getCurveKey(i).toStdString(), std::get<zeno::CurveData>(vvar[i])});
                } else {
                    if (bValid) *bValid = false;
                    return curves;
                }
            }
            if (bValid) *bValid = true;
            return curves;
        }
    }
    if (bValid) *bValid = false;
    return curves;
}

QVariant UiHelper::getQVarFromCurves(const zeno::CurvesData& curves) {
    return QVariant::fromValue(zeno::reflect::make_any<zeno::CurvesData>(curves));
}

QString UiHelper::getNaiveParamPath(const QModelIndex& param, int dim)
{
    auto nodeIdx = param.data(QtRole::ROLE_NODEIDX).toModelIndex();
    if (!nodeIdx.isValid())
        return "";
    QString path = nodeIdx.data(QtRole::ROLE_OBJPATH).toString();
    QString paramName = param.data(QtRole::ROLE_PARAM_NAME).toString();
    if (dim == 0) {
        paramName += ".x";
    }
    else if (dim == 1) {
        paramName += ".y";
    }
    else if (dim == 2) {
        paramName += ".z";
    }
    else if (dim == 3) {
        paramName += ".w";
    }
    path += paramName;
    return path;
}

zeno::NodesData UiHelper::dumpNodes(const QModelIndexList &nodeIndice)
{
    zeno::NodesData nodes;

    QSet<QString> existedNodes;
    for (const auto& idx : nodeIndice)
    {
        zeno::NodeData data = idx.data(QtRole::ROLE_NODEDATA).value<zeno::NodeData>();
        nodes[data.name] = data;
    }
    return nodes;
}

void UiHelper::reAllocIdents(const QString& targetSubgraph,
                              const zeno::NodesData& inNodes,
                              const zeno::LinksData& inLinks,
                              zeno::NodesData& outNodes,
                              zeno::LinksData& outLinks)
{
    QMap<QString, QString> old2new;
    for (const auto& [key, data] : inNodes)
    {
        const auto& oldId = data.name;
        const auto& name = data.cls;
        const QString& newId = UiHelper::generateUuid(QString::fromStdString(name));
        zeno::NodeData newData = data;
        newData.name = newId.toStdString();
        outNodes.insert(std::make_pair(newData.name, newData));
        old2new.insert(QString::fromStdString(oldId), newId);
    }
    //replace all the old-id in newNodes, and clear cached links.
    for (auto& [key, data] : outNodes)
    {
        for (auto& param : data.customUi.inputObjs)
        {
            param.links.clear();
        }
        for (auto& param : data.customUi.outputObjs)
        {
            param.links.clear();
        }
        auto inputs = zeno::customUiToParams(data.customUi.inputPrims);
        for (auto& param : inputs)
        {
            param.links.clear();
        }
        for (auto& param : data.customUi.outputPrims)
        {
            param.links.clear();
        }
    }

    for (const zeno::EdgeInfo& link : inLinks)
    {
        QString outputNode = QString::fromStdString(link.outNode);
        QString outParam = QString::fromStdString(link.outParam);
        QString outKey = QString::fromStdString(link.outKey);

        QString inputNode = QString::fromStdString(link.inNode);
        QString inParam = QString::fromStdString(link.inParam);
        QString inKey = QString::fromStdString(link.inKey);

        ZASSERT_EXIT(old2new.find(inputNode) != old2new.end() &&
                     old2new.find(outputNode) != old2new.end());
        QString newInputNode = old2new[inputNode];
        QString newOutputNode = old2new[outputNode];

        zeno::EdgeInfo newLink = link;
        newLink.outNode = newOutputNode.toStdString();
        newLink.inNode = newInputNode.toStdString();

        outLinks.push_back(newLink);
    }
}

QStandardItemModel* UiHelper::genParamsModel(const std::vector<zeno::ParamPrimitive>& inputs, const std::vector<zeno::ParamPrimitive>& outputs)
{
    QStandardItemModel* customParamsM = new QStandardItemModel;

    QStandardItem* pRoot = new QStandardItem("root");
    QStandardItem* pInputs = new QStandardItem("input");
    QStandardItem* pOutputs = new QStandardItem("output");

    for (zeno::ParamPrimitive info : inputs) {
        QStandardItem* paramItem = new QStandardItem(QString::fromStdString(info.name));
        const QString& paramName = QString::fromStdString(info.name);
        paramItem->setData(paramName, Qt::DisplayRole);
        paramItem->setData(paramName, QtRole::ROLE_PARAM_NAME);
        paramItem->setData(paramName, ROLE_MAP_TO_PARAMNAME);
        paramItem->setData(QVariant::fromValue(info.defl), QtRole::ROLE_PARAM_VALUE);
        paramItem->setData(info.control, QtRole::ROLE_PARAM_CONTROL);
        paramItem->setData(info.type, QtRole::ROLE_PARAM_TYPE);
        paramItem->setData(true, QtRole::ROLE_ISINPUT);
        paramItem->setData(VPARAM_PARAM, ROLE_ELEMENT_TYPE);
        paramItem->setEditable(true);
        pInputs->appendRow(paramItem);
    }

    for (zeno::ParamPrimitive info : outputs) {
        QStandardItem* paramItem = new QStandardItem(QString::fromStdString(info.name));
        const QString& paramName = QString::fromStdString(info.name);
        paramItem->setData(paramName, Qt::DisplayRole);
        paramItem->setData(paramName, QtRole::ROLE_PARAM_NAME);
        paramItem->setData(paramName, ROLE_MAP_TO_PARAMNAME);
        paramItem->setData(QVariant::fromValue(info.defl), QtRole::ROLE_PARAM_VALUE);
        paramItem->setData(info.control, QtRole::ROLE_PARAM_CONTROL);
        paramItem->setData(info.type, QtRole::ROLE_PARAM_TYPE);
        paramItem->setData(false, QtRole::ROLE_ISINPUT);
        paramItem->setData(VPARAM_PARAM, ROLE_ELEMENT_TYPE);
        paramItem->setEditable(true);
        pOutputs->appendRow(paramItem);
    }

    pRoot->setEditable(false);
    pRoot->setData(VPARAM_TAB, ROLE_ELEMENT_TYPE);
    pInputs->setEditable(false);
    pInputs->setData(VPARAM_GROUP, ROLE_ELEMENT_TYPE);
    pOutputs->setEditable(false);
    pOutputs->setData(VPARAM_GROUP, ROLE_ELEMENT_TYPE);

    pRoot->appendRow(pInputs);
    pRoot->appendRow(pOutputs);

    customParamsM->appendRow(pRoot);
    return customParamsM;
}

void UiHelper::newCustomModel(QStandardItemModel* customParamsM, const zeno::CustomUI& customui)
{
    ZASSERT_EXIT(customParamsM);

    customParamsM->clear();

    QStandardItem* pInputs = new QStandardItem("prim_inputs");
    pInputs->setEditable(false);
    for (const zeno::ParamTab& tab : customui.inputPrims)
    {
        const QString& tabName = QString::fromStdString(tab.name);
        QStandardItem* pTab = new QStandardItem(tabName);
        pTab->setData(VPARAM_TAB, ROLE_ELEMENT_TYPE);
        pTab->setData(zeno::Role_InputPrimitive, QtRole::ROLE_PARAM_GROUP);
        pTab->setData(tabName, QtRole::ROLE_PARAM_NAME);

        for (const zeno::ParamGroup& group : tab.groups)
        {
            const QString& groupName = QString::fromStdString(group.name);
            QStandardItem* pGroup = new QStandardItem(groupName);
            pGroup->setData(VPARAM_GROUP, ROLE_ELEMENT_TYPE);
            pGroup->setData(zeno::Role_InputPrimitive, QtRole::ROLE_PARAM_GROUP);
            pGroup->setData(groupName, QtRole::ROLE_PARAM_NAME);

            for (const zeno::ParamPrimitive& param : group.params)
            {
                QStandardItem* paramItem = new QStandardItem(QString::fromStdString(param.name));
                paramItem->setData(VPARAM_PARAM, ROLE_ELEMENT_TYPE);

                const QString& paramName = QString::fromStdString(param.name);
                paramItem->setData(paramName, Qt::DisplayRole);
                paramItem->setData(paramName, QtRole::ROLE_PARAM_NAME);
                paramItem->setData(paramName, ROLE_MAP_TO_PARAMNAME);
                paramItem->setData(QVariant::fromValue(param.defl), QtRole::ROLE_PARAM_VALUE);
                paramItem->setData(param.control, QtRole::ROLE_PARAM_CONTROL);
                paramItem->setData(param.type, QtRole::ROLE_PARAM_TYPE);
                paramItem->setData(true, QtRole::ROLE_ISINPUT);
                paramItem->setData(param.socketType, QtRole::ROLE_SOCKET_TYPE);
                paramItem->setData(param.bSocketVisible, QtRole::ROLE_PARAM_SOCKET_VISIBLE);
                if (param.ctrlProps.has_value())
                    paramItem->setData(QVariant::fromValue(param.ctrlProps), QtRole::ROLE_PARAM_CTRL_PROPERTIES);
                paramItem->setData(zeno::Role_InputPrimitive, QtRole::ROLE_PARAM_GROUP);
                pGroup->appendRow(paramItem);
            }
            pTab->appendRow(pGroup);
        }
        pInputs->appendRow(pTab);
    }

    QStandardItem* pOutputs = new QStandardItem("prim_outputs");
    pOutputs->setEditable(false);
    for (const zeno::ParamPrimitive& param : customui.outputPrims)
    {
        const QString& paramName = QString::fromStdString(param.name);
        QStandardItem* paramItem = new QStandardItem(paramName);
        paramItem->setData(paramName, Qt::DisplayRole);
        paramItem->setData(paramName, QtRole::ROLE_PARAM_NAME);
        paramItem->setData(paramName, ROLE_MAP_TO_PARAMNAME);
        paramItem->setData(QVariant::fromValue(param.defl), QtRole::ROLE_PARAM_VALUE);
        paramItem->setData(param.control, QtRole::ROLE_PARAM_CONTROL);
        paramItem->setData(param.type, QtRole::ROLE_PARAM_TYPE);
        paramItem->setData(false, QtRole::ROLE_ISINPUT);
        paramItem->setData(param.socketType, QtRole::ROLE_SOCKET_TYPE);
        if (param.ctrlProps.has_value())
            paramItem->setData(QVariant::fromValue(param.ctrlProps), QtRole::ROLE_PARAM_CTRL_PROPERTIES);
        pOutputs->appendRow(paramItem);
        paramItem->setData(VPARAM_PARAM, ROLE_ELEMENT_TYPE);
        paramItem->setData(zeno::Role_OutputPrimitive, QtRole::ROLE_PARAM_GROUP);
        paramItem->setEditable(true);
    }
    //object params
    QStandardItem* pObjInputs = new QStandardItem("object_inputs");
    pObjInputs->setEditable(false);
    for (const auto& param : customui.inputObjs)
    {
        const QString& paramName = QString::fromStdString(param.name);
        QStandardItem* paramItem = new QStandardItem(paramName);
        paramItem->setData(paramName, Qt::DisplayRole);
        paramItem->setData(paramName, QtRole::ROLE_PARAM_NAME);
        paramItem->setData(paramName, ROLE_MAP_TO_PARAMNAME);
        paramItem->setData(param.type, QtRole::ROLE_PARAM_TYPE);
        paramItem->setData(true, QtRole::ROLE_ISINPUT);
        paramItem->setData(param.socketType, QtRole::ROLE_SOCKET_TYPE);
        paramItem->setData(VPARAM_PARAM, ROLE_ELEMENT_TYPE);
        paramItem->setData(zeno::Role_InputObject, QtRole::ROLE_PARAM_GROUP);
        paramItem->setEditable(true);
        pObjInputs->appendRow(paramItem);
    }

    QStandardItem* pObjOutputs = new QStandardItem("object_outputs");
    pObjOutputs->setEditable(false);
    for (const auto& param : customui.outputObjs)
    {
        const QString& paramName = QString::fromStdString(param.name);
        QStandardItem* paramItem = new QStandardItem(paramName);
        paramItem->setData(paramName, Qt::DisplayRole);
        paramItem->setData(paramName, QtRole::ROLE_PARAM_NAME);
        paramItem->setData(paramName, ROLE_MAP_TO_PARAMNAME);
        paramItem->setData(param.type, QtRole::ROLE_PARAM_TYPE);
        paramItem->setData(false, QtRole::ROLE_ISINPUT);
        paramItem->setData(param.socketType, QtRole::ROLE_SOCKET_TYPE);
        paramItem->setData(VPARAM_PARAM, ROLE_ELEMENT_TYPE);
        paramItem->setData(zeno::Role_OutputObject, QtRole::ROLE_PARAM_GROUP);
        paramItem->setEditable(true);
        pObjOutputs->appendRow(paramItem);
    }
    customParamsM->appendRow(pInputs);
    customParamsM->appendRow(pOutputs);
    customParamsM->appendRow(pObjInputs);
    customParamsM->appendRow(pObjOutputs);
}

void UiHelper::udpateCustomModelIncremental(QStandardItemModel* customParamsM, const zeno::params_change_info& changes, const zeno::CustomUI& customui)
{
    QStandardItem* pInputsRoot = customParamsM->item(0);
    QStandardItem* pOutputsRoot = customParamsM->item(1);
    QStandardItem* pObjInputsRoot = customParamsM->item(2);
    QStandardItem* pObjOutputsRoot = customParamsM->item(3);
    if (!pInputsRoot || !pOutputsRoot || !pObjInputsRoot || !pObjOutputsRoot)
        return;
    //if (!pInputsRoot->hasChildren() || !pInputsRoot->child(0)->hasChildren())
    //    return;
    //if (customui.inputPrims.empty() || customui.inputPrims[0].groups.empty())
    //    return;
    auto const& renameItem = [](QStandardItem* item, const std::string& name) {
        item->setData(QString::fromStdString(name), Qt::DisplayRole);
        item->setData(QString::fromStdString(name), QtRole::ROLE_PARAM_NAME);
        item->setData(QString::fromStdString(name), ROLE_MAP_TO_PARAMNAME);
    };
    auto const& makeInputPrimItem = [](zeno::ParamPrimitive const& param) -> QStandardItem* {
        QStandardItem* paramItem = new QStandardItem(QString::fromStdString(param.name));
        paramItem->setData(VPARAM_PARAM, ROLE_ELEMENT_TYPE);
        const QString& paramName = QString::fromStdString(param.name);
        paramItem->setData(paramName, Qt::DisplayRole);
        paramItem->setData(paramName, QtRole::ROLE_PARAM_NAME);
        paramItem->setData(paramName, ROLE_MAP_TO_PARAMNAME);
        paramItem->setData(QVariant::fromValue(param.defl), QtRole::ROLE_PARAM_VALUE);
        paramItem->setData(param.control, QtRole::ROLE_PARAM_CONTROL);
        paramItem->setData(param.type, QtRole::ROLE_PARAM_TYPE);
        paramItem->setData(true, QtRole::ROLE_ISINPUT);
        paramItem->setData(param.socketType, QtRole::ROLE_SOCKET_TYPE);
        paramItem->setData(param.bSocketVisible, QtRole::ROLE_PARAM_SOCKET_VISIBLE);
        if (param.ctrlProps.has_value())
            paramItem->setData(QVariant::fromValue(param.ctrlProps), QtRole::ROLE_PARAM_CTRL_PROPERTIES);
        paramItem->setData(zeno::Role_InputPrimitive, QtRole::ROLE_PARAM_GROUP);
        return paramItem;
    };
    auto const& updateGroup = [&changes, &renameItem, &makeInputPrimItem](QStandardItem* pItemGroup, zeno::PrimitiveParams const& inputPrims) {
        for (int i = 0; i < inputPrims.size(); i++) {
            bool defaultInitCase = i >= pItemGroup->rowCount();
            auto const& param = inputPrims[i];
            if (defaultInitCase || param.name != pItemGroup->child(i)->data(QtRole::ROLE_PARAM_NAME).toString().toStdString()) {
                if (defaultInitCase || changes.new_inputs.find(param.name) != changes.new_inputs.end()) {
                    pItemGroup->insertRow(i, makeInputPrimItem(param));
                    continue;
                }
                for (auto& pair : changes.rename_inputs) {
                    if (pair.first == pItemGroup->child(i)->data(QtRole::ROLE_PARAM_NAME).toString().toStdString()) {
                        renameItem(pItemGroup->child(i), pair.second);
                        break;
                    }
                }
            }
        }
    };
    //inputPrims
    std::unordered_set<std::string> tabnames, groupnames;
    for (int tabCount = 0; tabCount < customui.inputPrims.size(); tabCount++) {
        for (int groupCount = 0; groupCount < customui.inputPrims[tabCount].groups.size(); groupCount++)
            groupnames.insert(customui.inputPrims[tabCount].groups[groupCount].name);
        tabnames.insert(customui.inputPrims[tabCount].name);
    }
    for (int tabCount = pInputsRoot->rowCount() - 1; tabCount >= 0; tabCount--) {
        QStandardItem* tabitem = pInputsRoot->child(tabCount);
        for (int groupCount = tabitem->rowCount() - 1; groupCount >= 0; groupCount--) {
            QStandardItem* groupitem = tabitem->child(groupCount);
            for (int paramCount = groupitem->rowCount() - 1; paramCount >= 0; paramCount--) {
                if (changes.remove_inputs.find(groupitem->child(paramCount)->data(QtRole::ROLE_PARAM_NAME).toString().toStdString()) != changes.remove_inputs.end())
                    groupitem->removeRow(paramCount);
            }
            if (groupitem->rowCount() == 0 && !groupnames.count(groupitem->data(QtRole::ROLE_PARAM_NAME).toString().toStdString()))
                tabitem->removeRow(groupCount);
        }
        if (tabitem->rowCount() == 0 && !tabnames.count(tabitem->data(QtRole::ROLE_PARAM_NAME).toString().toStdString()))
            pInputsRoot->removeRow(tabCount);
    }
    for (int tabCount = 0; tabCount < customui.inputPrims.size(); tabCount++) {
        const auto& tabItem = pInputsRoot->child(tabCount);
        const auto& tabName = customui.inputPrims[tabCount].name;
        if (pInputsRoot->rowCount() < tabCount + 1) {
            QStandardItem* pTab = new QStandardItem(QString::fromStdString(tabName));
            pTab->setData(VPARAM_TAB, ROLE_ELEMENT_TYPE);
            pTab->setData(zeno::Role_InputPrimitive, QtRole::ROLE_PARAM_GROUP);
            pTab->setData(QString::fromStdString(tabName), QtRole::ROLE_PARAM_NAME);
            pInputsRoot->appendRow(pTab);
            for (const zeno::ParamGroup& group : customui.inputPrims[tabCount].groups) {
                const QString& groupName = QString::fromStdString(group.name);
                QStandardItem* pGroup = new QStandardItem(groupName);
                pGroup->setData(VPARAM_GROUP, ROLE_ELEMENT_TYPE);
                pGroup->setData(zeno::Role_InputPrimitive, QtRole::ROLE_PARAM_GROUP);
                pGroup->setData(groupName, QtRole::ROLE_PARAM_NAME);
                pTab->appendRow(pGroup);
                for (const zeno::ParamPrimitive& param : group.params) {
                    pGroup->appendRow(makeInputPrimItem(param));
                }
            }
        }
        else {
            if (tabItem->data(QtRole::ROLE_PARAM_NAME).toString().toStdString() != tabName)
                renameItem(tabItem, tabName);
            for (int groupCount = 0; groupCount < customui.inputPrims[tabCount].groups.size(); groupCount++) {
                const auto& groupItem = tabItem->child(groupCount);
                const auto& groupName = customui.inputPrims[tabCount].groups[groupCount].name;
                if (tabItem->rowCount() < groupCount + 1) {
                    QStandardItem* pGroup = new QStandardItem(QString::fromStdString(groupName));
                    pGroup->setData(VPARAM_GROUP, ROLE_ELEMENT_TYPE);
                    pGroup->setData(zeno::Role_InputPrimitive, QtRole::ROLE_PARAM_GROUP);
                    pGroup->setData(QString::fromStdString(groupName), QtRole::ROLE_PARAM_NAME);
                    tabItem->appendRow(pGroup);
                    for (const zeno::ParamPrimitive& param : customui.inputPrims[tabCount].groups[groupCount].params) {
                        pGroup->appendRow(makeInputPrimItem(param));
                    }
                }
                else {
                    if (groupItem->data(QtRole::ROLE_PARAM_NAME).toString().toStdString() != groupName)
                        renameItem(groupItem, groupName);
                    updateGroup(groupItem, customui.inputPrims[tabCount].groups[groupCount].params);
                }
            }
        }
    }
    //inputObjs
    for (int i = pObjInputsRoot->rowCount() - 1; i >= 0; i--) {
        if (changes.remove_inputs.find(pObjInputsRoot->child(i)->data(QtRole::ROLE_PARAM_NAME).toString().toStdString()) != changes.remove_inputs.end())
            pObjInputsRoot->removeRow(i);
    }
    for (int i = 0; i < customui.inputObjs.size(); i++) {
        bool defaultInitCase = i >= pObjInputsRoot->rowCount();
        auto const& param = customui.inputObjs[i];
        if (defaultInitCase || param.name != pObjInputsRoot->child(i)->data(QtRole::ROLE_PARAM_NAME).toString().toStdString()) {
            if (defaultInitCase || changes.new_inputs.find(param.name) != changes.new_inputs.end()) {
                const QString& paramName = QString::fromStdString(param.name);
                QStandardItem* paramItem = new QStandardItem(paramName);
                paramItem->setData(paramName, Qt::DisplayRole);
                paramItem->setData(paramName, QtRole::ROLE_PARAM_NAME);
                paramItem->setData(paramName, ROLE_MAP_TO_PARAMNAME);
                paramItem->setData(param.type, QtRole::ROLE_PARAM_TYPE);
                paramItem->setData(true, QtRole::ROLE_ISINPUT);
                paramItem->setData(param.socketType, QtRole::ROLE_SOCKET_TYPE);
                paramItem->setData(VPARAM_PARAM, ROLE_ELEMENT_TYPE);
                paramItem->setData(zeno::Role_InputObject, QtRole::ROLE_PARAM_GROUP);
                paramItem->setEditable(true);
                pObjInputsRoot->insertRow(i, paramItem);
                continue;
            }
            for (auto& pair : changes.rename_inputs) {
                if (pair.first == pObjInputsRoot->child(i)->data(QtRole::ROLE_PARAM_NAME).toString().toStdString()) {
                    renameItem(pObjInputsRoot->child(i), pair.second);
                    break;
                }
            }
        }
    }
    //outputPrims
    for (int i = pOutputsRoot->rowCount() - 1; i >= 0; i--) {
        if (changes.remove_outputs.find(pOutputsRoot->child(i)->data(QtRole::ROLE_PARAM_NAME).toString().toStdString()) != changes.remove_outputs.end())
            pOutputsRoot->removeRow(i);
    }
    for (int i = 0; i < customui.outputPrims.size(); i++) {
        bool defaultInitCase = i >= pOutputsRoot->rowCount();
        auto const& param = customui.outputPrims[i];
        if (defaultInitCase || param.name != pOutputsRoot->child(i)->data(QtRole::ROLE_PARAM_NAME).toString().toStdString()) {
            if (defaultInitCase || changes.new_outputs.find(param.name) != changes.new_outputs.end()) {
                const QString& paramName = QString::fromStdString(param.name);
                QStandardItem* paramItem = new QStandardItem(paramName);
                paramItem->setData(paramName, Qt::DisplayRole);
                paramItem->setData(paramName, QtRole::ROLE_PARAM_NAME);
                paramItem->setData(paramName, ROLE_MAP_TO_PARAMNAME);
                paramItem->setData(QVariant::fromValue(param.defl), QtRole::ROLE_PARAM_VALUE);
                paramItem->setData(param.control, QtRole::ROLE_PARAM_CONTROL);
                paramItem->setData(param.type, QtRole::ROLE_PARAM_TYPE);
                paramItem->setData(false, QtRole::ROLE_ISINPUT);
                paramItem->setData(param.socketType, QtRole::ROLE_SOCKET_TYPE);
                if (param.ctrlProps.has_value())
                    paramItem->setData(QVariant::fromValue(param.ctrlProps), QtRole::ROLE_PARAM_CTRL_PROPERTIES);
                paramItem->setData(VPARAM_PARAM, ROLE_ELEMENT_TYPE);
                paramItem->setData(zeno::Role_OutputPrimitive, QtRole::ROLE_PARAM_GROUP);
                paramItem->setEditable(true);
                pOutputsRoot->insertRow(i, paramItem);
                continue;
            }
            for (auto& pair : changes.rename_outputs) {
                if (pair.first == pOutputsRoot->child(i)->data(QtRole::ROLE_PARAM_NAME).toString().toStdString()) {
                    renameItem(pOutputsRoot->child(i), pair.second);
                    break;
                }
            }
        }
    }
    //outputObjs
    for (int i = pObjOutputsRoot->rowCount() - 1; i >= 0; i--) {
        if (changes.remove_outputs.find(pObjOutputsRoot->child(i)->data(QtRole::ROLE_PARAM_NAME).toString().toStdString()) != changes.remove_outputs.end())
            pObjOutputsRoot->removeRow(i);
    }
    for (int i = 0; i < customui.outputObjs.size(); i++) {
        bool defaultInitCase = i >= pObjOutputsRoot->rowCount();
        auto const& param = customui.outputObjs[i];
        if (defaultInitCase || param.name != pObjOutputsRoot->child(i)->data(QtRole::ROLE_PARAM_NAME).toString().toStdString()) {
            if (defaultInitCase || changes.new_outputs.find(param.name) != changes.new_outputs.end()) {
                const QString& paramName = QString::fromStdString(param.name);
                QStandardItem* paramItem = new QStandardItem(paramName);
                paramItem->setData(paramName, Qt::DisplayRole);
                paramItem->setData(paramName, QtRole::ROLE_PARAM_NAME);
                paramItem->setData(paramName, ROLE_MAP_TO_PARAMNAME);
                paramItem->setData(param.type, QtRole::ROLE_PARAM_TYPE);
                paramItem->setData(false, QtRole::ROLE_ISINPUT);
                paramItem->setData(param.socketType, QtRole::ROLE_SOCKET_TYPE);
                paramItem->setData(VPARAM_PARAM, ROLE_ELEMENT_TYPE);
                paramItem->setData(zeno::Role_OutputObject, QtRole::ROLE_PARAM_GROUP);
                paramItem->setEditable(true);
                pObjOutputsRoot->insertRow(i, paramItem);
                continue;
            }
            for (auto& pair : changes.rename_outputs) {
                if (pair.first == pObjOutputsRoot->child(i)->data(QtRole::ROLE_PARAM_NAME).toString().toStdString()) {
                    renameItem(pObjOutputsRoot->child(i), pair.second);
                    break;
                }
            }
        }
    }
}

static std::string getZenoVersion()
{
    const char *date = __DATE__;
    const char *table[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
    };
    int month = std::find(table, table + 12, std::string(date, 3)) - table + 1;
    int day = std::stoi(std::string(date + 4, 2));
    int year = std::stoi(std::string(date + 7, 4));
    return zeno::format("{:04d}.{:02d}.{:02d}", year, month, day);
}

void UiHelper::saveProject(const QString& name)
{
    if (name == "main") {
        zenoApp->getMainWindow()->save();
    }
    else {
        zenoApp->graphsManager()->assetsModel()->saveAsset(name);
        GraphModel* pModel =  zenoApp->graphsManager()->getGraph({ "main" });
        if (pModel)
        {
            pModel->syncToAssetsInstance(name);
        }
    }
}

QStringList UiHelper::stdlistToQStringList(const zeno::ObjPath& objpath)
{
    QString str = QString::fromStdString(objpath);
    QStringList lst = str.split("/");
    return lst;
}

QStringList UiHelper::findPreviousNode(GraphModel* pModel, const QString& node)
{
    QStringList nodes;
    const QModelIndex& nodeIdx = pModel->indexFromName(node);
    if (!nodeIdx.isValid())
        return nodes;

    zeno::NodeData nodeDat = nodeIdx.data(QtRole::ROLE_NODEDATA).value<zeno::NodeData>();
    auto inputs = zeno::customUiToParams(nodeDat.customUi.inputPrims);
    for (const zeno::ParamPrimitive& param : inputs)
    {
        for (const zeno::EdgeInfo& link : param.links)
        {
            nodes.append(QString::fromStdString(link.outNode));
        }
    }
    return nodes;
}

QStringList UiHelper::findSuccessorNode(GraphModel* pModel, const QString& node)
{
    QStringList nodes;
    const QModelIndex& nodeIdx = pModel->indexFromName(node);
    if (!nodeIdx.isValid())
        return nodes;

    zeno::NodeData nodeDat = nodeIdx.data(QtRole::ROLE_NODEDATA).value<zeno::NodeData>();
    for (const zeno::ParamPrimitive& param : nodeDat.customUi.outputPrims)
    {
        for (const zeno::EdgeInfo& link : param.links)
        {
            nodes.append(QString::fromStdString(link.inNode));
        }
    }
    return nodes;
}

QStringList UiHelper::findAllLinkdNodes(GraphModel* pModel, const QString& node)
{
    std::unordered_set<std::string> nodesName;
    std::function<void(GraphModel*, std::string, std::unordered_set<std::string>&)> findNodes = [&findNodes](GraphModel* pModel, std::string node, std::unordered_set<std::string>& nodesName) {
        if (nodesName.count(node) || node.empty()) {
            return;
        } else {
            nodesName.insert(node);
        }
        const QModelIndex& idx = pModel->indexFromName(QString::fromStdString(node));
        if (ParamsModel* paramModel = QVariantPtr<ParamsModel>::asPtr(idx.data(QtRole::ROLE_PARAMS))) {
            for (int i = 0; i < paramModel->rowCount(); ++i) {
                QModelIndex idx = paramModel->index(i, 0);
                auto name = idx.data(QtRole::ROLE_PARAM_NAME).toString();
                PARAM_LINKS links = idx.data(QtRole::ROLE_LINKS).value<PARAM_LINKS>();
                for (auto link : links) {
                    zeno::EdgeInfo edge = link.data(QtRole::ROLE_LINK_INFO).value<zeno::EdgeInfo>();
                    if (idx.data(QtRole::ROLE_ISINPUT).toBool()) {
                        findNodes(pModel, edge.outNode, nodesName);
                    } else {
                        findNodes(pModel, edge.inNode, nodesName);
                    }
                }
            }
        }
    };
    findNodes(pModel, node.toStdString(), nodesName);
    QStringList list;
    for (auto& i : nodesName) {
        list.append(QString::fromStdString(i));
    }
    return list;
}

int UiHelper::getIndegree(const QModelIndex& nodeIdx)
{
    if (!nodeIdx.isValid())
        return 0;
    ParamsModel* paramsM = QVariantPtr<ParamsModel>::asPtr(nodeIdx.data(QtRole::ROLE_PARAMS));
    ZASSERT_EXIT(paramsM, 0);
    int inDegrees = 0, outDegrees = 0;
    paramsM->getDegrees(inDegrees, outDegrees);
    return inDegrees;
}

PANEL_TYPE UiHelper::title2Type(const QString& title)
{
    PANEL_TYPE type = PANEL_EMPTY;
    if (title == QObject::tr("Parameter") || title == "Parameter") {
        type = PANEL_NODE_PARAMS;
    }
    else if (title == QObject::tr("View") || title == "View" || title == QObject::tr("Scene Viewport") || title == "Scene Viewport") {
        type = PANEL_GL_VIEW;
    }
    else if (title == QObject::tr("Editor") || title == "Editor" || title == QObject::tr("Node Editor") || title == "Node Editor") {
        type = PANEL_EDITOR;
    }
    else if (title == QObject::tr("Data") || title == "Data" || title == QObject::tr("Spreadsheet") || title == "Spreadsheet") {
        type = PANEL_NODE_DATA;
    }
    else if (title == QObject::tr("Geometry Data") || title == "Geometry Data") {
        type = PANEL_GEOM_DATA;
    }
    else if (title == QObject::tr("Logger") || title == "Logger" || title == QObject::tr("Log") || title == "Log") {
        type = PANEL_LOG;
    }
    else if (title == QObject::tr("QML Opengl")) {
        type = PANEL_QML_GLVIEW;
    }
    else if (title == QObject::tr("QML Panel"))
    {
        type = PANEL_QMLPANEL;
    }
    else if (title == QObject::tr("Light") || title == "Light") {
        type = PANEL_LIGHT;
    }
    else if (title == QObject::tr("Image") || title == "Image") {
        type = PANEL_IMAGE;
    }
    else if (title == QObject::tr("Optix") || title == "Optix") {
        type = PANEL_OPTIX_VIEW;
    }
    else if (title == QObject::tr("Command Params") || title == "Command Params") {
        type = PANEL_COMMAND_PARAMS;
    }
    else if (title == QObject::tr("Open Path") || title == "Open Path") {
        type = PANEL_OPEN_PATH;
    }
    return type;
}

QString UiHelper::getTypeNameFromRtti(zeno::ParamType type)
{
    QString typeStr;
    if (type == Param_Null) {
        typeStr = "null";
        return typeStr;
    } else if (type == Param_Wildcard) {
        typeStr = "paramWildcard";
        return typeStr;
    } else if (type == Obj_Wildcard) {
        typeStr = "objWildcard";
        return typeStr;
    }
    else {
        const zeno::reflect::RTTITypeInfo& typeInfo = zeno::reflect::ReflectionRegistry::get().getRttiMap()->get(type);
        std::vector<std::regex> patterns = {
            std::regex(R"(std::shared_ptr\s*<\s*(?:const)?\s*struct\s*zeno::(.*?)\s*>)"),    //��ȡobj��
            std::regex(R"(struct\s*zeno::_impl_vec::(vec\s*<\s*[^>]+\s*>))"),   //��ȡvec��
            std::regex(R"(std::basic_(.*?)\s*<\s*char\s*>)")                    //��ȡstring
        };
        std::string rttiname = typeInfo.name();
        typeStr = QString::fromStdString(rttiname);
        for (const auto& pattern : patterns) {
            std::smatch match;
            if (std::regex_search(rttiname, match, pattern)) {
                typeStr = QString::fromStdString(match[1].str());
                break;
            }
        }
    }
    return typeStr;
}