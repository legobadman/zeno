#include "pluginsmodel.h"
#include "uicommon.h"
#include "settings/zsettings.h"
#include "zenoapplication.h"
#include "zenomainwindow.h"
#include "zassert.h"
#include <zeno/core/Session.h>
#include <zeno/core/NodeRegister.h>


PluginsModel::PluginsModel(QObject* parent)
    : QAbstractListModel(parent)
{
    QSettings settings(QSettings::UserScope, zsCompanyName, zsEditor);
    settings.beginGroup("Zeno Plugins");
    QStringList lst = settings.childKeys();
#ifdef _WIN32
    if (lst.indexOf("zs_base.dll") == -1) {
        lst.push_front("zs_base.dll");
    }
#else
    if (lst.indexOf("libzs_base.so") == -1) {
        lst.push_front("libzs_base.so");
    }
#endif

    for (int i = 0; i < lst.size(); i++)
    {
        const QString& key = lst[i];
        QString path = settings.value(key).toString();
        QString appDir = zenoApp->applicationDirPath();
        //if (path.isEmpty())
        {
            //目前加载本地build目录，暂时不考虑外部文件
            path = appDir + '/' + key;
        }
        QFileInfo fninfo(path);
        QString dllName = fninfo.fileName();
        path = appDir + '/' + dllName;

        _pluginItem _item;
        _item.path = path;
        _item.bLoaded = fninfo.exists();
        if (_item.bLoaded) {
            zeno::getNodeRegister().beginLoadModule(path.toStdString());
            zeno::scope_exit sp([&]() { zeno::getNodeRegister().endLoadModule(); });
#ifdef _WIN32
            _item.hDll = LoadLibrary(path.toUtf8().data());
            ZASSERT_EXIT(_item.hDll != INVALID_HANDLE_VALUE);
#else
            _item.hDll = dlopen(path.toUtf8().data(), RTLD_NOW | RTLD_LOCAL);
#endif
            m_items.append(_item);
        }
    }
}

PluginsModel::~PluginsModel()
{
    for (auto _item : m_items) {
        #ifdef _WIN32
        FreeLibrary(_item.hDll);
        #else
        dlclose(_item.hDll);
        #endif
    }
    m_items.clear();
}

void PluginsModel::removePlugin(int row) {
    removeRows(row, 1);
}

int PluginsModel::rowCount(const QModelIndex& parent) const {
    return m_items.size();
}

QVariant PluginsModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_items.size()) {
        return QVariant();
    }
    if (role == Qt::DisplayRole) {
        QFileInfo fileInfo(m_items[index.row()].path);
        return fileInfo.fileName();
    }
    else if (role == QtRole::ROLE_PLUGIN_PATH) {
        return m_items[index.row()].path;
    }
    else if (role == QtRole::ROLE_PLUGIN_LOADED) {
        return m_items[index.row()].bLoaded;
    }
    else {
        return QVariant();
    }
}

bool PluginsModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    return false;
}

QModelIndexList PluginsModel::match(
    const QModelIndex& start,
    int role,
    const QVariant& value,
    int hits,
    Qt::MatchFlags flags) const {
    return QModelIndexList();
}

static QString getFileName(const QString& path) {
    int slashIndex = path.lastIndexOf('/');
    int backslashIndex = path.lastIndexOf('\\');
    int lastSeparator = qMax(slashIndex, backslashIndex);
    return path.mid(lastSeparator + 1);
}

bool PluginsModel::removeRows(int row, int count, const QModelIndex& parent) {

    beginRemoveRows(parent, row, row);

    auto& nodeReg = zeno::getNodeRegister();
    auto ptr = nodeReg.getNodeClassPtr("erode_noise_perlin_GEO");
#ifdef _WIN32
    bool ret = FreeLibrary(m_items[row].hDll);
#else
    dlclose(m_items[row].hDll);
#endif
    //先从注册表移除
    const QString& filePath = m_items[row].path;
    QFileInfo fn(filePath);
    QString name;
    if (fn.exists()) {
        name = QFileInfo(filePath).fileName();
    }
    else {
        //已经不存在了，只能从文件路径提取名字
        name = getFileName(filePath);
    }
    
    QSettings settings(QSettings::UserScope, zsCompanyName, zsEditor);
    settings.beginGroup("Zeno Plugins");
    settings.remove(name);

    nodeReg.uninstallModule(filePath.toStdString());

    m_items.removeAt(row);
    endRemoveRows();
    return true;
}

QHash<int, QByteArray> PluginsModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "name";
    roles[QtRole::ROLE_PLUGIN_PATH] = "path";
    roles[QtRole::ROLE_PLUGIN_LOADED] = "isLoaded";
    return roles;
}

void PluginsModel::addPlugin(const QString& filePath) {

    if (!filePath.isEmpty()) {
        #ifdef _WIN32
        HMODULE hDll = 0;
        #else
        void* hDll = nullptr;
        #endif
        {
            zeno::getNodeRegister().beginLoadModule(filePath.toStdString());
            zeno::scope_exit sp([&]() { zeno::getNodeRegister().endLoadModule(); });

            #ifdef _WIN32
            hDll = LoadLibrary(filePath.toUtf8().data());
            #else
            hDll = dlopen(filePath.toUtf8().data(), RTLD_NOW | RTLD_LOCAL);
            #endif
            //添加到注册表
            if (hDll) {
                QString name = QFileInfo(filePath).fileName();
                QSettings settings(QSettings::UserScope, zsCompanyName, zsEditor);
                settings.beginGroup("Zeno Plugins");
                settings.setValue(name, filePath);
            }
        }

        //还要考虑已经加载过的
        if (hDll) {
            int nRows = m_items.size();
            beginInsertRows(QModelIndex(), nRows, nRows);
            _pluginItem item;
            item.bLoaded = true;
            item.path = filePath;
            item.hDll = hDll;
            m_items.append(item);
            endInsertRows();
        }
        else {
            //TODO: 提示框
        }
    }
}