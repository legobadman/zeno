import QtQuick 2.12
import QtQuick.Controls 2.0


Item{
    width: 96
    height: 20
    property alias text: textInput.text

    TextField{
        id : textInput
        //anchors.margins: 0
        anchors.fill: parent
        verticalAlignment: TextInput.AlignVCenter
        clip:true
        padding: 0

        color: "#000000"
        selectionColor: "#0078D7"
        //font.pointSize: 12
        font.family: "Consolas"

        focus: true
        selectByMouse: true
        Keys.onEscapePressed: focus = false

        background:Rectangle {
            id: backGround
            color: "white"
            border.color: "grey"
        }
    }
}