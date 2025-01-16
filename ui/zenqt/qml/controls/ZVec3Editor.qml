import QtQuick 2.12
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3


Item {
    id: root
    implicitWidth:  mainLayout.implicitWidth
    implicitHeight: mainLayout.implicitHeight
    property var value: ["0","0","0"]
    signal editingFinished

    function get_value() {
        var vec = []
        vec.push(xedit.text)
        vec.push(yedit.text)
        vec.push(zedit.text)
        return vec
    }

    RowLayout {
        id: mainLayout
        anchors.fill: parent
        spacing: 10

        VecEdit {
            id: xedit
            text: value[0]
            onEditingFinished: {
                root.editingFinished()
            }
        }
        VecEdit {
            id: yedit
            text: value[1]
            onEditingFinished: {
                root.editingFinished()
            }    
        }
        VecEdit {
            id: zedit
            text: value[2]
            onEditingFinished: {
                root.editingFinished()
            }    
        }
    }
}
