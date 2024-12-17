import QtQuick                   2.12
import QtQuick.Controls          2.1
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts           1.3


Pane {
    id: comp
    property var node: undefined
    property var nodeItem: undefined
    property bool use_tabview: false
    implicitWidth: base_tabview.implicitWidth
    implicitHeight: base_tabview.implicitHeight;
    padding: 0

    onNodeChanged: nodeItem = node ? node.item : undefined

    Frame {
        id: base_tabview
        anchors.fill: parent
        property var treemodel: comp.node.params.customParamModel()
        property var tabsindex: treemodel.index(0, 0)

        //为了调试方便，只拿第一个tab
        ZPropPanel_Tab {
            id: ptab
            anchors.fill: parent

            model: base_tabview.treemodel
            parentIndex: model.index(0, 0, base_tabview.tabsindex)
            childCount: model.rowCount(parentIndex)

            Component.onCompleted: {
                
            }
        }
    }

    /*
    Frame {
        id: base_notabview
        ColumnLayout {
            Label {
                text: comp.node ? "Editing node <b>" + comp.node.label + "</b>": "Select a node..."
            }
            CheckBox {
                text: "Draggable"
                enabled: comp.nodeItem !== undefined
                checked: comp.nodeItem ? comp.nodeItem.draggable : false
                onClicked: comp.nodeItem.draggable = checked
            }
            CheckBox {
                text: "Resizable"
                enabled: comp.nodeItem !== undefined
                checked: comp.nodeItem ? comp.nodeItem.resizable : false
                onClicked: comp.nodeItem.resizable = checked
            }
            CheckBox {
                text: "Selected (read-only)"
                enabled: false
                checked: comp.nodeItem ? comp.nodeItem.selected : false
            }
            CheckBox {
                text: "Selectable"
                enabled: comp.nodeItem != null
                checked: comp.nodeItem ? comp.nodeItem.selectable : false
                onClicked: comp.nodeItem.selectable = checked
            }
            Label { text: "style.backRadius" }
            Slider {
                from: 0.; to: 15.0;
                value: defaultNodeStyle.backRadius
                stepSize: 1.0
                onMoved: defaultNodeStyle.backRadius = value
            }
        }
    }
    */
}