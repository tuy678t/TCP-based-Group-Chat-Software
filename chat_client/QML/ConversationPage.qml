import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12
import QtQuick.Dialogs 1.3

Page {
    id: root

    property var model
    property int uid
    property string name

    property color headerColor
    property color messageColor
    property color buttonColor

    signal sendTxt(string txt)

    enabled: uid
    visible: uid

    header: ToolBar {
        Material.primary: headerColor
        Material.theme: Material.Dark
        Label {
            id: pageTitle
            text: root.name
            font.pixelSize: 20
            anchors.centerIn: parent
        }
    }

    onUidChanged: {
        messageField.text = ""
        listView.currentIndex = 0
    }

    ColumnLayout {
        anchors.fill: parent

        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            displayMarginBeginning: 40
            displayMarginEnd: 40
            verticalLayoutDirection: ListView.BottomToTop
            spacing: 12
            highlightFollowsCurrentItem: true

            model: root.model
            delegate: Column {
                anchors.left: model.sentByMe ? undefined : parent.left
                anchors.right: model.sentByMe ? parent.right : undefined
                anchors.margins: pane.padding
                spacing: 6

                Label {
                    id: nameText
                    text: model.name
                    color: "grey"
                    anchors.right: model.sentByMe ? parent.right : undefined
                }

                Rectangle {
                    width: Math.min(messageText.implicitWidth + 24, listView.width - 2*pane.padding)
                    height: messageText.implicitHeight + 24
                    color: model.sentByMe ? messageColor : "lightgrey"
                    Label {
                        id: messageText
                        text: model.txt
                        color: model.sentByMe ? "white" : "black"
                        anchors.fill: parent
                        anchors.margins: 12
                        wrapMode: Label.Wrap
                    }
                    anchors.right: model.sentByMe ? parent.right : undefined
                }

                Label {
                    id: timestampText
                    text: model.time
                    color: "lightgrey"
                    anchors.right: model.sentByMe ? parent.right : undefined
                }
            }

            ScrollBar.vertical: ScrollBar {
                width: 10
            }
        }

        Pane {
            id: pane
            Layout.fillWidth: true
            Material.background: "white"
            Material.accent: buttonColor
            Material.elevation: 2

            RowLayout {
                width: parent.width
                spacing: 10
                Button {
                    id: sendFileButton
                    visible: uid != 0xFFFF
                    Layout.alignment: Qt.AlignBottom
                    highlighted: true
                    text: qsTr("FILE")
                    onClicked: fileDialog.open()
                }

                TextArea {
                    id: messageField
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignBottom
                    placeholderText: qsTr("Compose message")
                    wrapMode: TextArea.Wrap
                }

                Button {
                    id: sendButton
                    Layout.alignment: Qt.AlignBottom
                    highlighted: true
                    text: qsTr("SEND")
                    enabled: messageField.length > 0
                    onClicked: {
                        if (messageField.length <= 1000) {
                            root.sendTxt(messageField.text)
                            messageField.text = ""
                            listView.currentIndex = 0
                        }
                        else {
                            print("message too long!")
                        }
                    }
                }
            }
        }
    }

    FileDialog {
        id: fileDialog
        onAccepted: {
            var filename = fileDialog.fileUrl
            print("sending ", filename)
            backEnd.sendFile(filename)
            backEnd.sendMessage(filename)
        }
    }
}
