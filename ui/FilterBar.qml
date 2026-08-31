import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: Theme.bgSurface
    implicitHeight: 48

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Theme.space20
        anchors.rightMargin: Theme.space20
        spacing: Theme.space12

        // Filter Pills Row
        RowLayout {
            spacing: Theme.space8
            Layout.alignment: Qt.AlignVCenter

            // Tab 0: All Cars
            Rectangle {
                id: tabAll
                implicitWidth: contentAll.implicitWidth + Theme.space24
                implicitHeight: 32
                radius: Theme.radiusFull
                color: appController.filterMode === 0 ? Theme.primarySurface : (allArea.containsMouse ? Theme.bgHover : Theme.bgCard)
                border.color: appController.filterMode === 0 ? Theme.primary : Theme.borderDefault
                border.width: 1

                Behavior on color { ColorAnimation { duration: 120 } }
                Behavior on border.color { ColorAnimation { duration: 120 } }

                RowLayout {
                    id: contentAll
                    anchors.centerIn: parent
                    spacing: Theme.space8

                    Image {
                        width: 13
                        height: 13
                        source: Theme.isDarkMode ? "icons/car_white.svg" : "icons/car.svg"
                        sourceSize.width: 26
                        sourceSize.height: 26
                    }

                    Text {
                        text: "All Cars (" + carModel.totalCount + ")"
                        color: appController.filterMode === 0 ? Theme.textPrimary : Theme.textSecondary
                        font.pixelSize: 12
                        font.weight: appController.filterMode === 0 ? Font.DemiBold : Font.Normal
                    }
                }

                MouseArea {
                    id: allArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: appController.filterMode = 0
                }
            }

            // Tab 1: Unassigned / Unknown
            Rectangle {
                id: tabUnassigned
                implicitWidth: contentUnassigned.implicitWidth + Theme.space24
                implicitHeight: 32
                radius: Theme.radiusFull
                color: appController.filterMode === 1 ? Theme.dangerSurface : (unassignedArea.containsMouse ? Theme.bgHover : Theme.bgCard)
                border.color: appController.filterMode === 1 ? Theme.danger : Theme.borderDefault
                border.width: 1

                Behavior on color { ColorAnimation { duration: 120 } }
                Behavior on border.color { ColorAnimation { duration: 120 } }

                RowLayout {
                    id: contentUnassigned
                    anchors.centerIn: parent
                    spacing: Theme.space8

                    Image {
                        width: 13
                        height: 13
                        source: Theme.isDarkMode ? "icons/alert_white.svg" : "icons/alert.svg"
                        sourceSize.width: 26
                        sourceSize.height: 26
                    }

                    Text {
                        text: "Missing / Unknown (" + carModel.missingCount + ")"
                        color: appController.filterMode === 1 ? Theme.textPrimary : (carModel.missingCount > 0 ? Theme.dangerText : Theme.textSecondary)
                        font.pixelSize: 12
                        font.weight: appController.filterMode === 1 ? Font.DemiBold : Font.Normal
                    }
                }

                MouseArea {
                    id: unassignedArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: appController.filterMode = 1
                }
            }

            // Tab 2: Auto-Detected Matches
            Rectangle {
                id: tabDetected
                implicitWidth: contentDetected.implicitWidth + Theme.space24
                implicitHeight: 32
                radius: Theme.radiusFull
                color: appController.filterMode === 2 ? Theme.successSurface : (detectedArea.containsMouse ? Theme.bgHover : Theme.bgCard)
                border.color: appController.filterMode === 2 ? Theme.success : Theme.borderDefault
                border.width: 1

                Behavior on color { ColorAnimation { duration: 120 } }
                Behavior on border.color { ColorAnimation { duration: 120 } }

                RowLayout {
                    id: contentDetected
                    anchors.centerIn: parent
                    spacing: Theme.space8

                    Image {
                        width: 13
                        height: 13
                        source: Theme.isDarkMode ? "icons/zap_white.svg" : "icons/zap.svg"
                        sourceSize.width: 26
                        sourceSize.height: 26
                    }

                    Text {
                        text: "Auto-Detected (" + carModel.detectedCount + ")"
                        color: appController.filterMode === 2 ? Theme.textPrimary : (carModel.detectedCount > 0 ? Theme.successText : Theme.textSecondary)
                        font.pixelSize: 12
                        font.weight: appController.filterMode === 2 ? Font.DemiBold : Font.Normal
                    }
                }

                MouseArea {
                    id: detectedArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: appController.filterMode = 2
                }
            }

            // Tab 3: Pending Changes
            Rectangle {
                id: tabPending
                implicitWidth: contentPending.implicitWidth + Theme.space24
                implicitHeight: 32
                radius: Theme.radiusFull
                color: appController.filterMode === 3 ? Theme.warningSurface : (pendingArea.containsMouse ? Theme.bgHover : Theme.bgCard)
                border.color: appController.filterMode === 3 ? Theme.warning : Theme.borderDefault
                border.width: 1

                Behavior on color { ColorAnimation { duration: 120 } }
                Behavior on border.color { ColorAnimation { duration: 120 } }

                RowLayout {
                    id: contentPending
                    anchors.centerIn: parent
                    spacing: Theme.space8

                    Image {
                        width: 13
                        height: 13
                        source: Theme.isDarkMode ? "icons/edit_white.svg" : "icons/edit.svg"
                        sourceSize.width: 26
                        sourceSize.height: 26
                    }

                    Text {
                        text: "Pending Changes (" + carModel.pendingCount + ")"
                        color: appController.filterMode === 3 ? Theme.textPrimary : (carModel.pendingCount > 0 ? Theme.warningText : Theme.textSecondary)
                        font.pixelSize: 12
                        font.weight: appController.filterMode === 3 ? Font.DemiBold : Font.Normal
                    }
                }

                MouseArea {
                    id: pendingArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: appController.filterMode = 3
                }
            }

            // Tab 4: Verified Cars
            Rectangle {
                id: tabVerified
                implicitWidth: contentVerified.implicitWidth + Theme.space24
                implicitHeight: 32
                radius: Theme.radiusFull
                color: appController.filterMode === 4 ? Theme.bgHover : (verifiedArea.containsMouse ? Theme.bgHover : Theme.bgCard)
                border.color: appController.filterMode === 4 ? Theme.borderHover : Theme.borderDefault
                border.width: 1

                Behavior on color { ColorAnimation { duration: 120 } }
                Behavior on border.color { ColorAnimation { duration: 120 } }

                RowLayout {
                    id: contentVerified
                    anchors.centerIn: parent
                    spacing: Theme.space8

                    Image {
                        width: 13
                        height: 13
                        source: Theme.isDarkMode ? "icons/check_white.svg" : "icons/check.svg"
                        sourceSize.width: 26
                        sourceSize.height: 26
                    }

                    Text {
                        text: "Verified"
                        color: appController.filterMode === 4 ? Theme.textPrimary : Theme.textSecondary
                        font.pixelSize: 12
                        font.weight: appController.filterMode === 4 ? Font.DemiBold : Font.Normal
                    }
                }

                MouseArea {
                    id: verifiedArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: appController.filterMode = 4
                }
            }
        }

        Item { Layout.fillWidth: true }

        // Filtered Count Summary
        Text {
            text: "Showing " + carModel.filteredCount + " of " + carModel.totalCount + " cars"
            color: Theme.textMuted
            font.pixelSize: 12
            font.weight: Font.Normal
        }
    }
}
