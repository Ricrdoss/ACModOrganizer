#include "CarItem.hpp"
#include <cctype>
#include <algorithm>
#include <filesystem>

namespace acbo {

namespace {

bool isPlaceholderString(std::string_view s) {
    if (s.empty()) return true;
    std::string lower;
    lower.reserve(s.size());
    for (char c : s) {
        lower.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }

    static const std::vector<std::string> placeholders = {
        "unknown", "none", "null", "undefined", "unspecified", "tbd", "n/a", "na", "?", "-", "kunos",
        "brand not found", "not found", "unknown brand"
    };

    for (const auto& ph : placeholders) {
        if (lower == ph) return true;
    }
    return false;
}

} // namespace

bool CarItem::isBrandMissing() const {
    return brand.empty() || isPlaceholderString(brand);
}

bool CarItem::isCountryMissing() const {
    return country.empty() || isPlaceholderString(country);
}

bool CarItem::isCountryMismatched() const {
    return false;
}

CarStatus CarItem::status() const {
    if (isPendingSave) return CarStatus::PendingSave;
    if (hasSuggestion) return CarStatus::AutoDetected;
    if (isBrandMissing()) return CarStatus::MissingBrand;
    return CarStatus::Verified;
}

QString CarItem::statusString() const {
    switch (status()) {
        case CarStatus::PendingSave:    return QStringLiteral("Pending");
        case CarStatus::AutoDetected:   return QStringLiteral("Auto-Detected");
        case CarStatus::MissingBrand:   return QStringLiteral("Brand Not Found");
        case CarStatus::MissingCountry: return QStringLiteral("Missing Country");
        case CarStatus::Mismatched:     return QStringLiteral("Mismatch");
        case CarStatus::Verified:       return QStringLiteral("Verified");
    }
    return QStringLiteral("Unknown");
}

void CarItem::applySuggestion() {
    if (hasSuggestion) {
        editedBrand = suggestedBrand;
        editedCountry = suggestedCountry;
        isPendingSave = true;
    }
}

void CarItem::setCustomBrand(const std::string& newBrand) {
    editedBrand = newBrand;
    isPendingSave = (editedBrand != brand || editedCountry != country);
}

void CarItem::setCustomCountry(const std::string& newCountry) {
    editedCountry = newCountry;
    isPendingSave = (editedBrand != brand || editedCountry != country);
}

void CarItem::markSaved() {
    brand = editedBrand;
    country = editedCountry;
    isPendingSave = false;
    isSaved = true;
    hasSuggestion = false;
}

QUrl CarItem::previewUrl() const {
    std::error_code ec;
    if (!previewPath.empty()) {
        std::filesystem::path p(QString::fromStdString(previewPath).toStdWString());
        if (std::filesystem::exists(p, ec) && !ec) {
            return QUrl::fromLocalFile(QString::fromStdString(previewPath));
        }
    }

    if (!folderPath.empty()) {
        std::filesystem::path root(QString::fromStdString(folderPath).toStdWString());
        
        // 1. Direct ui preview (e.g. ui/preview.png / preview.jpg)
        for (const auto& ext : {L".png", L".jpg", L".jpeg", L".PNG", L".JPG"}) {
            std::filesystem::path uiPrev = root / L"ui" / (std::wstring(L"preview") + ext);
            if (std::filesystem::exists(uiPrev, ec) && !ec) {
                return QUrl::fromLocalFile(QString::fromStdWString(uiPrev.wstring()));
            }
        }

        // 2. Look in skins subdirectories (e.g. skins/<skin>/preview.jpg)
        std::filesystem::path skinsDir = root / L"skins";
        if (std::filesystem::exists(skinsDir, ec) && std::filesystem::is_directory(skinsDir, ec)) {
            for (const auto& skinEntry : std::filesystem::directory_iterator(skinsDir, ec)) {
                if (ec) break;
                if (skinEntry.is_directory(ec)) {
                    for (const auto& ext : {L".jpg", L".png", L".jpeg", L".JPG", L".PNG"}) {
                        std::filesystem::path sp = skinEntry.path() / (std::wstring(L"preview") + ext);
                        if (std::filesystem::exists(sp, ec) && !ec) {
                            return QUrl::fromLocalFile(QString::fromStdWString(sp.wstring()));
                        }
                    }
                }
            }
        }
    }

    return QUrl();
}

QUrl CarItem::badgeUrl() const {
    std::error_code ec;
    if (!badgePath.empty()) {
        std::filesystem::path p(QString::fromStdString(badgePath).toStdWString());
        if (std::filesystem::exists(p, ec) && !ec) {
            return QUrl::fromLocalFile(QString::fromStdString(badgePath));
        }
    }

    if (!folderPath.empty()) {
        std::filesystem::path root(QString::fromStdString(folderPath).toStdWString());
        for (const auto& ext : {L".png", L".jpg", L".svg", L".PNG"}) {
            std::filesystem::path b = root / L"ui" / (std::wstring(L"badge") + ext);
            if (std::filesystem::exists(b, ec) && !ec) {
                return QUrl::fromLocalFile(QString::fromStdWString(b.wstring()));
            }
        }
    }

    return QUrl();
}

QString CarItem::getCountryFlag(std::string_view countryName) {
    if (countryName.empty()) return QString();

    std::string clean;
    for (char ch : countryName) {
        if (std::isalnum(static_cast<unsigned char>(ch)) || ch == ' ') {
            clean.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
        }
    }
    while (!clean.empty() && clean.front() == ' ') clean.erase(clean.begin());
    while (!clean.empty() && clean.back() == ' ') clean.pop_back();

    if (clean == "germany" || clean == "deutschland" || clean == "de" || clean == "ger" || clean == "deu") {
        return QString::fromUtf8("🇩🇪");
    }
    if (clean == "italy" || clean == "italia" || clean == "it" || clean == "ita") {
        return QString::fromUtf8("🇮🇹");
    }
    if (clean == "japan" || clean == "nippon" || clean == "jp" || clean == "jpn") {
        return QString::fromUtf8("🇯🇵");
    }
    if (clean == "united states" || clean == "usa" || clean == "us" || clean == "america" || clean == "united states of america") {
        return QString::fromUtf8("🇺🇸");
    }
    if (clean == "united kingdom" || clean == "uk" || clean == "great britain" || clean == "britain" || clean == "england" || clean == "gb" || clean == "gbr" || clean == "scotland" || clean == "wales") {
        return QString::fromUtf8("🇬🇧");
    }
    if (clean == "france" || clean == "fr" || clean == "fra") {
        return QString::fromUtf8("🇫🇷");
    }
    if (clean == "sweden" || clean == "sverige" || clean == "se" || clean == "swe") {
        return QString::fromUtf8("🇸🇪");
    }
    if (clean == "spain" || clean == "espana" || clean == "es" || clean == "esp") {
        return QString::fromUtf8("🇪🇸");
    }
    if (clean == "czech republic" || clean == "czechia" || clean == "czech" || clean == "cz" || clean == "cze") {
        return QString::fromUtf8("🇨🇿");
    }
    if (clean == "south korea" || clean == "korea" || clean == "kr" || clean == "kor") {
        return QString::fromUtf8("🇰🇷");
    }
    if (clean == "austria" || clean == "osterreich" || clean == "at" || clean == "aut") {
        return QString::fromUtf8("🇦🇹");
    }
    if (clean == "australia" || clean == "au" || clean == "aus") {
        return QString::fromUtf8("🇦🇺");
    }
    if (clean == "netherlands" || clean == "holland" || clean == "nl" || clean == "nld") {
        return QString::fromUtf8("🇳🇱");
    }
    if (clean == "denmark" || clean == "danmark" || clean == "dk" || clean == "dnk") {
        return QString::fromUtf8("🇩🇰");
    }
    if (clean == "romania" || clean == "ro" || clean == "rou") {
        return QString::fromUtf8("🇷🇴");
    }
    if (clean == "russia" || clean == "ru" || clean == "rus") {
        return QString::fromUtf8("🇷🇺");
    }
    if (clean == "turkey" || clean == "turkiye" || clean == "tr" || clean == "tur") {
        return QString::fromUtf8("🇹🇷");
    }
    if (clean == "united arab emirates" || clean == "uae" || clean == "dubai" || clean == "ae" || clean == "are") {
        return QString::fromUtf8("🇦🇪");
    }
    if (clean == "croatia" || clean == "hrvatska" || clean == "hr" || clean == "hrv") {
        return QString::fromUtf8("🇭🇷");
    }
    if (clean == "switzerland" || clean == "schweiz" || clean == "suisse" || clean == "ch" || clean == "che") {
        return QString::fromUtf8("🇨🇭");
    }
    if (clean == "new zealand" || clean == "nz" || clean == "nzl") {
        return QString::fromUtf8("🇳🇿");
    }
    if (clean == "canada" || clean == "ca" || clean == "can") {
        return QString::fromUtf8("🇨🇦");
    }
    if (clean == "brazil" || clean == "brasil" || clean == "br" || clean == "bra") {
        return QString::fromUtf8("🇧🇷");
    }
    if (clean == "mexico" || clean == "mx" || clean == "mex") {
        return QString::fromUtf8("🇲🇽");
    }
    if (clean == "china" || clean == "cn" || clean == "chn") {
        return QString::fromUtf8("🇨🇳");
    }
    if (clean == "poland" || clean == "polska" || clean == "pl" || clean == "pol") {
        return QString::fromUtf8("🇵🇱");
    }
    if (clean == "belgium" || clean == "belgique" || clean == "be" || clean == "bel") {
        return QString::fromUtf8("🇧🇪");
    }
    if (clean == "portugal" || clean == "pt" || clean == "prt") {
        return QString::fromUtf8("🇵🇹");
    }
    if (clean == "saudi" || clean == "saudi arabia" || clean == "sa" || clean == "sau") {
        return QString::fromUtf8("🇸🇦");
    }
    if (clean == "finland" || clean == "suomi" || clean == "fi" || clean == "fin") {
        return QString::fromUtf8("🇫🇮");
    }
    if (clean == "norway" || clean == "norge" || clean == "no" || clean == "nor") {
        return QString::fromUtf8("🇳🇴");
    }
    if (clean == "ireland" || clean == "ie" || clean == "irl") {
        return QString::fromUtf8("🇮🇪");
    }
    if (clean == "argentina" || clean == "ar" || clean == "arg") {
        return QString::fromUtf8("🇦🇷");
    }
    if (clean == "monaco" || clean == "mc" || clean == "mco") {
        return QString::fromUtf8("🇲🇨");
    }
    if (clean == "south africa" || clean == "za" || clean == "zaf") {
        return QString::fromUtf8("🇿🇦");
    }
    if (clean == "hungary" || clean == "magyarorszag" || clean == "hu" || clean == "hun") {
        return QString::fromUtf8("🇭🇺");
    }
    if (clean == "slovakia" || clean == "slovensko" || clean == "sk" || clean == "svk") {
        return QString::fromUtf8("🇸🇰");
    }
    if (clean == "slovenia" || clean == "slovenija" || clean == "si" || clean == "svn") {
        return QString::fromUtf8("🇸🇮");
    }
    if (clean == "greece" || clean == "hellas" || clean == "gr" || clean == "grc") {
        return QString::fromUtf8("🇬🇷");
    }
    if (clean == "ukraine" || clean == "ua" || clean == "ukr") {
        return QString::fromUtf8("🇺🇦");
    }

    return QString();
}

QString CarItem::countryFlag() const {
    const std::string& c = !editedCountry.empty() ? editedCountry : country;
    return getCountryFlag(c);
}

QString CarItem::tagsString() const {
    QStringList list;
    for (const auto& tag : tags) {
        list.append(QString::fromStdString(tag));
    }
    return list.join(QStringLiteral(", "));
}

} // namespace acbo
