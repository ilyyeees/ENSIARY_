#include "digitalcontent.h"
#include <QJsonObject>
#include <QJsonArray>
#include <algorithm>

/**
 * @brief Constructor for DigitalContent class
 */
DigitalContent::DigitalContent(const QString& id, const QString& title, const QString& author, int publicationYear)
    : Resource(id, title, author, publicationYear, Resource::Category::DigitalContent), 
      m_contentType(ContentType::EBook), m_accessType(AccessType::Online), 
      m_requiresAuthentication(false), m_simultaneousUsers(1) {
    
    initializeDigitalContent();
    validateDigitalContentData();
}

/**
 * @brief Copy constructor for DigitalContent class
 */
DigitalContent::DigitalContent(const DigitalContent& other)
    : Resource(other), m_contentType(other.m_contentType), m_accessType(other.m_accessType),
      m_fileFormat(other.m_fileFormat), m_fileSize(other.m_fileSize), m_url(other.m_url),
      m_platform(other.m_platform), m_requiresAuthentication(other.m_requiresAuthentication),
      m_simultaneousUsers(other.m_simultaneousUsers), m_systemRequirements(other.m_systemRequirements) {
}

/**
 * @brief Assignment operator for DigitalContent class
 */
DigitalContent& DigitalContent::operator=(const DigitalContent& other) {
    if (this != &other) {
        Resource::operator=(other);
        m_contentType = other.m_contentType;
        m_accessType = other.m_accessType;
        m_fileFormat = other.m_fileFormat;
        m_fileSize = other.m_fileSize;
        m_url = other.m_url;
        m_platform = other.m_platform;
        m_requiresAuthentication = other.m_requiresAuthentication;
        m_simultaneousUsers = other.m_simultaneousUsers;
        m_systemRequirements = other.m_systemRequirements;
    }
    return *this;
}

/**
 * @brief Set content type
 */
void DigitalContent::setContentType(ContentType type) {
    m_contentType = type;
}

/**
 * @brief Set access type
 */
void DigitalContent::setAccessType(AccessType type) {
    m_accessType = type;
}

/**
 * @brief Set file format with validation
 */
void DigitalContent::setFileFormat(const QString& format) {
    if (format.trimmed().isEmpty()) {
        throw DigitalContentException("File format cannot be empty");
    }
    m_fileFormat = format.trimmed().toUpper();
}

/**
 * @brief Set file size
 */
void DigitalContent::setFileSize(const QString& size) {
    m_fileSize = size.trimmed();
}

/**
 * @brief Set URL with validation
 */
void DigitalContent::setUrl(const QUrl& url) {
    if (!url.isValid() && m_accessType == AccessType::Online) {
        throw DigitalContentException("Invalid URL for online content");
    }
    m_url = url;
}

/**
 * @brief Set platform
 */
void DigitalContent::setPlatform(const QString& platform) {
    m_platform = platform.trimmed();
}

/**
 * @brief Set authentication requirement
 */
void DigitalContent::setRequiresAuthentication(bool requiresAuth) {
    m_requiresAuthentication = requiresAuth;
}

/**
 * @brief Set simultaneous users with validation
 */
void DigitalContent::setSimultaneousUsers(int users) {
    if (users < 1) {
        throw DigitalContentException("Simultaneous users must be at least 1");
    }
    m_simultaneousUsers = users;
}

/**
 * @brief Set system requirements
 */
void DigitalContent::setSystemRequirements(const QString& requirements) {
    m_systemRequirements = requirements.trimmed();
}

/**
 * @brief Get detailed information about the digital content
 */
QString DigitalContent::getDetails() const {
    QString details;
    details += QString("Title: %1\n").arg(getTitle());
    details += QString("Author: %1\n").arg(getAuthor());
    details += QString("Type: %1 (%2)\n").arg(getResourceType(), contentTypeToString(m_contentType));
    details += QString("Access: %1\n").arg(accessTypeToString(m_accessType));
    details += QString("Publication Year: %1\n").arg(getPublicationYear());
    details += QString("Status: %1\n").arg(statusToString(getStatus()));
    
    if (!m_fileFormat.isEmpty()) {
        details += QString("Format: %1\n").arg(m_fileFormat);
    }
    
    if (!m_fileSize.isEmpty()) {
        details += QString("Size: %1\n").arg(m_fileSize);
    }
    
    if (!m_platform.isEmpty()) {
        details += QString("Platform: %1\n").arg(m_platform);
    }
    
    if (m_simultaneousUsers > 1) {
        details += QString("Simultaneous Users: %1\n").arg(m_simultaneousUsers);
    }
    
    if (m_requiresAuthentication) {
        details += "Requires Authentication: Yes\n";
    }
    
    if (!m_systemRequirements.isEmpty()) {
        details += QString("System Requirements: %1\n").arg(m_systemRequirements);
    }
    
    if (!getDescription().isEmpty()) {
        details += QString("Description: %1\n").arg(getDescription());
    }
    
    return details;
}

/**
 * @brief Check if digital content is valid for loan
 */
/**
 * @brief Check if can accommodate simultaneous loans
 */
bool DigitalContent::canAccommodateSimultaneousLoans() const {
    return m_simultaneousUsers > 1 || m_accessType == AccessType::Online;
}

/**
 * @brief Check if currently available (for digital content, usually always available)
 */
bool DigitalContent::isCurrentlyAvailable() const {
    return isAvailable() && (canAccommodateSimultaneousLoans() || !isBorrowed());
}

/**
 * @brief Convert digital content to JSON object
 */
QJsonObject DigitalContent::toJson() const {
    QJsonObject json;
    json["id"] = getId();
    json["type"] = getResourceType();
    json["title"] = getTitle();
    json["author"] = getAuthor();
    json["publicationYear"] = getPublicationYear();
    json["category"] = categoryToString(getCategory());
    json["status"] = statusToString(getStatus());
    json["dateAdded"] = getDateAdded().toString(Qt::ISODate);
    json["description"] = getDescription();
    
    // Digital content-specific fields
    json["contentType"] = contentTypeToString(m_contentType);
    json["accessType"] = accessTypeToString(m_accessType);
    json["fileFormat"] = m_fileFormat;
    json["fileSize"] = m_fileSize;
    json["url"] = m_url.toString();
    json["platform"] = m_platform;
    json["requiresAuthentication"] = m_requiresAuthentication;
    json["simultaneousUsers"] = m_simultaneousUsers;
    json["systemRequirements"] = m_systemRequirements;
    return json;
}

/**
 * @brief Load digital content from JSON object
 */
void DigitalContent::fromJson(const QJsonObject& json) {
    // Load base Resource fields
    setTitle(json["title"].toString());
    setAuthor(json["author"].toString());
    setPublicationYear(json["publicationYear"].toInt(2024));
    setCategory(stringToCategory(json["category"].toString()));
    setStatus(stringToStatus(json["status"].toString()));
    setDescription(json["description"].toString());
    
    // Load digital content-specific fields
    m_contentType = stringToContentType(json["contentType"].toString());
    m_accessType = stringToAccessType(json["accessType"].toString());
    m_fileFormat = json["fileFormat"].toString();
    m_fileSize = json["fileSize"].toString();
    m_url = QUrl(json["url"].toString());
    m_platform = json["platform"].toString();
    m_requiresAuthentication = json["requiresAuthentication"].toBool();
    m_simultaneousUsers = json["simultaneousUsers"].toInt(1);
    m_systemRequirements = json["systemRequirements"].toString();
}

/**
 * @brief Convert content type to string
 */
QString DigitalContent::contentTypeToString(ContentType type) {
    switch (type) {
        case ContentType::EBook: return "E-Book";
        case ContentType::AudioBook: return "Audio Book";
        case ContentType::Video: return "Video";
        case ContentType::Document: return "Document";
        case ContentType::Software: return "Software";
        case ContentType::Database: return "Database";
        case ContentType::WebResource: return "Web Resource";
        default: return "E-Book";
    }
}

/**
 * @brief Convert string to content type
 */
DigitalContent::ContentType DigitalContent::stringToContentType(const QString& typeStr) {
    if (typeStr == "Audio Book") return ContentType::AudioBook;
    if (typeStr == "Video") return ContentType::Video;
    if (typeStr == "Document") return ContentType::Document;
    if (typeStr == "Software") return ContentType::Software;
    if (typeStr == "Database") return ContentType::Database;
    if (typeStr == "Web Resource") return ContentType::WebResource;
    return ContentType::EBook; // Default
}

/**
 * @brief Convert access type to string
 */
QString DigitalContent::accessTypeToString(AccessType type) {
    switch (type) {
        case AccessType::Online: return "Online";
        case AccessType::Download: return "Download";
        case AccessType::Streaming: return "Streaming";
        default: return "Online";
    }
}

/**
 * @brief Convert string to access type
 */
DigitalContent::AccessType DigitalContent::stringToAccessType(const QString& typeStr) {
    if (typeStr == "Download") return AccessType::Download;
    if (typeStr == "Streaming") return AccessType::Streaming;
    return AccessType::Online; // Default
}

/**
 * @brief Validate digital content data
 */
bool DigitalContent::isValidDigitalContent() const {
    // Basic validation
    if (m_accessType == AccessType::Online && !m_url.isValid()) {
        return false;
    }
    return true;
}

/**
 * @brief Initialize digital content with default values
 */
void DigitalContent::initializeDigitalContent() {
    m_simultaneousUsers = 1;
    m_requiresAuthentication = false;
}

/**
 * @brief Validate digital content data and throw exception if invalid
 */
void DigitalContent::validateDigitalContentData() const {
    if (m_accessType == AccessType::Online && !m_url.isValid() && !m_url.isEmpty()) {
        throw DigitalContentException("Invalid URL for online content");
    }
}
