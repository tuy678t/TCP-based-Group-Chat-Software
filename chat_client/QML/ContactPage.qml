import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
//import QtQuick.Controls.Material 2.12

Page {
    id: root

    property var model
    property int uid
    property string name

    implicitWidth: 200

//    header: ToolBar {
//        Material.background: "black"
//        Label {
//            id: pageTitle
//            text: qsTr("Contact")
//            font.pixelSize: 20
//            anchors.centerIn: parent
//        }
//    }

    ListView {
        id: listView
        anchors.fill: parent
        model: root.model
        currentIndex: -1
        delegate: ItemDelegate {
            text: model.name + (model.is_offline ? qsTr("(offline)") : "")
            opacity: model.is_offline ? 0.5 : 1
            width: parent.width
            highlighted: ListView.isCurrentItem
            onClicked: {
                listView.currentIndex = index
                root.uid = model.uid
                root.name = text
            }
        }
        ScrollBar.vertical: ScrollBar {
            width: 10
        }
    }
}
