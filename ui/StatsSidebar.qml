import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: Theme.bgSurface
    implicitWidth: 272

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.space16
        spacing: Theme.space12

        Text {
            text: "COLLECTION STATS"
            font.pixelSize: 11
            font.weight: Font.DemiBold
            color: Theme.textMuted
            font.letterSpacing: 1.0
        }

        // Stat Card 1: Total Cars (Informational)
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 64
            radius: Theme.radiusMd
            color: Theme.bgCard
            border.color: Theme.borderDefault
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: Theme.space12
                spacing: Theme.space12

                Rectangle {
                    width: 36
                    height: 36
                    radius: Theme.radiusSm
                    color: Theme.bgSurface
                    border.color: Theme.borderSubtle
                    border.width: 1

                    Image {
                        anchors.centerIn: parent
                        width: 18
                        height: 18
                        source: Theme.isDarkMode ? "icons/car_white.svg" : "icons/car.svg"
                        sourceSize.width: 36
                        sourceSize.height: 36
                    }
                }

                ColumnLayout {
                    spacing: 0
                    Text {
                        text: "Total Cars"
                        font.pixelSize: 11
                        font.weight: Font.Medium
                        color: Theme.textSecondary
                    }
                    Text {
                        text: carModel.totalCount.toString()
                        font.pixelSize: 20
                        font.weight: Font.Bold
                        color: Theme.textPrimary
                    }
                }
            }
        }

        // Stat Card 2: Auto-Detected (Interactive Filter)
        Rectangle {
            id: detectedStatCard
            Layout.fillWidth: true
            Layout.preferredHeight: 64
            radius: Theme.radiusMd
            color: detArea.containsMouse ? Theme.bgCardHover : Theme.bgCard
            border.color: carModel.detectedCount > 0 ? Theme.successBorder : Theme.borderDefault
            border.width: 1

            Behavior on color { ColorAnimation { duration: 120 } }

            MouseArea {
                id: detArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: appController.filterMode = 2
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: Theme.space12
                spacing: Theme.space12

                Rectangle {
                    width: 36
                    height: 36
                    radius: Theme.radiusSm
                    color: Theme.successSurface
                    border.color: Theme.successBorder
                    border.width: 1

                    Image {
                        anchors.centerIn: parent
                        width: 16
                        height: 16
                        source: Theme.isDarkMode ? "icons/zap_white.svg" : "icons/zap.svg"
                        sourceSize.width: 32
                        sourceSize.height: 32
                    }
                }

                ColumnLayout {
                    spacing: 0
                    Text {
                        text: "Auto-Detected"
                        font.pixelSize: 11
                        font.weight: Font.Medium
                        color: Theme.successText
                    }
                    Text {
                        text: carModel.detectedCount.toString()
                        font.pixelSize: 20
                        font.weight: Font.Bold
                        color: Theme.successText
                    }
                }
            }
        }

        // Stat Card 3: Missing / Inconsistent (Interactive Filter)
        Rectangle {
            id: missingStatCard
            Layout.fillWidth: true
            Layout.preferredHeight: 64
            radius: Theme.radiusMd
            color: missArea.containsMouse ? Theme.bgCardHover : Theme.bgCard
            border.color: carModel.missingCount > 0 ? Theme.dangerBorder : Theme.borderDefault
            border.width: 1

            Behavior on color { ColorAnimation { duration: 120 } }

            MouseArea {
                id: missArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: appController.filterMode = 1
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: Theme.space12
                spacing: Theme.space12

                Rectangle {
                    width: 36
                    height: 36
                    radius: Theme.radiusSm
                    color: Theme.dangerSurface
                    border.color: Theme.dangerBorder
                    border.width: 1

                    Image {
                        anchors.centerIn: parent
                        width: 16
                        height: 16
                        source: Theme.isDarkMode ? "icons/alert_white.svg" : "icons/alert.svg"
                        sourceSize.width: 32
                        sourceSize.height: 32
                    }
                }

                ColumnLayout {
                    spacing: 0
                    Text {
                        text: "Missing Fields"
                        font.pixelSize: 11
                        font.weight: Font.Medium
                        color: Theme.dangerText
                    }
                    Text {
                        text: carModel.missingCount.toString()
                        font.pixelSize: 20
                        font.weight: Font.Bold
                        color: Theme.dangerText
                    }
                }
            }
        }

        // Stat Card 4: Pending Changes (Interactive Filter)
        Rectangle {
            id: pendingStatCard
            Layout.fillWidth: true
            Layout.preferredHeight: 64
            radius: Theme.radiusMd
            color: pendArea.containsMouse ? Theme.bgCardHover : Theme.bgCard
            border.color: carModel.pendingCount > 0 ? Theme.warningBorder : Theme.borderDefault
            border.width: 1

            Behavior on color { ColorAnimation { duration: 120 } }

            MouseArea {
                id: pendArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: appController.filterMode = 3
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: Theme.space12
                spacing: Theme.space12

                Rectangle {
                    width: 36
                    height: 36
                    radius: Theme.radiusSm
                    color: Theme.warningSurface
                    border.color: Theme.warningBorder
                    border.width: 1

                    Image {
                        anchors.centerIn: parent
                        width: 16
                        height: 16
                        source: Theme.isDarkMode ? "icons/edit_white.svg" : "icons/edit.svg"
                        sourceSize.width: 32
                        sourceSize.height: 32
                    }
                }

                ColumnLayout {
                    spacing: 0
                    Text {
                        text: "Pending Changes"
                        font.pixelSize: 11
                        font.weight: Font.Medium
                        color: Theme.warningText
                    }
                    Text {
                        text: carModel.pendingCount.toString()
                        font.pixelSize: 20
                        font.weight: Font.Bold
                        color: Theme.warningText
                    }
                }
            }
        }

        // Safety Info Box
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 120
            radius: Theme.radiusMd
            color: Theme.bgCard
            border.color: Theme.borderDefault
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: Theme.space12
                spacing: Theme.space8

                RowLayout {
                    spacing: Theme.space8
                    Image {
                        width: 14
                        height: 14
                        source: Theme.isDarkMode ? "icons/shield_white.svg" : "icons/shield.svg"
                        sourceSize.width: 28
                        sourceSize.height: 28
                    }
                    Text {
                        text: "Safety & Backups"
                        font.pixelSize: 12
                        font.weight: Font.DemiBold
                        color: Theme.textPrimary
                    }
                }

                Text {
                    Layout.fillWidth: true
                    text: "Original files are backed up as '.bak' before saving. All other JSON properties (specs, curves, author) are preserved."
                    font.pixelSize: 11
                    color: Theme.textMuted
                    lineHeight: 1.25
                    wrapMode: Text.WordWrap
                }
            }
        }

        Item { Layout.fillHeight: true }

        // Knowledge Base Metric Footer
        Text {
            Layout.alignment: Qt.AlignHCenter
            text: appController.knownBrands.length + " Brands • " + appController.knownCountries.length + " Countries"
            font.pixelSize: 11
            font.weight: Font.Normal
            color: Theme.textDisabled
        }
    }
}
