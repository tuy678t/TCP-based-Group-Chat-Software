import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

Page {
    id: root
    signal changePwd(string oldPwd, string newPwd)

    Column {
        anchors.centerIn: parent
        spacing: 10

        TextField {
            id: oldTextField
            width: 200
            anchors.horizontalCenter: parent.horizontalCenter
            placeholderText: qsTr("Old password")
            maximumLength: 28
            echoMode: TextInput.Password
        }
        TextField {
            id: newTextField
            width: 200
            anchors.horizontalCenter: parent.horizontalCenter
            placeholderText: qsTr("New password")
            maximumLength: 28
            echoMode: TextInput.Password
        }
        TextField {
            id: confirmTextField
            width: 200
            anchors.horizontalCenter: parent.horizontalCenter
            placeholderText: qsTr("Confirm new password")
            maximumLength: 28
            echoMode: TextInput.Password
            onAccepted: button.clicked()
        }
        Text {
            id: hintText
            opacity: newTextField.text != confirmTextField.text
                     || (oldTextField.text.length
                         && oldTextField.text == newTextField.text)
            text: newTextField.text
                  != confirmTextField.text ? qsTr("Passwords don't match!") : qsTr(
                                                 "New password can't be same as the old one")
            color: "red"
        }
        Button {
            id: button
            text: qsTr("Submit")
            anchors.horizontalCenter: parent.horizontalCenter
            highlighted: true
            enabled: oldTextField.text && newTextField.text
                     && newTextField.text == confirmTextField.text
            onClicked: root.changePwd(oldTextField.text, newTextField.text)
        }
    }
}
