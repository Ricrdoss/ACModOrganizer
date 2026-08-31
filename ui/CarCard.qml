import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: card
    radius: Theme.radiusMd
    color: Theme.bgCard
    border.color: {
        if (cardHover.hovered) return Theme.borderFocus;
        if (model.isPendingSave) return Theme.borderHover;
        return Theme.borderDefault;
    }
    border.width: 1

    Behavior on border.color { ColorAnimation { duration: 150 } }

    HoverHandler {
        id: cardHover
    }

    property var brandListModel: appController.knownBrands
    property var countryListModel: appController.knownCountries

    function filterList(query, list) {
        if (!query || query.trim() === "") return list;
        var q = query.toLowerCase().trim();
        var startsWith = [];
        var contains = [];
        for (var i = 0; i < list.length; ++i) {
            var item = list[i];
            var itemLower = item.toLowerCase();
            if (itemLower.indexOf(q) === 0) {
                startsWith.push(item);
            } else if (itemLower.indexOf(q) > 0) {
                contains.push(item);
            }
        }
        return startsWith.concat(contains);
    }

    function selectBrand(b) {
        brandField.text = b;
        carModel.setEditedBrand(index, b);
        brandPopup.close();

        var autoCountry = appController.getCountryForBrand(b);
        if (autoCountry && autoCountry !== "") {
            countryField.text = autoCountry;
            carModel.setEditedCountry(index, autoCountry);
        }
    }

    function selectCountry(c) {
        countryField.text = c;
        carModel.setEditedCountry(index, c);
        countryPopup.close();
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 1. Thumbnail Header (Aspect Ratio 16:9 approx)
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 160
            radius: Theme.radiusMd
            color: Theme.bgApp
            clip: true

            // Square off bottom corners so card flows continuously
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: Theme.radiusMd
                color: parent.color
            }

            // High-Resolution Car Preview Image from files
            Image {
                id: thumbnail
                anchors.fill: parent
                source: model.previewUrl ? (model.previewUrl + "?t=" + appController.previewReloadToken) : ""
                asynchronous: true
                cache: false
                fillMode: Image.PreserveAspectCrop
                sourceSize.width: 400
                sourceSize.height: 225
                visible: model.previewUrl != "" && status !== Image.Error && status !== Image.Null
            }

            // Fallback Vector Placeholder
            Rectangle {
                anchors.fill: parent
                visible: !thumbnail.visible
                color: Theme.bgApp

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: Theme.space8
                    Image {
                        Layout.alignment: Qt.AlignHCenter
                        width: 36
                        height: 36
                        source: Theme.isDarkMode ? "icons/car_white.svg" : "icons/car.svg"
                        sourceSize.width: 72
                        sourceSize.height: 72
                        opacity: 0.4
                    }
                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: thumbnail.status === Image.Loading ? "Loading Preview..." : "No Preview Image"
                        font.pixelSize: 11
                        font.weight: Font.Normal
                        color: Theme.textMuted
                    }
                }
            }

            // Top Status Badge Pill (Fluent Pill)
            Rectangle {
                id: statusBadge
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: Theme.space8
                implicitWidth: statusLayout.implicitWidth + Theme.space12
                implicitHeight: 22
                radius: Theme.radiusPill
                color: Theme.bgSurface
                border.color: Theme.borderDefault
                border.width: 1

                RowLayout {
                    id: statusLayout
                    anchors.centerIn: parent
                    spacing: Theme.space4

                    Image {
                        width: 11
                        height: 11
                        source: {
                            if (model.isPendingSave) return Theme.isDarkMode ? "icons/edit_white.svg" : "icons/edit.svg";
                            if (model.hasSuggestion) return Theme.isDarkMode ? "icons/zap_white.svg" : "icons/zap.svg";
                            if (model.isBrandMissing || model.editedBrand === "Brand Not Found") return Theme.isDarkMode ? "icons/alert_white.svg" : "icons/alert.svg";
                            return Theme.isDarkMode ? "icons/check_white.svg" : "icons/check.svg";
                        }
                        sourceSize.width: 22
                        sourceSize.height: 22
                        opacity: 0.8
                    }

                    Text {
                        text: model.statusString
                        color: Theme.textPrimary
                        font.family: Theme.fontFamily
                        font.pixelSize: 10
                        font.weight: Font.DemiBold
                    }
                }
            }

            // Top-Right Open Folder Button
            Rectangle {
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.margins: Theme.space12
                width: 28
                height: 28
                radius: Theme.radiusSm
                color: openHover.containsMouse ? Theme.borderHover : Theme.bgCard
                border.color: Theme.borderDefault
                border.width: 1

                Behavior on color { ColorAnimation { duration: 120 } }

                Image {
                    anchors.centerIn: parent
                    width: 14
                    height: 14
                    source: Theme.isDarkMode ? "icons/open_white.svg" : "icons/open.svg"
                    sourceSize.width: 28
                    sourceSize.height: 28
                }

                MouseArea {
                    id: openHover
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: carModel.openFolder(index)
                }

                ToolTip.visible: openHover.containsMouse
                ToolTip.text: "Open folder in Windows Explorer"
            }
        }

        // 2. Card Body Details
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: Theme.space16
            spacing: Theme.space12

            // Title & Subtitle Hierarchy
            ColumnLayout {
                Layout.fillWidth: true
                spacing: Theme.space2

                Text {
                    Layout.fillWidth: true
                    text: model.name || model.folderName
                    color: Theme.textPrimary
                    font.family: Theme.fontFamilyDisplay
                    font.pixelSize: 13
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Theme.space4

                    Text {
                        Layout.fillWidth: true
                        text: model.folderName
                        color: Theme.textMuted
                        font.pixelSize: 11
                        font.family: Theme.fontFamilyMono
                        elide: Text.ElideRight
                    }

                    Rectangle {
                        width: 18
                        height: 18
                        radius: Theme.radiusXs
                        color: copyHover.containsMouse ? Theme.bgHover : "transparent"

                        Image {
                            anchors.centerIn: parent
                            width: 11
                            height: 11
                            source: Theme.isDarkMode ? "icons/copy_white.svg" : "icons/copy.svg"
                            sourceSize.width: 22
                            sourceSize.height: 22
                            opacity: copyHover.containsMouse ? 1.0 : 0.6
                        }

                        MouseArea {
                            id: copyHover
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            ToolTip.visible: containsMouse
                            ToolTip.text: "Copy folder name"
                            onClicked: appController.copyToClipboard(model.folderName)
                        }
                    }
                }
            }

            // Auto-Detection Suggestion Banner
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 32
                radius: Theme.radiusSm
                visible: model.hasSuggestion
                color: Theme.bgSurface
                border.color: Theme.borderFocus
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: Theme.space8
                    anchors.rightMargin: Theme.space4
                    spacing: Theme.space6

                    Image {
                        width: 12
                        height: 12
                        source: Theme.isDarkMode ? "icons/zap_white.svg" : "icons/zap.svg"
                        sourceSize.width: 24
                        sourceSize.height: 24
                    }

                    Text {
                        Layout.fillWidth: true
                        text: "Suggested: " + model.suggestedBrand + (model.suggestedCountry ? " (" + model.suggestedCountry + ")" : "")
                        color: Theme.textPrimary
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                    }

                    Button {
                        implicitHeight: 24
                        implicitWidth: 64
                        padding: 0
                        background: Rectangle {
                            radius: Theme.radiusSm
                            color: parent.hovered ? Theme.primaryHover : Theme.primary
                        }
                        contentItem: Text {
                            text: "Apply"
                            color: Theme.primaryText
                            font.family: Theme.fontFamily
                            font.pixelSize: 11
                            font.weight: Font.DemiBold
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: carModel.applySuggestion(index)
                    }
                }
            }

            // Undetected Brand Warning Banner (Monochrome neutral alert)
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 30
                radius: Theme.radiusSm
                visible: !model.hasSuggestion && (model.editedBrand === "Brand Not Found" || model.isBrandMissing)
                color: Theme.bgSurface
                border.color: Theme.borderDefault
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: Theme.space8
                    anchors.rightMargin: Theme.space8
                    spacing: Theme.space8

                    Image {
                        width: 12
                        height: 12
                        source: Theme.isDarkMode ? "icons/alert_white.svg" : "icons/alert.svg"
                        sourceSize.width: 24
                        sourceSize.height: 24
                    }

                    Text {
                        Layout.fillWidth: true
                        text: "Brand Not Found — enter brand below"
                        color: Theme.dangerText
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                    }
                }
            }

            // Metadata Form: Brand & Country Inputs
            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: Theme.space8
                rowSpacing: Theme.space8

                // Brand Column
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: Theme.space4

                    Text {
                        text: "BRAND"
                        color: Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 10
                        font.weight: Font.DemiBold
                        font.letterSpacing: 0.8
                    }

                    TextField {
                        id: brandField
                        Layout.fillWidth: true
                        text: model.editedBrand !== "" ? model.editedBrand : (model.brand !== "" ? model.brand : "Brand Not Found")
                        color: (brandField.text === "Brand Not Found" || brandField.text === "Unknown") ? Theme.dangerText : Theme.textPrimary
                        font.family: Theme.fontFamily
                        font.pixelSize: 12
                        font.weight: (brandField.text === "Brand Not Found") ? Font.DemiBold : Font.Normal
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: (brandBadgeImg.visible) ? 28 : Theme.space8
                        rightPadding: 26
                        selectByMouse: true
                        placeholderText: "Brand..."
                        placeholderTextColor: Theme.textMuted

                        background: Rectangle {
                            implicitHeight: 34
                            radius: Theme.radiusSm
                            color: Theme.bgInput
                            border.color: brandField.activeFocus ? Theme.borderFocus : Theme.borderDefault
                            border.width: 1

                            // Small Brand Logo in input box
                            Image {
                                id: brandBadgeImg
                                anchors.left: parent.left
                                anchors.leftMargin: 6
                                anchors.verticalCenter: parent.verticalCenter
                                width: 16
                                height: 16
                                sourceSize.width: 32
                                sourceSize.height: 32
                                source: (model.badgeUrl && model.badgeUrl != "") ? model.badgeUrl : (appController.getBadgeForBrand(brandField.text) || "")
                                fillMode: Image.PreserveAspectFit
                                visible: source != "" && source != undefined
                                asynchronous: true
                            }
                        }

                        onTextEdited: {
                            card.brandListModel = card.filterList(text, appController.knownBrands)
                            if (!brandPopup.opened && card.brandListModel.length > 0) {
                                brandPopup.open()
                            }
                            carModel.setEditedBrand(index, text)
                            var autoCountry = appController.getCountryForBrand(text)
                            if (autoCountry && autoCountry !== "") {
                                countryField.text = autoCountry
                                carModel.setEditedCountry(index, autoCountry)
                            }
                        }

                        // Dropdown toggle arrow
                        Rectangle {
                            anchors.right: parent.right
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            anchors.margins: 4
                            width: 20
                            radius: Theme.radiusXs
                            color: brandArrowArea.containsMouse ? Theme.bgCardHover : "transparent"

                            Text {
                                anchors.centerIn: parent
                                text: "▾"
                                color: Theme.textSecondary
                                font.pixelSize: 12
                            }

                            MouseArea {
                                id: brandArrowArea
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    card.brandListModel = appController.knownBrands
                                    if (brandPopup.opened) brandPopup.close()
                                    else brandPopup.open()
                                }
                            }
                        }

                        Popup {
                            id: brandPopup
                            y: brandField.height + 4
                            width: Math.max(brandField.width, 200)
                            height: Math.min(240, brandListView.contentHeight + 8)
                            padding: 4
                            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
                            background: Rectangle {
                                color: Theme.bgSurface
                                border.color: Theme.borderDefault
                                border.width: 1
                                radius: Theme.radiusSm
                            }

                            ListView {
                                id: brandListView
                                anchors.fill: parent
                                clip: true
                                model: card.brandListModel
                                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                                delegate: ItemDelegate {
                                    width: brandListView.width
                                    height: 28
                                    padding: 0
                                    background: Rectangle {
                                        color: parent.hovered ? Theme.bgCardHover : "transparent"
                                        radius: Theme.radiusXs
                                    }
                                    contentItem: RowLayout {
                                        anchors.fill: parent
                                        anchors.leftMargin: Theme.space8
                                        anchors.rightMargin: Theme.space8
                                        spacing: Theme.space6

                                        Image {
                                            Layout.preferredWidth: 16
                                            Layout.preferredHeight: 16
                                            Layout.alignment: Qt.AlignVCenter
                                            source: appController.getBadgeForBrand(modelData) || ""
                                            sourceSize.width: 32
                                            sourceSize.height: 32
                                            fillMode: Image.PreserveAspectFit
                                            visible: source != "" && source != undefined
                                            asynchronous: true
                                        }

                                        Text {
                                            Layout.fillWidth: true
                                            text: modelData
                                            color: Theme.textPrimary
                                            font.family: Theme.fontFamily
                                            font.pixelSize: 12
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                    }
                                    onClicked: {
                                        card.selectBrand(modelData)
                                    }
                                }
                            }
                        }
                    }
                }

                // Country Column
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: Theme.space4

                    Text {
                        text: "COUNTRY"
                        color: Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 10
                        font.weight: Font.DemiBold
                        font.letterSpacing: 0.8
                    }

                    TextField {
                        id: countryField
                        Layout.fillWidth: true
                        text: model.editedCountry !== "" ? model.editedCountry : model.country
                        color: Theme.textPrimary
                        font.family: Theme.fontFamily
                        font.pixelSize: 12
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: Theme.space8
                        rightPadding: 26
                        selectByMouse: true
                        placeholderText: "Country..."
                        placeholderTextColor: Theme.textMuted

                        background: Rectangle {
                            implicitHeight: 34
                            radius: Theme.radiusSm
                            color: Theme.bgInput
                            border.color: countryField.activeFocus ? Theme.borderFocus : Theme.borderDefault
                            border.width: 1
                        }

                        onTextEdited: {
                            card.countryListModel = card.filterList(text, appController.knownCountries)
                            if (!countryPopup.opened && card.countryListModel.length > 0) {
                                countryPopup.open()
                            }
                            carModel.setEditedCountry(index, text)
                        }

                        // Dropdown toggle arrow
                        Rectangle {
                            anchors.right: parent.right
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            anchors.margins: 4
                            width: 20
                            radius: Theme.radiusXs
                            color: countryArrowArea.containsMouse ? Theme.bgCardHover : "transparent"

                            Text {
                                anchors.centerIn: parent
                                text: "▾"
                                color: Theme.textSecondary
                                font.pixelSize: 12
                            }

                            MouseArea {
                                id: countryArrowArea
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    card.countryListModel = appController.knownCountries
                                    if (countryPopup.opened) countryPopup.close()
                                    else countryPopup.open()
                                }
                            }
                        }

                        Popup {
                            id: countryPopup
                            y: countryField.height + 4
                            width: Math.max(countryField.width, 200)
                            height: Math.min(240, countryListView.contentHeight + 8)
                            padding: 4
                            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
                            background: Rectangle {
                                color: Theme.bgSurface
                                border.color: Theme.borderDefault
                                border.width: 1
                                radius: Theme.radiusSm
                            }

                            ListView {
                                id: countryListView
                                anchors.fill: parent
                                clip: true
                                model: card.countryListModel
                                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                                delegate: ItemDelegate {
                                    width: countryListView.width
                                    height: 28
                                    padding: 0
                                    background: Rectangle {
                                        color: parent.hovered ? Theme.bgCardHover : "transparent"
                                        radius: Theme.radiusXs
                                    }
                                    contentItem: Text {
                                        text: modelData
                                        color: Theme.textPrimary
                                        font.family: Theme.fontFamily
                                        font.pixelSize: 12
                                        verticalAlignment: Text.AlignVCenter
                                        leftPadding: Theme.space8
                                    }
                                    onClicked: {
                                        card.selectCountry(modelData)
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Item { Layout.fillHeight: true }

            // Card Footer Actions
            RowLayout {
                Layout.fillWidth: true
                spacing: Theme.space8

                Text {
                    text: model.year > 0 ? ("Year: " + model.year) : ""
                    color: Theme.textMuted
                    font.family: Theme.fontFamily
                    font.pixelSize: 11
                    font.weight: Font.Normal
                }

                Item { Layout.fillWidth: true }

                // Individual Save Button (Monochrome contrast button)
                Button {
                    id: saveBtn
                    enabled: model.isPendingSave
                    padding: 0
                    background: Rectangle {
                        implicitWidth: 80
                        implicitHeight: 28
                        radius: Theme.radiusSm
                        color: {
                            if (!saveBtn.enabled) return Theme.bgInput;
                            if (saveBtn.down) return Theme.primaryPressed;
                            return saveBtn.hovered ? Theme.primaryHover : Theme.primary;
                        }
                        border.color: saveBtn.enabled ? Theme.borderDefault : Theme.borderSubtle
                        border.width: 1
                        Behavior on color { ColorAnimation { duration: 120 } }
                    }
                    contentItem: RowLayout {
                        spacing: Theme.space4
                        Item { Layout.fillWidth: true }
                        Image {
                            Layout.alignment: Qt.AlignVCenter
                            width: 12
                            height: 12
                            source: {
                                if (model.isPendingSave) return Theme.isDarkMode ? "icons/save_dark.svg" : "icons/save_white.svg";
                                if (model.isSaved) return Theme.isDarkMode ? "icons/check.svg" : "icons/check_white.svg";
                                return "icons/save.svg";
                            }
                            sourceSize.width: 24
                            sourceSize.height: 24
                            visible: saveBtn.enabled || model.isSaved
                        }

                        Text {
                            Layout.alignment: Qt.AlignVCenter
                            verticalAlignment: Text.AlignVCenter
                            text: model.isPendingSave ? "Save" : (model.isSaved ? "Saved" : "Save")
                            color: saveBtn.enabled ? Theme.primaryText : Theme.textDisabled
                            font.family: Theme.fontFamily
                            font.pixelSize: 11
                            font.weight: Font.DemiBold
                        }
                        Item { Layout.fillWidth: true }
                    }
                    onClicked: carModel.saveCar(index)
                }
            }
        }
    }
}
