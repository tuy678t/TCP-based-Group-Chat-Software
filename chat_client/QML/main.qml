import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

import DataModel 1.0

ApplicationWindow {
    id: root
    visible: true
    width: 700
    height: 500
    title: qsTr("Chat Client")

    minimumHeight: 400
    minimumWidth: 400

    property bool isLogin: false
    property bool isConnected: false

    TcpClient {
        id: backEnd
        onConnected: {
            isConnected = true
            stackView.push(loginPage)
        }
        onConnectFailed: {
            popup(qsTr("connection failed"))
        }
        onDisconnected: {
            isConnected = false
            stackView.pop()
            busyIndicator.close()
            if (isLogin) {
                popup(qsTr("disconnected"))
                isLogin = false
            }
            //Qt.quit()
        }
        onLoginSuccess: {
            isLogin = true
            busyIndicator.close()
            stackView.pop()
            stackView.push(mainPage)
        }
        onNeedChangePassword: {
            isLogin = true
            busyIndicator.close()
            stackView.pop()
            stackView.push(changePwdPage)
        }
        onLogoutOthers: {
            isLogin = true
            busyIndicator.close()
            stackView.pop()
            stackView.push(mainPage)
            popup(qsTr("others logged out"))
        }
        onLoginFailed: {
            busyIndicator.close()
            popup(qsTr("login failed"))
        }
        onLogoutByOthers: {
            popup(qsTr("logged out by others"))
            isLogin = false
        }
        onPasswordChangedFail: {
            busyIndicator.close()
            popup(qsTr("change password failed"))
        }
        onPasswordChangedSucc: {
            busyIndicator.close()
            stackView.pop()
            stackView.push(mainPage)
        }
    }

    StackView {
        id: stackView
        initialItem: busyPage
        anchors.fill: parent
        Page {
            id: busyPage
            BusyIndicator {
                anchors.centerIn: parent
            }
        }
    }

    Component {
        id: loginPage
        LoginPage {
            onLogin: {
                backEnd.login(username, password)
                busyIndicator.open()
            }
        }
    }

    Popup {
        id: busyIndicator
        anchors.centerIn: parent
        modal: true
        closePolicy: Popup.NoAutoClose
        BusyIndicator {
            anchors.centerIn: parent
        }
    }

    Dialog {
        id: systemDialog
        property alias text: systemMsg.text

        anchors.centerIn: parent
        modal: true
        standardButtons: Dialog.Ok
        title: qsTr("System Message")
        Text {
            id: systemMsg
            text: "text"
        }
        onClosed: {
            if (!isConnected) {
                print("retry connect")
                backEnd.connect2Host()
            }
        }
    }

    Component {
        id: changePwdPage
        ChangePwdPage {
            onChangePwd: {
                backEnd.changePassword(oldPwd, newPwd)
                busyIndicator.open()
            }
        }
    }

    Component {
        id: mainPage
        RowLayout {
            spacing: 0

            ContactPage {
                id: contactPage
                model: backEnd.contactsModel
                Layout.fillHeight: true
                Material.theme: Material.Dark
                onUidChanged: {
                    backEnd.setUid(uid)
                }
            }

            ConversationPage {
                id: conversationPage
                model: backEnd.recordModel

                uid: contactPage.uid
                name: contactPage.name

                Layout.fillHeight: true
                Layout.fillWidth: true

                Material.theme: Material.Light
//                headerColor: Material.color(Material.Indigo)
//                messageColor: Material.color(Material.Indigo, Material.Shade400)
                buttonColor: Material.color(Material.Teal)
                headerColor: backEnd.theme1
                messageColor: backEnd.theme2
//                buttonColor: backEnd.theme3

                onSendTxt: {
                    backEnd.sendMessage(txt)
                }
            }
        }
    }

    function popup(str) {
        systemDialog.text = str
        systemDialog.open()
        systemDialog.focus = true
    }
}
