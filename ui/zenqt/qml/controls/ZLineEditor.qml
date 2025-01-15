import QtQuick 2.12
import QtQuick.Controls 2.0

Item {
    id: container
    width: 120
    height: 24
    //anchors.centerIn: parent

    property alias text: inputField.text

    signal editingFinished

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.color: inputField.activeFocus ? "blue" : "gray"  // 焦点时改变边框颜色
        border.width: 1
    }

    TextInput {
        id: inputField
        anchors.fill: parent
        // anchors.centerIn: parent
        verticalAlignment: Text.AlignVCenter
        // width: inputField.width
        // height: inputField.height

        topPadding: 5
        bottomPadding: 5
        leftPadding: 5
        rightPadding: 5
        font.pixelSize: 14
        color: "white"
        focus: true // 确保输入框可以获得焦点
        selectByMouse: true // 启用鼠标选择文本功能
        clip: true

        onTextChanged: {
            //console.log("Text changed:", text)
        }

        Keys.onReturnPressed: {
                // 按下回车键时失去焦点
            focus = false;
        }

        onEditingFinished: {
            //暂时不判断value是否有更改，留给model层或内核层去处理
            container.editingFinished()
        }
    }
}