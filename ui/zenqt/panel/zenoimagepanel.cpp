//
// Created by zh on 2023/3/23.
//
#if 1

#include "zenoimagepanel.h"
#include "viewport/zenovis.h"
#include "zeno/utils/format.h"
#include <zeno/types/UserData.h>
#include <zeno/core/ZNode.h>
#include <zeno/types/PrimitiveObject.h>
#include "zeno/utils/vec.h"
#include "zeno/utils/log.h"
#include "zenoapplication.h"
#include "zassert.h"
#include "viewport/viewportwidget.h"
#include "zenomainwindow.h"
#include "viewport/displaywidget.h"


void ZenoImagePanel::clear() {
    if (image_view) {
        image_view->clearImage();
    }
    pPrimName->clear();
    pStatusBar->clear();
}

void ZenoImagePanel::setObject(zeno::IObject2* pObject) {
    if (!pObject)
        return;
    auto ud = pObject->userData();
    if (!ud || ud->get_int("isImage", 0) == 0 || ud->get_bool("isImage", false) == false) {
        return;
    }
    bool enableGamma = pGamma->checkState() == Qt::Checked;
    int width = ud->get_int("w");
    int height = ud->get_int("h");

    if (pObject->type() == zeno::ZObj_Image) {
        if (width <= 0 || height <= 0 || !image_view)
            return;
        int channels = ud->get_int("channels", 3);
        const size_t pixelCount = static_cast<size_t>(width) * height * channels;
        std::vector<float> pixels(pixelCount);
        size_t got = ud->get_float_arr("pixels", pixels.data(), pixelCount);
        if (got == 0)
            return;
        if (pMode->currentText() != "Alpha") {
            auto index = std::map<QString, zeno::vec3i>{
                {"RGB", {0, 1, 2}},
                {"Red", {0, 0, 0}},
                {"Green", {1, 1, 1}},
                {"Blue", {2, 2, 2}},
            }.at(pMode->currentText());
            QImage img(width, height, QImage::Format_RGB32);
            for (int i = 0; i < width * height; i++) {
                int h = i / width;
                int w = i % width;
                float c[3] = {
                    pixels[i * channels + (index[0] < channels ? index[0] : 0)],
                    pixels[i * channels + (index[1] < channels ? index[1] : 0)],
                    pixels[i * channels + (index[2] < channels ? index[2] : 0)]
                };
                if (enableGamma) {
                    c[0] = zeno::pow(c[0], 1.0f / 2.2f);
                    c[1] = zeno::pow(c[1], 1.0f / 2.2f);
                    c[2] = zeno::pow(c[2], 1.0f / 2.2f);
                }
                int r = glm::clamp(int(c[0] * 255.99f), 0, 255);
                int g = glm::clamp(int(c[1] * 255.99f), 0, 255);
                int b = glm::clamp(int(c[2] * 255.99f), 0, 255);
                img.setPixel(w, height - 1 - h, qRgb(r, g, b));
            }
            img = img.mirrored(true, false);
            image_view->setImage(img);
        } else if (pMode->currentText() == "Alpha" && channels >= 4) {
            QImage img(width, height, QImage::Format_RGB32);
            for (int i = 0; i < width * height; i++) {
                int h = i / width;
                int w = i % width;
                float a = pixels[i * channels + 3];
                int v = glm::clamp(int(a * 255.99f), 0, 255);
                img.setPixel(w, height - 1 - h, qRgb(v, v, v));
            }
            img = img.mirrored(true, false);
            image_view->setImage(img);
        }
        pStatusBar->setText(QString(zeno::format("width: {}, height: {}", width, height).c_str()));
        return;
    }

    if (auto geom = dynamic_cast<zeno::GeometryObject*>(pObject)) {
        auto obj = geom->toPrimitive();
        if (image_view) {
            if (pMode->currentText() != "Alpha") {
                QImage img(width, height, QImage::Format_RGB32);
                auto index = std::map<QString, zeno::vec3i>{
                    {"RGB", {0, 1, 2}},
                    {"Red", {0, 0, 0}},
                    {"Green", {1, 1, 1}},
                    {"Blue", {2, 2, 2}},
                }.at(pMode->currentText());
                for (auto i = 0; i < obj->verts.size(); i++) {
                    int h = i / width;
                    int w = i % width;
                    auto c = obj->verts[i];
                    if (enableGamma) {
                        c = zeno::pow(c, 1.0f / 2.2f);
                    }
                    int r = glm::clamp(int(c[index[0]] * 255.99), 0, 255);
                    int g = glm::clamp(int(c[index[1]] * 255.99), 0, 255);
                    int b = glm::clamp(int(c[index[2]] * 255.99), 0, 255);

                    img.setPixel(w, height - 1 - h, qRgb(r, g, b));
                }
                img = img.mirrored(true, false);
                image_view->setImage(img);
            }
            else if (pMode->currentText() == "Alpha") {
                QImage img(width, height, QImage::Format_RGB32);
                if (obj->verts.has_attr("alpha")) {
                    auto& alpha = obj->verts.attr<float>("alpha");
                    for (auto i = 0; i < obj->verts.size(); i++) {
                        int h = i / width;
                        int w = i % width;
                        auto c = alpha[i];
                        int r = glm::clamp(int(c * 255.99), 0, 255);
                        int g = glm::clamp(int(c * 255.99), 0, 255);
                        int b = glm::clamp(int(c * 255.99), 0, 255);

                        img.setPixel(w, height - 1 - h, qRgb(r, g, b));
                    }
                }
                image_view->setImage(img);
            }
        }
        QString statusInfo = QString(zeno::format("width: {}, height: {}", width, height).c_str());
        pStatusBar->setText(statusInfo);
    }
}

void ZenoImagePanel::reload(const zeno::render_reload_info& info) {
    return;
    m_info = info;
    bool enableGamma = pGamma->checkState() == Qt::Checked;
    const auto& update = info.objs[0];
    if (update.reason == zeno::Update_Remove) {
        for (const std::string& remkey : update.remove_objs) {
        }
    }
    else {
        auto spNode = zeno::getSession().getNodeByUuidPath(update.uuidpath_node_objkey);
        assert(spNode);
        auto spObject = spNode->getNodeParams().get_default_output_object();
        if (spObject) {
            if (update.reason == zeno::Update_View) {
                auto ud = spObject->userData();
                if (ud->get_int("isImage", 0) == 0 || ud->get_bool("isImage", false) == false) {
                    return;
                }
                if (auto geom = dynamic_cast<zeno::GeometryObject*>(spObject)) {
                    auto obj = geom->toPrimitive();
                    int width = ud->get_int("w");
                    int height = ud->get_int("h");
                    if (image_view) {
                        if (pMode->currentText() != "Alpha") {
                            QImage img(width, height, QImage::Format_RGB32);
                            auto index = std::map<QString, zeno::vec3i>{
                                {"RGB", {0, 1, 2}},
                                {"Red", {0, 0, 0}},
                                {"Green", {1, 1, 1}},
                                {"Blue", {2, 2, 2}},
                            }.at(pMode->currentText());
                            for (auto i = 0; i < obj->verts.size(); i++) {
                                int h = i / width;
                                int w = i % width;
                                auto c = obj->verts[i];
                                if (enableGamma) {
                                    c = zeno::pow(c, 1.0f / 2.2f);
                                }
                                int r = glm::clamp(int(c[index[0]] * 255.99), 0, 255);
                                int g = glm::clamp(int(c[index[1]] * 255.99), 0, 255);
                                int b = glm::clamp(int(c[index[2]] * 255.99), 0, 255);

                                img.setPixel(w, height - 1 - h, qRgb(r, g, b));
                            }
                            image_view->setImage(img);
                        }
                        else if (pMode->currentText() == "Alpha") {
                            QImage img(width, height, QImage::Format_RGB32);
                            if (obj->verts.has_attr("alpha")) {
                                auto& alpha = obj->verts.attr<float>("alpha");
                                for (auto i = 0; i < obj->verts.size(); i++) {
                                    int h = i / width;
                                    int w = i % width;
                                    auto c = alpha[i];
                                    int r = glm::clamp(int(c * 255.99), 0, 255);
                                    int g = glm::clamp(int(c * 255.99), 0, 255);
                                    int b = glm::clamp(int(c * 255.99), 0, 255);

                                    img.setPixel(w, height - 1 - h, qRgb(r, g, b));
                                }
                            }
                            image_view->setImage(img);
                        }
                    }
                    QString statusInfo = QString(zeno::format("width: {}, height: {}", width, height).c_str());
                    pStatusBar->setText(statusInfo);
                }
            }
        }
    }
}

ZenoImagePanel::ZenoImagePanel(QWidget *parent) : QWidget(parent) {
    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->setContentsMargins(QMargins(0, 0, 0, 0));
    setLayout(pMainLayout);
    setFocusPolicy(Qt::ClickFocus);

    QPalette palette = this->palette();
    palette.setBrush(QPalette::Window, QColor(37, 37, 38));
    setPalette(palette);
    setAutoFillBackground(true);

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    QHBoxLayout* pTitleLayout = new QHBoxLayout;

    QLabel* pPrim = new QLabel(tr("Prim: "));
    pPrim->setProperty("cssClass", "proppanel");
    pTitleLayout->addWidget(pPrim);

    pPrimName->setProperty("cssClass", "proppanel");
    pTitleLayout->addWidget(pPrimName);

    pGamma->setStyleSheet("color: white;");
    pGamma->setCheckState(Qt::Checked);
    pTitleLayout->addWidget(pGamma);

    pMode->addItem("RGB");
    pMode->addItem("Red");
    pMode->addItem("Green");
    pMode->addItem("Blue");
    pMode->addItem("Alpha");
    pMode->setCurrentIndex(0);
    pTitleLayout->addWidget(pMode);

    pFit->setProperty("cssClass", "grayButton");
    pTitleLayout->addWidget(pFit);

    pMainLayout->addLayout(pTitleLayout);

    image_view = new ZenoImageView(this);
    image_view->setProperty("cssClass", "proppanel");
    pMainLayout->addWidget(image_view);

    pStatusBar->setProperty("cssClass", "proppanel");
    QFont font("Consolas", 10);
    pStatusBar->setFont(font);
    pStatusBar->setText("PlaceHolder");


    pMainLayout->addWidget(pStatusBar);

    auto mainWin = zenoApp->getMainWindow();
    ZASSERT_EXIT(mainWin);
    QVector<DisplayWidget*> wids = mainWin->viewports();
    if (wids.isEmpty())
        return;

    Zenovis* zenovis = wids[0]->getZenoVis();
    if (!zenovis)
        return;

    connect(pGamma, &QCheckBox::stateChanged, this, [=](int state) {
        reload(m_info);
    });
    connect(pMode, &ZComboBox::_textActivated, [=](const QString& text) {
        reload(m_info);
    });

    connect(pFit, &QPushButton::clicked, this, [=](bool _) {
        image_view->fitMode = true;
        image_view->updateImageView();
    });
    connect(image_view, &ZenoImageView::pixelChanged, this, [=](float x, float y) {
        std::string primid = pPrimName->text().toStdString();
        zenovis::Scene* scene = nullptr;
        auto mainWin = zenoApp->getMainWindow();
        ZASSERT_EXIT(mainWin);
        QVector<DisplayWidget*> wids = mainWin->viewports();
        if (!wids.isEmpty())
        {
            auto session = wids[0]->getZenoVis()->getSession();
            ZASSERT_EXIT(session);
            scene = session->get_scene();
        }
        if (!scene)
            return;

        bool found = false;
        const auto& update = m_info.objs[0];
        auto spNode = zeno::getSession().getNodeByUuidPath(update.uuidpath_node_objkey);
        assert(spNode);
        auto spObject = spNode->getNodeParams().get_default_output_object();
        if (!spObject) return;

        auto ud = spObject->userData();
        if (!ud || ud->get_int("isImage", 0) == 0 || ud->get_bool("isImage", false) == false) {
            return;
        }
        found = true;
        int width = ud->get_int("w");
        int height = ud->get_int("h");
        int w = int(zeno::clamp(x, 0, width - 1));
        int h = int(zeno::clamp(y, 0, height - 1));
        int i = (height - 1 - h) * width + w;

        if (spObject->type() == zeno::ZObj_Image) {
            int channels = ud->get_int("channels", 3);
            std::vector<float> pixels(static_cast<size_t>(width) * height * channels);
            size_t got = ud->get_float_arr("pixels", pixels.data(), pixels.size());
            if (got > 0 && i * channels + 2 < static_cast<int>(got)) {
                float c0 = pixels[i * channels + 0];
                float c1 = pixels[i * channels + 1];
                float c2 = pixels[i * channels + 2];
                std::string info = zeno::format("width: {}, height: {}", width, height);
                info += zeno::format(" | x: {:5}, y: {:5}", w, h);
                if (channels >= 4) {
                    float a = pixels[i * channels + 3];
                    info += zeno::format(" | value: {}, {}, {}, {}",
                        QString::number(c0, 'f', 6).toStdString(),
                        QString::number(c1, 'f', 6).toStdString(),
                        QString::number(c2, 'f', 6).toStdString(),
                        QString::number(a, 'f', 6).toStdString());
                } else {
                    info += zeno::format(" | value: {}, {}, {}",
                        QString::number(c0, 'f', 6).toStdString(),
                        QString::number(c1, 'f', 6).toStdString(),
                        QString::number(c2, 'f', 6).toStdString());
                }
                pStatusBar->setText(QString(info.c_str()));
            }
        } else if (auto geom = dynamic_cast<zeno::GeometryObject*>(spObject)) {
            auto obj = geom->toPrimitive();
            auto c = obj->verts[i];
            std::string info = zeno::format("width: {}, height: {}", width, height);
            info += zeno::format(" | x: {:5}, y: {:5}", w, h);
            if (obj->verts.has_attr("alpha")) {
                auto &alpha = obj->verts.attr<float>("alpha");
                info += zeno::format(
                    " | value: {}, {}, {}, {}",
                    QString::number(c[0], 'f', 6).toStdString(),
                    QString::number(c[1], 'f', 6).toStdString(),
                    QString::number(c[2], 'f', 6).toStdString(),
                    QString::number(alpha[i], 'f', 6).toStdString()
                );
            }
            else {
                info += zeno::format(
                    " | value: {}, {}, {}",
                    QString::number(c[0], 'f', 6).toStdString(),
                    QString::number(c[1], 'f', 6).toStdString(),
                    QString::number(c[2], 'f', 6).toStdString()
                );
            }
            pStatusBar->setText(QString(info.c_str()));
        }

    if (found == false) {
        clear();
    }
    });
}

#endif