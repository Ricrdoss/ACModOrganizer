import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: updateNotificationDialog
    title: ""
    modal: true
    dim: true
    anchors.centerIn: parent
    width: Math.min(560, parent.width - 40)
    height: Math.min(480, parent.height - 40)
    padding: 0

    background: Rectangle {
        color: Theme.bgSurface
        border.color: Theme.borderDefault
        border.width: 1
        radius: Theme.radiusMd
        clip: true
    }

    contentItem: ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 1. Dialog Header
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 58
            color: Theme.bgCard
            border.color: Theme.borderDefault
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: Theme.space20
                anchors.rightMargin: Theme.space16
                spacing: Theme.space12

                Rectangle {
                    width: 32
                    height: 32
                    radius: Theme.radiusSm
                    color: Theme.bgSurface
                    border.color: Theme.borderDefault
                    border.width: 1

                    Image {
                        anchors.centerIn: parent
                        width: 18
                        height: 18
                        source: "qrc:/qt/qml/ACBO/ui/icons/zap_white.svg"
                        fillMode: Image.PreserveAspectFit
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 1

                    RowLayout {
                        spacing: Theme.space8
                        Text {
                            text: "New Update Available"
                            color: Theme.textPrimary
                            font.family: Theme.fontFamily
                            font.pixelSize: 15
                            font.weight: Font.DemiBold
                        }

                        Rectangle {
                            height: 20
                            radius: Theme.radiusXs
                            color: "#2563eb"
                            implicitWidth: newVerLabel.implicitWidth + 12

                            Text {
                                id: newVerLabel
                                anchors.centerIn: parent
                                text: appController.updateManager ? appController.updateManager.latestVersion : ""
                                color: "#ffffff"
                                font.family: Theme.fontFamilyMono
                                font.pixelSize: 11
                                font.weight: Font.Bold
                            }
                        }
                    }

                    Text {
                        text: "A new version of ACModOrganizer is ready to install"
                        color: Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                    }
                }

                // Close Button (disabled while downloading)
                Button {
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 32
                    enabled: !appController.updateManager || !appController.updateManager.isDownloading
                    background: Rectangle {
                        color: parent.hovered ? Theme.bgCardHover : "transparent"
                        radius: Theme.radiusSm
                    }
                    contentItem: Text {
                        text: "✕"
                        color: Theme.textSecondary
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: updateNotificationDialog.close()
                }
            }
        }

        // 2. Dialog Content: Release Title & Changelog
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: Theme.space20
            spacing: Theme.space12

            Text {
                text: appController.updateManager ? appController.updateManager.releaseTitle : ""
                color: Theme.textPrimary
                font.family: Theme.fontFamily
                font.pixelSize: 14
                font.weight: Font.DemiBold
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }

            Text {
                text: "WHAT'S NEW IN THIS VERSION"
                color: Theme.textSecondary
                font.family: Theme.fontFamily
                font.pixelSize: 10
                font.weight: Font.DemiBold
                font.letterSpacing: 0.8
            }

            // Scrollable Changelog box
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: Theme.radiusSm
                color: Theme.bgInput
                border.color: Theme.borderDefault
                border.width: 1
                clip: true

                ScrollView {
                    anchors.fill: parent
                    anchors.margins: Theme.space12
                    ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                    TextArea {
                        width: parent.width
                        readOnly: true
                        selectByMouse: true
                        text: appController.updateManager ? appController.updateManager.releaseNotes : ""
                        color: Theme.textPrimary
                        font.family: Theme.fontFamily
                        font.pixelSize: 12
                        wrapMode: Text.Wrap
                        background: null
                    }
                }
            }

            // Live Download Progress Bar
            ColumnLayout {
                Layout.fillWidth: true
                spacing: Theme.space4
                visible: appController.updateManager && appController.updateManager.isDownloading

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: appController.updateManager ? appController.updateManager.downloadStatusText : ""
                        color: Theme.textPrimary
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        font.weight: Font.Medium
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }
                    Text {
                        text: appController.updateManager ? (Math.round(appController.updateManager.downloadProgress * 100) + "%") : "0%"
                        color: Theme.textPrimary
                        font.family: Theme.fontFamilyMono
                        font.pixelSize: 11
                        font.weight: Font.Bold
                    }
                }

                ProgressBar {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 6
                    value: appController.updateManager ? appController.updateManager.downloadProgress : 0
                    background: Rectangle {
                        color: Theme.bgInput
                        radius: 3
                    }
                    contentItem: Item {
                        Rectangle {
                            width: parent.width * (appController.updateManager ? appController.updateManager.downloadProgress : 0)
                            height: parent.height
                            radius: 3
                            color: Theme.textPrimary
                        }
                    }
                }
            }
        }

        // 3. Dialog Footer Action Buttons (Monochrome & Clean)
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: Theme.bgCard
            border.color: Theme.borderDefault
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: Theme.space20
                anchors.rightMargin: Theme.space20
                spacing: Theme.space12

                Text {
                    text: "Current version: " + appController.appVersion
                    color: Theme.textMuted
                    font.family: Theme.fontFamilyMono
                    font.pixelSize: 11
                }

                Item { Layout.fillWidth: true }

                // Later / Cancel Button
                Button {
                    Layout.preferredWidth: 90
                    Layout.preferredHeight: 34
                    background: Rectangle {
                        color: parent.hovered ? Theme.bgCardHover : Theme.bgSurface
                        border.color: Theme.borderDefault
                        border.width: 1
                        radius: Theme.radiusSm
                    }
                    contentItem: Text {
                        text: (appController.updateManager && appController.updateManager.isDownloading) ? "Cancel" : "Later"
                        color: Theme.textPrimary
                        font.family: Theme.fontFamily
                        font.pixelSize: 12
                        font.weight: Font.Medium
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: {
                        if (appController.updateManager && appController.updateManager.isDownloading) {
                            appController.updateManager.cancelDownload()
                        } else {
                            updateNotificationDialog.close()
                        }
                    }
                }

                // Download & Install Button (Monochrome High Contrast)
                Button {
                    Layout.preferredWidth: 150
                    Layout.preferredHeight: 34
                    enabled: appController.updateManager && !appController.updateManager.isDownloading
                    background: Rectangle {
                        color: parent.enabled ? (parent.hovered ? Theme.btnPrimaryHover : Theme.btnPrimary) : Theme.bgInput
                        border.color: Theme.btnPrimaryBorder
                        border.width: 1
                        radius: Theme.radiusSm
                    }
                    contentItem: RowLayout {
                        anchors.centerIn: parent
                        spacing: Theme.space6

                        Image {
                            width: 14
                            height: 14
                            source: "qrc:/qt/qml/ACBO/ui/icons/zap.svg"
                            fillMode: Image.PreserveAspectFit
                            visible: parent.parent.enabled
                        }

                        Text {
                            text: (appController.updateManager && appController.updateManager.isDownloading) ? "Downloading..." : "Download & Restart"
                            color: parent.parent.enabled ? Theme.btnPrimaryText : Theme.textMuted
                            font.family: Theme.fontFamily
                            font.pixelSize: 12
                            font.weight: Font.DemiBold
                        }
                    }
                    onClicked: {
                        if (appController.updateManager) {
                            appController.updateManager.startDownloadAndInstall()
                        }
                    }
                }
            }
        }
    }
}
