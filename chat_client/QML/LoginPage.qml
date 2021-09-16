import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

Page {
    id: root
    signal login(string username, string password)

    Column {
        anchors.centerIn: parent
        spacing: 10

        TextField {
            id: userTextField
            width: 150
            anchors.horizontalCenter: parent.horizontalCenter
            placeholderText: qsTr("Username")
            maximumLength: 28
        }
        TextField {
            id: pwdTextField
            width: 150
            anchors.horizontalCenter: parent.horizontalCenter
            placeholderText: qsTr("Password")
            maximumLength: 28
            echoMode: TextInput.Password
            onAccepted: button.clicked()
        }
        Button {
            id: button
            text: qsTr("Login")
            anchors.horizontalCenter: parent.horizontalCenter
            highlighted: true
            enabled: userTextField.text && pwdTextField.text
            onClicked: root.login(userTextField.text, pwdTextField.text)
        }
    }
}
