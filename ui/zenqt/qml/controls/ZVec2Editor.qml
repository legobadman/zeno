import QtQuick 2.12
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3


Item {
    implicitWidth:  mainLayout.implicitWidth
    implicitHeight: mainLayout.implicitHeight
    property var value: [0,0]

    RowLayout {
        id: mainLayout
        anchors.fill: parent
        spacing: 10
        VecEdit { text: value[0] }
        VecEdit { text: value[1] }
    }
}

