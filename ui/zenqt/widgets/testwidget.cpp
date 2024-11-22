#include "testwidget.h"
#include "widgets/zwidgetfactory.h"
#include "uicommon.h"
#include <zeno/core/typeinfo.h>


TestNormalWidget::TestNormalWidget()
{
    QVBoxLayout* pLayout = new QVBoxLayout;
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setBrush(QPalette::Background, QColor(49, 54, 72));
    setPalette(pal);

    QString type = "";
    std::vector<std::string> items = {"23.5 fps", "24 fps", "25 fps", "30 fps", "60 fps"};
    zeno::reflect::Any properties = items;

    CallbackCollection cbSet;

    QWidget* pWidget = zenoui::createWidget(QModelIndex(), "turnLeft", zeno::Combobox, Param_Null, cbSet, properties);
    pLayout->addWidget(pWidget);

    std::vector<float> sliderInfo = { 1.0, 100.0, 1.0 };
    properties = sliderInfo;

    QWidget* pSlider = zenoui::createWidget(QModelIndex(), 10, zeno::Slider, ui_gParamType_Int, cbSet, properties);
    pLayout->addWidget(pSlider);

    QWidget* pSpinBox = zenoui::createWidget(QModelIndex(), 10, zeno::SpinBoxSlider, ui_gParamType_Int, cbSet, properties);
    pLayout->addWidget(pSpinBox);

    setLayout(pLayout);
}