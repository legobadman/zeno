import QtQuick                   2.12
import QtQuick.Controls          2.15
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts           1.3
import QtQuick.Controls.Styles   1.4
import zeno.enum 1.0


Item {
    id: comp

    property string socket_name;
    property int socket_group;
    property color bg_color;

    signal socketClicked()

    height: childrenRect.height
    width: childrenRect.width

    Rectangle {
        height: childrenRect.height
        width: childrenRect.width
        color: comp.bg_color

        Text {
            color: "black"
            text: comp.socket_name
        }
    }

    MouseArea {
        id: comp_mousearea
        anchors.fill: parent

        onClicked: {
            comp.socketClicked()
        }
    }
}