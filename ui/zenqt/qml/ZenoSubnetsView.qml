import QtQuick                   2.12
import QtQuick.Controls          2.15
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts           1.3
import QtQuick.Shapes            1.0
import QtQuick.Controls.Styles   1.4


StackLayout {
    id: stack_subnet_views
    property variant root_graphmodel

    ZenoGraphView {
        graphModel: root_graphmodel
    }

    function jumpTo() {
        //TODO:
        console.log("stack_subnet_views.jumpTo");
    }
}