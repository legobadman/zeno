#include "colormanager.h"
#include <zeno/core/NodeRegister.h>


ZColorManager::ZColorManager() {
    initColorsFromCustom();
}

void ZColorManager::initColorsFromCustom()
{

}

QColor ZColorManager::getColorByType(zeno::ParamType type)
{
    std::string_view color, name;
    if (zeno::getNodeRegister().getObjUIInfo(type, color, name)) {
        return QColor(QString::fromLatin1(color.data()));
    }
    else {
        return QColor();
    }
}

