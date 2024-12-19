import QtQuick 2.12
import QtQuick.Controls 2.0

Item{
    width: 64
    height: 32

    property alias text: textInput.text

    TextField{
        id : textInput
        //anchors.margins: 0
        anchors.fill: parent
        height: 32
        verticalAlignment: Text.AlignVCenter
        clip:true
        padding: 0

        //color: "#FFF"
        selectionColor: "#0078D7"
        font.pointSize: 10
        font.family: "Consolas"

        focus: true
        selectByMouse: true
        Keys.onEscapePressed: focus = false

        background: Rectangle {
            color: "white"
            border.color: textInput.activeFocus ? "blue" : "gray"  // 焦点时改变边框颜色
            border.width: 1
            radius: 0
        }
    }
}