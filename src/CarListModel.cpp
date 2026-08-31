#include "CarListModel.hpp"
#include "JsonWriter.hpp"
#include "Logger.hpp"
#include <QDesktopServices>
#include <QUrl>
#include <algorithm>

namespace acbo {

CarListModel::CarListModel(QObject* parent)
    : QAbstractListModel(parent) {}

int CarListModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    if (m_filteredIndices.empty()) return 0;
    int start = (m_currentPage - 1) * m_pageSize;
    if (start < 0 || static_cast<size_t>(start) >= m_filteredIndices.size()) return 0;
    int remaining = static_cast<int>(m_filteredIndices.size()) - start;
    return std::min(m_pageSize, remaining);
}

int CarListModel::totalPages() const {
    if (m_filteredIndices.empty()) return 1;
    return std::max(1, static_cast<int>((m_filteredIndices.size() + m_pageSize - 1) / m_pageSize));
}

void CarListModel::setCurrentPage(int page) {
    int clamped = std::clamp(page, 1, totalPages());
    if (m_currentPage == clamped) return;
    beginResetModel();
    m_currentPage = clamped;
    endResetModel();
    emit pageChanged();
}

void CarListModel::setPageSize(int size) {
    if (size <= 0 || m_pageSize == size) return;
    beginResetModel();
    m_pageSize = size;
    m_currentPage = 1;
    endResetModel();
    emit pageSizeChanged();
    emit pageChanged();
}

void CarListModel::nextPage() {
    if (m_currentPage < totalPages()) {
        setCurrentPage(m_currentPage + 1);
    }
}

void CarListModel::prevPage() {
    if (m_currentPage > 1) {
        setCurrentPage(m_currentPage - 1);
    }
}

void CarListModel::goToPage(int page) {
    setCurrentPage(page);
}

QHash<int, QByteArray> CarListModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[FolderNameRole] = "folderName";
    roles[FolderPathRole] = "folderPath";
    roles[JsonPathRole] = "jsonPath";
    roles[PreviewUrlRole] = "previewUrl";
    roles[BrandRole] = "brand";
    roles[CountryRole] = "country";
    roles[AuthorRole] = "author";
    roles[YearRole] = "year";
    roles[TagsRole] = "tags";
    roles[SuggestedBrandRole] = "suggestedBrand";
    roles[SuggestedCountryRole] = "suggestedCountry";
    roles[DetectionConfidenceRole] = "detectionConfidence";
    roles[DetectionReasonRole] = "detectionReason";
    roles[HasSuggestionRole] = "hasSuggestion";
    roles[EditedBrandRole] = "editedBrand";
    roles[EditedCountryRole] = "editedCountry";
    roles[IsPendingSaveRole] = "isPendingSave";
    roles[IsSavedRole] = "isSaved";
    roles[StatusStringRole] = "statusString";
    roles[StatusTypeRole] = "statusType";
    roles[IsValidJsonRole] = "isValidJson";
    roles[ParseErrorRole] = "parseError";
    roles[OriginalIndexRole] = "originalIndex";
    roles[IsBrandMissingRole] = "isBrandMissing";
    roles[IsCountryMissingRole] = "isCountryMissing";
    roles[BadgeUrlRole] = "badgeUrl";
    roles[CountryFlagRole] = "countryFlag";
    return roles;
}

QVariant CarListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0) {
        return QVariant();
    }
    int filteredRow = (m_currentPage - 1) * m_pageSize + index.row();
    if (filteredRow < 0 || static_cast<size_t>(filteredRow) >= m_filteredIndices.size()) {
        return QVariant();
    }

    const size_t realIndex = m_filteredIndices[static_cast<size_t>(filteredRow)];
    const CarItem& car = m_allCars[realIndex];

    switch (role) {
        case NameRole:                return QString::fromStdString(car.name);
        case FolderNameRole:          return QString::fromStdString(car.folderName);
        case FolderPathRole:          return QString::fromStdString(car.folderPath);
        case JsonPathRole:            return QString::fromStdString(car.jsonPath);
        case PreviewUrlRole:          return car.previewUrl();
        case BrandRole:               return QString::fromStdString(car.brand);
        case CountryRole:             return QString::fromStdString(car.country);
        case AuthorRole:              return QString::fromStdString(car.author);
        case YearRole:                return car.year;
        case TagsRole:                return car.tagsString();
        case SuggestedBrandRole:      return QString::fromStdString(car.suggestedBrand);
        case SuggestedCountryRole:    return QString::fromStdString(car.suggestedCountry);
        case DetectionConfidenceRole: return QString::fromStdString(car.detectionConfidence);
        case DetectionReasonRole:     return QString::fromStdString(car.detectionReason);
        case HasSuggestionRole:       return car.hasSuggestion;
        case EditedBrandRole:         return QString::fromStdString(car.editedBrand);
        case EditedCountryRole:       return QString::fromStdString(car.editedCountry);
        case IsPendingSaveRole:       return car.isPendingSave;
        case IsSavedRole:             return car.isSaved;
        case StatusStringRole:        return car.statusString();
        case StatusTypeRole:          return static_cast<int>(car.status());
        case IsValidJsonRole:         return car.isValidJson;
        case ParseErrorRole:          return QString::fromStdString(car.parseError);
        case OriginalIndexRole:       return static_cast<int>(realIndex);
        case IsBrandMissingRole:      return car.isBrandMissing();
        case IsCountryMissingRole:    return car.isCountryMissing();
        case BadgeUrlRole:            return car.badgeUrl();
        case CountryFlagRole:         return car.countryFlag();
        default:                      return QVariant();
    }
}

void CarListModel::setCars(std::vector<CarItem> cars) {
    beginResetModel();
    m_allCars = std::move(cars);
    updateCounters();
    recomputeFilteredIndices();
    m_currentPage = 1;
    endResetModel();
    emit countsChanged();
    emit filterChanged();
    emit pageChanged();
}

void CarListModel::clear() {
    beginResetModel();
    m_allCars.clear();
    m_filteredIndices.clear();
    m_missingCount = 0;
    m_detectedCount = 0;
    m_pendingCount = 0;
    m_savedCount = 0;
    m_currentPage = 1;
    endResetModel();
    emit countsChanged();
    emit filterChanged();
    emit pageChanged();
}

void CarListModel::setSearchText(const QString& text) {
    if (m_searchText == text) return;
    beginResetModel();
    m_searchText = text.trimmed();
    recomputeFilteredIndices();
    m_currentPage = 1;
    endResetModel();
    emit filterChanged();
    emit pageChanged();
}

void CarListModel::setFilterMode(FilterMode mode) {
    if (m_filterMode == mode) return;
    beginResetModel();
    m_filterMode = mode;
    recomputeFilteredIndices();
    m_currentPage = 1;
    endResetModel();
    emit filterChanged();
    emit pageChanged();
}

int CarListModel::totalCount() const { return static_cast<int>(m_allCars.size()); }
int CarListModel::missingCount() const { return m_missingCount; }
int CarListModel::detectedCount() const { return m_detectedCount; }
int CarListModel::pendingCount() const { return m_pendingCount; }
int CarListModel::savedCount() const { return m_savedCount; }

bool CarListModel::passesFilter(const CarItem& car) const {
    // Mode filtering
    switch (m_filterMode) {
        case FilterMode::All:
            break;
        case FilterMode::Unassigned:
            // "Missing / Unknown" should ONLY show cars where brand is missing and NO suggestion could be detected!
            if (!car.isBrandMissing() || car.hasSuggestion) return false;
            break;
        case FilterMode::AutoDetected:
            if (!car.hasSuggestion) return false;
            break;
        case FilterMode::PendingChanges:
            if (!car.isPendingSave) return false;
            break;
        case FilterMode::Verified:
            if (car.status() != CarStatus::Verified) return false;
            break;
    }

    // Search query filtering
    if (!m_searchText.isEmpty()) {
        const QString q = m_searchText.toLower();
        const QString name = QString::fromStdString(car.name).toLower();
        const QString folder = QString::fromStdString(car.folderName).toLower();
        const QString brand = QString::fromStdString(car.brand).toLower();
        const QString country = QString::fromStdString(car.country).toLower();
        const QString author = QString::fromStdString(car.author).toLower();

        if (!name.contains(q) && !folder.contains(q) && !brand.contains(q) && !country.contains(q) && !author.contains(q)) {
            return false;
        }
    }

    return true;
}

void CarListModel::recomputeFilteredIndices() {
    m_filteredIndices.clear();
    for (size_t i = 0; i < m_allCars.size(); ++i) {
        if (passesFilter(m_allCars[i])) {
            m_filteredIndices.push_back(i);
        }
    }
}

void CarListModel::updateCounters() {
    m_missingCount = 0;
    m_detectedCount = 0;
    m_pendingCount = 0;
    m_savedCount = 0;

    for (const auto& car : m_allCars) {
        if (car.isBrandMissing() && !car.hasSuggestion) {
            m_missingCount++;
        }
        if (car.hasSuggestion) {
            m_detectedCount++;
        }
        if (car.isPendingSave) {
            m_pendingCount++;
        }
        if (car.isSaved) {
            m_savedCount++;
        }
    }
}

void CarListModel::applySuggestion(int filteredRow) {
    int actualIndex = (m_currentPage - 1) * m_pageSize + filteredRow;
    if (actualIndex < 0 || actualIndex >= static_cast<int>(m_filteredIndices.size())) return;
    const size_t realIndex = m_filteredIndices[static_cast<size_t>(actualIndex)];
    CarItem& car = m_allCars[realIndex];

    car.applySuggestion();
    updateCounters();

    QModelIndex idx = index(filteredRow);
    emit dataChanged(idx, idx);
    emit countsChanged();
}

void CarListModel::setEditedBrand(int filteredRow, const QString& newBrand) {
    int actualIndex = (m_currentPage - 1) * m_pageSize + filteredRow;
    if (actualIndex < 0 || actualIndex >= static_cast<int>(m_filteredIndices.size())) return;
    const size_t realIndex = m_filteredIndices[static_cast<size_t>(actualIndex)];
    CarItem& car = m_allCars[realIndex];

    car.setCustomBrand(newBrand.toStdString());
    updateCounters();

    QModelIndex idx = index(filteredRow);
    emit dataChanged(idx, idx);
    emit countsChanged();
}

void CarListModel::setEditedCountry(int filteredRow, const QString& newCountry) {
    int actualIndex = (m_currentPage - 1) * m_pageSize + filteredRow;
    if (actualIndex < 0 || actualIndex >= static_cast<int>(m_filteredIndices.size())) return;
    const size_t realIndex = m_filteredIndices[static_cast<size_t>(actualIndex)];
    CarItem& car = m_allCars[realIndex];

    car.setCustomCountry(newCountry.toStdString());
    updateCounters();

    QModelIndex idx = index(filteredRow);
    emit dataChanged(idx, idx);
    emit countsChanged();
}

bool CarListModel::saveCar(int filteredRow) {
    int actualIndex = (m_currentPage - 1) * m_pageSize + filteredRow;
    if (actualIndex < 0 || actualIndex >= static_cast<int>(m_filteredIndices.size())) return false;
    const size_t realIndex = m_filteredIndices[static_cast<size_t>(actualIndex)];
    CarItem& car = m_allCars[realIndex];

    auto res = JsonWriter::saveCar(car);
    if (res.success) {
        updateCounters();
        QModelIndex idx = index(filteredRow);
        emit dataChanged(idx, idx);
        emit countsChanged();
        emit saveSuccess(QString::fromStdString(car.folderName));
        return true;
    } else {
        emit saveError(QString::fromStdString(car.folderName), QString::fromStdString(res.errorMessage));
        return false;
    }
}

void CarListModel::applyAllSuggestions() {
    bool anyApplied = false;
    for (size_t i = 0; i < m_allCars.size(); ++i) {
        if (m_allCars[i].hasSuggestion) {
            m_allCars[i].applySuggestion();
            anyApplied = true;
        }
    }

    if (anyApplied) {
        updateCounters();
        beginResetModel();
        endResetModel();
        emit countsChanged();
    }
}

int CarListModel::saveAllPending() {
    int count = 0;
    for (size_t i = 0; i < m_allCars.size(); ++i) {
        if (m_allCars[i].isPendingSave) {
            auto res = JsonWriter::saveCar(m_allCars[i]);
            if (res.success) {
                count++;
            }
        }
    }

    if (count > 0) {
        updateCounters();
        beginResetModel();
        endResetModel();
        emit countsChanged();
    }
    return count;
}

void CarListModel::openFolder(int filteredRow) {
    int actualIndex = (m_currentPage - 1) * m_pageSize + filteredRow;
    if (actualIndex < 0 || actualIndex >= static_cast<int>(m_filteredIndices.size())) return;
    const size_t realIndex = m_filteredIndices[static_cast<size_t>(actualIndex)];
    const CarItem& car = m_allCars[realIndex];

    QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(car.folderPath)));
}

void CarListModel::updatePreviewForCar(const QString& folderName, const QString& newPreviewPath) {
    const std::string stdFolder = folderName.toStdString();
    const std::string stdPath = newPreviewPath.toStdString();

    for (size_t i = 0; i < m_allCars.size(); ++i) {
        if (m_allCars[i].folderName == stdFolder) {
            m_allCars[i].previewPath = stdPath;
            int start = (m_currentPage - 1) * m_pageSize;
            int count = rowCount();
            for (int r = 0; r < count; ++r) {
                int fIdx = start + r;
                if (fIdx < static_cast<int>(m_filteredIndices.size()) && m_filteredIndices[fIdx] == i) {
                    QModelIndex modelIdx = createIndex(r, 0);
                    emit dataChanged(modelIdx, modelIdx, {PreviewUrlRole});
                    break;
                }
            }
            break;
        }
    }
}

void CarListModel::refreshAllPreviews() {
    int count = rowCount();
    if (count > 0) {
        QModelIndex start = createIndex(0, 0);
        QModelIndex end = createIndex(count - 1, 0);
        emit dataChanged(start, end, {PreviewUrlRole});
    }
}

} // namespace acbo
