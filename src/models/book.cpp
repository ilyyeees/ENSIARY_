#include "book.h"
#include <QJsonObject>
#include <QRegularExpression>

/**
 * @brief Constructor for Book class
 */
Book::Book(const QString& id, const QString& title, const QString& author,
           int publicationYear, const QString& isbn, const QString& publisher,
           int pageCount, const QString& language, const QString& genre, bool isHardcover)
    : Resource(id, title, author, publicationYear, Category::Book),
      m_isbn(isbn), m_publisher(publisher), m_pageCount(pageCount),
      m_language(language), m_genre(genre), m_isHardcover(isHardcover) {
    
    validateBookData();
}

/**
 * @brief Get detailed information about the book
 */
QString Book::getDetails() const {
    QString details;
    details += QString("Title: %1\n").arg(getTitle());
    details += QString("Author: %1\n").arg(getAuthor());
    details += QString("ISBN: %1\n").arg(m_isbn);
    details += QString("Publisher: %1\n").arg(m_publisher);
    details += QString("Publication Year: %1\n").arg(getPublicationYear());
    details += QString("Pages: %1\n").arg(m_pageCount);
    details += QString("Language: %1\n").arg(m_language);
    details += QString("Genre: %1\n").arg(m_genre);
    details += QString("Format: %1\n").arg(m_isHardcover ? "Hardcover" : "Paperback");
    details += QString("Status: %1\n").arg(statusToString(getStatus()));
    
    if (!getDescription().isEmpty()) {
        details += QString("Description: %1\n").arg(getDescription());
    }
    
    return details;
}

/**
 * @brief Get the resource type
 */
QString Book::getResourceType() const {
    return "Book";
}

/**
 * @brief Convert book to JSON object
 */
QJsonObject Book::toJson() const {
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
    
    // Book-specific fields
    json["isbn"] = m_isbn;
    json["publisher"] = m_publisher;
    json["pageCount"] = m_pageCount;
    json["language"] = m_language;
    json["genre"] = m_genre;
    json["isHardcover"] = m_isHardcover;
    
    return json;
}

/**
 * @brief Load book from JSON object
 */
void Book::fromJson(const QJsonObject& json) {
    // Load base Resource fields
    setTitle(json["title"].toString());
    setAuthor(json["author"].toString());
    setPublicationYear(json["publicationYear"].toInt());
    setCategory(stringToCategory(json["category"].toString()));
    setStatus(stringToStatus(json["status"].toString()));
    setDescription(json["description"].toString());
    
    // Load Book-specific fields
    m_isbn = json["isbn"].toString();
    m_publisher = json["publisher"].toString();
    m_pageCount = json["pageCount"].toInt();
    m_language = json["language"].toString();
    m_genre = json["genre"].toString();
    m_isHardcover = json["isHardcover"].toBool();
}

/**
 * @brief Set ISBN with validation
 */
void Book::setIsbn(const QString& isbn) {
    if (!isValidIsbn(isbn)) {
        throw ResourceException("Invalid ISBN format");
    }
    m_isbn = isbn;
}

/**
 * @brief Set publisher with validation
 */
void Book::setPublisher(const QString& publisher) {
    if (publisher.isEmpty()) {
        throw ResourceException("Publisher cannot be empty");
    }
    m_publisher = publisher;
}

/**
 * @brief Set page count with validation
 */
void Book::setPageCount(int pageCount) {
    if (pageCount < 0) {
        throw ResourceException("Page count cannot be negative");
    }
    m_pageCount = pageCount;
}

/**
 * @brief Set language
 */
void Book::setLanguage(const QString& language) {
    m_language = language;
}

/**
 * @brief Set genre
 */
void Book::setGenre(const QString& genre) {
    m_genre = genre;
}

/**
 * @brief Set hardcover status
 */
void Book::setHardcover(bool isHardcover) {
    m_isHardcover = isHardcover;
}

/**
 * @brief Get formatted details for display
 */
QString Book::getFormattedDetails() const {
    return QString("%1 by %2 (%3) - %4")
           .arg(getTitle())
           .arg(getAuthor())
           .arg(getPublicationYear())
           .arg(statusToString(getStatus()));
}

/**
 * @brief Validate ISBN format (basic validation)
 */
bool Book::isValidIsbn(const QString& isbn) const {
    if (isbn.isEmpty()) return false;
    
    // Remove hyphens and spaces for validation
    QString cleanIsbn = isbn;
    cleanIsbn.remove('-').remove(' ');
    
    // Check for ISBN-10 or ISBN-13 format
    if (cleanIsbn.length() == 10 || cleanIsbn.length() == 13) {
        // Basic check: all characters should be digits (except last char of ISBN-10 can be 'X')
        QRegularExpression isbnRegex("^\\d{9}[\\dX]$|^\\d{13}$");
        return isbnRegex.match(cleanIsbn).hasMatch();
    }
    
    return false;
}

/**
 * @brief Validate book-specific data
 */
void Book::validateBookData() const {
    if (!m_isbn.isEmpty() && !isValidIsbn(m_isbn)) {
        throw ResourceException("Invalid ISBN format");
    }
    if (m_publisher.isEmpty()) {
        throw ResourceException("Publisher cannot be empty");
    }
    if (m_pageCount < 0) {
        throw ResourceException("Page count cannot be negative");
    }
}
