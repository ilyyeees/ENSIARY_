#include "thesis.h"
#include <QJsonObject>
#include <QJsonArray>
#include <algorithm>

/**
 * @brief Constructor for Thesis class
 */
Thesis::Thesis(const QString& id, const QString& title, const QString& author, int publicationYear)
    : Resource(id, title, author, publicationYear, Resource::Category::Thesis), 
      m_degreeLevel(DegreeLevel::Bachelors) {
    
    initializeThesis();
    validateThesisData();
}

/**
 * @brief Copy constructor for Thesis class
 */
Thesis::Thesis(const Thesis& other)
    : Resource(other), m_supervisor(other.m_supervisor), m_university(other.m_university),
      m_department(other.m_department), m_degreeLevel(other.m_degreeLevel),
      m_keywords(other.m_keywords) {
}

/**
 * @brief Assignment operator for Thesis class
 */
Thesis& Thesis::operator=(const Thesis& other) {
    if (this != &other) {
        Resource::operator=(other);
        m_supervisor = other.m_supervisor;
        m_university = other.m_university;
        m_department = other.m_department;
        m_degreeLevel = other.m_degreeLevel;
        m_keywords = other.m_keywords;
    }
    return *this;
}

/**
 * @brief Set supervisor with validation
 */
void Thesis::setSupervisor(const QString& supervisor) {
    if (supervisor.trimmed().isEmpty()) {
        throw ThesisException("Supervisor name cannot be empty");
    }
    m_supervisor = supervisor.trimmed();
}

/**
 * @brief Set university with validation
 */
void Thesis::setUniversity(const QString& university) {
    if (university.trimmed().isEmpty()) {
        throw ThesisException("University name cannot be empty");
    }
    m_university = university.trimmed();
}

/**
 * @brief Set department with validation
 */
void Thesis::setDepartment(const QString& department) {
    if (department.trimmed().isEmpty()) {
        throw ThesisException("Department name cannot be empty");
    }
    m_department = department.trimmed();
}

/**
 * @brief Set degree level
 */
void Thesis::setDegreeLevel(DegreeLevel level) {
    m_degreeLevel = level;
}

/**
 * @brief Set keywords
 */
void Thesis::setKeywords(const QString& keywords) {
    m_keywords = keywords.trimmed();
}

/**
 * @brief Get detailed information about the thesis
 */
QString Thesis::getDetails() const {
    QString details;
    details += QString("Title: %1\n").arg(getTitle());
    details += QString("Author: %1\n").arg(getAuthor());
    details += QString("Type: Thesis (%1)\n").arg(degreeLevelToString(m_degreeLevel));
    details += QString("Supervisor: %1\n").arg(m_supervisor);
    details += QString("University: %1\n").arg(m_university);
    details += QString("Department: %1\n").arg(m_department);
    details += QString("Publication Year: %1\n").arg(getPublicationYear());
    details += QString("Status: %1\n").arg(statusToString(getStatus()));
    
    if (!m_keywords.isEmpty()) {
        details += QString("Keywords: %1\n").arg(m_keywords);
    }
    
    if (!getDescription().isEmpty()) {
        details += QString("Description: %1\n").arg(getDescription());
    }
    
    return details;
}

/**
 * @brief Convert thesis to JSON object
 */
QJsonObject Thesis::toJson() const {
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
    
    // Thesis-specific fields
    json["supervisor"] = m_supervisor;
    json["university"] = m_university;
    json["department"] = m_department;
    json["degreeLevel"] = degreeLevelToString(m_degreeLevel);
    json["keywords"] = m_keywords;
    return json;
}

/**
 * @brief Load thesis from JSON object
 */
void Thesis::fromJson(const QJsonObject& json) {
    // Load base Resource fields
    setTitle(json["title"].toString());
    setAuthor(json["author"].toString());
    setPublicationYear(json["publicationYear"].toInt(2024));
    setCategory(stringToCategory(json["category"].toString()));
    setStatus(stringToStatus(json["status"].toString()));
    setDescription(json["description"].toString());
    
    // Load thesis-specific fields
    m_supervisor = json["supervisor"].toString();
    m_university = json["university"].toString();
    m_department = json["department"].toString();
    m_degreeLevel = stringToDegreeLevel(json["degreeLevel"].toString());
    m_keywords = json["keywords"].toString();
}

/**
 * @brief Convert degree level to string
 */
QString Thesis::degreeLevelToString(DegreeLevel level) {
    switch (level) {
        case DegreeLevel::Bachelors: return "Bachelor's";
        case DegreeLevel::Masters: return "Master's";
        case DegreeLevel::PhD: return "PhD";
        case DegreeLevel::Postdoc: return "Postdoc";
        default: return "Bachelor's";
    }
}

/**
 * @brief Convert string to degree level
 */
Thesis::DegreeLevel Thesis::stringToDegreeLevel(const QString& levelStr) {
    if (levelStr == "Master's") return DegreeLevel::Masters;
    if (levelStr == "PhD") return DegreeLevel::PhD;
    if (levelStr == "Postdoc") return DegreeLevel::Postdoc;
    return DegreeLevel::Bachelors; // Default
}

/**
 * @brief Validate thesis data
 */
bool Thesis::isValidThesis() const {
    return !m_supervisor.isEmpty() && !m_university.isEmpty() && !m_department.isEmpty();
}

/**
 * @brief Initialize thesis with default values
 */
void Thesis::initializeThesis() {
    // Set default values
    m_degreeLevel = DegreeLevel::Bachelors;
}

/**
 * @brief Validate thesis data and throw exception if invalid
 */
void Thesis::validateThesisData() const {
    // Basic validation is done in the setters
    // Additional validation can be added here if needed
}
