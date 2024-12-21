import QtQuick                   2.12
import QtQuick.Controls          2.15
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts           1.3


Pane {
    id: comp
    property var node: undefined
    property var nodeItem: undefined
    property bool use_tabview: false
    //implicitWidth: panel_loader.implicitWidth
    //implicitHeight: panel_loader.implicitHeight
    implicitWidth: 300
    implicitHeight: 300
    width: 300
    height: 300
    padding: 0

    onNodeChanged: nodeItem = node ? node.item : undefined

    Component {
        id: compBlank
        Rectangle {
            anchors.fill: parent
            width: 200
            height: 200
            color: "red"
        }
    }

    Component {
        id: compPropertyPane
        Frame {
            id: base_tabview
            anchors.fill: parent
            property var treemodel: comp.node ? comp.node.params.customParamModel() : undefined
            property var tabsindex: treemodel ? treemodel.index(0, 0) : undefined

            ColumnLayout {
                id: tablayout
                anchors.fill: parent

                TabBar {
                    id: property_tabbar
                    property int tabcount: base_tabview.treemodel.rowCount(base_tabview.tabsindex)
                    visible: tabcount > 1
                    Repeater {
                        model: property_tabbar.tabcount
                        delegate: TabButton {
                            property var mtabindex: base_tabview.treemodel.index(index, 0, base_tabview.tabsindex)
                            text: base_tabview.treemodel.data(mtabindex)
                            width: 100
                        }
                    }
                }

                StackLayout {
                    id: panel_stack
                    currentIndex: property_tabbar.currentIndex

                    Repeater {
                        model: base_tabview.treemodel.rowCount(base_tabview.tabsindex)
                        delegate: ScrollView {
                            clip: true
                            ZPropPanel_Tab {
                                model: base_tabview.treemodel
                                parentIndex: model.index(index, 0, base_tabview.tabsindex)
                                childCount: model.rowCount(parentIndex)
                            }
                        }
                    }
                }
            }
        }
    }

    Loader {
        id: panel_loader
        anchors.fill: parent

        sourceComponent: {
            return comp.nodeItem ? compPropertyPane : compBlank
        }
    }
}