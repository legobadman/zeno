import QtQuick 2.12
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4


Item {
    id: root
    property var graphModel: undefined
    property var graphView: undefined

    Repeater {
        model: root.graphModel.getLinkModel()
        delegate: Text {
            required property var fromParam
            required property var toParam

            text: '(' + fromParam[0] + ',' + fromParam[1] + ') -> (' + toParam[0] + ',' + toParam[1] + ')';
            color: root.graphView.resizeHandlerColor
        }
    }
}