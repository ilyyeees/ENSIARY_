#include "persistence_service.h"
#include "library_manager.h"
#include "../models/resource.h"
#include "../models/book.h"
#include "../models/article.h"
#include "../models/thesis.h"
#include "../models/digitalcontent.h"
#include "../models/user.h"
#include "../models/loan.h"
#include "../models/reservation.h"
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QDateTime>
#include <QDebug>

/**
 * @brief Constructor for PersistenceService
 */
PersistenceService::PersistenceService(const QString& dataDirectory)
    : m_dataDirectory(dataDirectory) {
      // Set up file paths
    m_resourcesFile = m_dataDirectory + "/resources.json";
    m_usersFile = m_dataDirectory + "/users.json";
    m_loansFile = m_dataDirectory + "/loans.json";
    m_reservationsFile = m_dataDirectory + "/reservations.json";
    m_configFile = m_dataDirectory + "/config.json";
    
    // Initialize data directory
    initializeDataDirectory();
}

/**
 * @brief Save all library data
 */
bool PersistenceService::saveLibraryData(const LibraryManager& libraryManager) {
    clearError();
    
    try {
        bool success = true;
        
        // Save configuration
        QJsonObject config;
        config["libraryName"] = libraryManager.getLibraryName();
        config["operatingHours"] = libraryManager.getOperatingHours();
        config["defaultLoanPeriod"] = libraryManager.getDefaultLoanPeriod();
        
        QJsonArray eventsArray;
        for (const QString& event : libraryManager.getUpcomingEvents()) {
            eventsArray.append(event);
        }
        config["upcomingEvents"] = eventsArray;
        config["lastSaved"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        success &= saveConfiguration(config);
        
        // Save resources - convert raw pointers to owned objects for saving
        auto resources = libraryManager.getAllResources();
        std::vector<std::unique_ptr<Resource>> resourcesCopy;
        for (const Resource* resource : resources) {
            if (const Book* book = dynamic_cast<const Book*>(resource)) {
                resourcesCopy.push_back(std::make_unique<Book>(*book));
            } else if (const Article* article = dynamic_cast<const Article*>(resource)) {
                resourcesCopy.push_back(std::make_unique<Article>(*article));
            }
        }
        success &= saveResources(resourcesCopy);
        
        // Save users
        auto users = libraryManager.getAllUsers();
        std::vector<std::unique_ptr<User>> usersCopy;
        for (const User* user : users) {
            usersCopy.push_back(std::make_unique<User>(*user));
        }        success &= saveUsers(usersCopy);
        
        // Save loans
        auto activeLoans = libraryManager.getActiveLoans();
        auto loanHistory = libraryManager.getLoanHistory();
        std::vector<std::unique_ptr<Loan>> activeLoansCopy;
        std::vector<std::unique_ptr<Loan>> loanHistoryCopy;
        
        for (const Loan* loan : activeLoans) {
            activeLoansCopy.push_back(std::make_unique<Loan>(*loan));
        }
        for (const Loan* loan : loanHistory) {
            loanHistoryCopy.push_back(std::make_unique<Loan>(*loan));
        }
        success &= saveLoans(activeLoansCopy, loanHistoryCopy);
        
        // Save reservations
        auto activeReservations = libraryManager.getActiveReservations();
        auto reservationHistory = libraryManager.getReservationHistory();
        std::vector<std::unique_ptr<Reservation>> activeReservationsCopy;
        std::vector<std::unique_ptr<Reservation>> reservationHistoryCopy;
        
        for (const Reservation* reservation : activeReservations) {
            activeReservationsCopy.push_back(std::make_unique<Reservation>(*reservation));
        }
        for (const Reservation* reservation : reservationHistory) {
            reservationHistoryCopy.push_back(std::make_unique<Reservation>(*reservation));
        }
        success &= saveReservations(activeReservationsCopy, reservationHistoryCopy);
        
        return success;
        
    } catch (const std::exception& e) {
        setError(QString("Failed to save library data: %1").arg(e.what()));
        return false;
    }
}

/**
 * @brief Load all library data
 */
bool PersistenceService::loadLibraryData(LibraryManager& libraryManager) {
    clearError();
    
    try {
        // Load configuration first
        QJsonObject config;        if (loadConfiguration(config)) {
            libraryManager.setLibraryName(config["libraryName"].toString());
            libraryManager.setOperatingHours(config["operatingHours"].toString());
            libraryManager.setDefaultLoanPeriod(config["defaultLoanPeriod"].toInt());
            
            // Load upcoming events
            QJsonArray eventsArray = config["upcomingEvents"].toArray();
            for (const QJsonValue& value : eventsArray) {
                libraryManager.addUpcomingEvent(value.toString());
            }
        }
        
        // Load actual data from files
        bool success = true;
        
        // Load resources
        std::vector<std::unique_ptr<Resource>> resources;
        if (loadResources(resources)) {
            for (auto& resource : resources) {
                libraryManager.addResource(std::move(resource));
            }
        } else {
            qDebug() << "No resources file found, starting with empty resources";
        }
        
        // Load users
        std::vector<std::unique_ptr<User>> users;
        if (loadUsers(users)) {
            for (auto& user : users) {
                libraryManager.addUser(std::move(user));
            }
        } else {
            qDebug() << "No users file found, starting with empty users";
        }
          // Load loans
        std::vector<std::unique_ptr<Loan>> activeLoans;
        std::vector<std::unique_ptr<Loan>> loanHistory;
        if (loadLoans(activeLoans, loanHistory)) {
            for (auto& loan : activeLoans) {
                libraryManager.addActiveLoan(std::move(loan));
            }
            for (auto& loan : loanHistory) {
                libraryManager.addLoanHistory(std::move(loan));
            }
        } else {
            qDebug() << "No loans file found, starting with empty loans";
        }
        
        // Load reservations
        std::vector<std::unique_ptr<Reservation>> activeReservations;
        std::vector<std::unique_ptr<Reservation>> reservationHistory;
        if (loadReservations(activeReservations, reservationHistory)) {
            for (auto& reservation : activeReservations) {
                libraryManager.addActiveReservation(std::move(reservation));
            }
            for (auto& reservation : reservationHistory) {
                libraryManager.addReservationHistory(std::move(reservation));
            }
        } else {
            qDebug() << "No reservations file found, starting with empty reservations";
        }        // Start with empty data - no sample data creation
        qDebug() << "Library data loaded successfully. Starting with clean slate.";
        
        return success;
        
    } catch (const std::exception& e) {
        setError(QString("Failed to load library data: %1").arg(e.what()));
        return false;
    }
}

/**
 * @brief Save resources to JSON file
 */
bool PersistenceService::saveResources(const std::vector<std::unique_ptr<Resource>>& resources) {
    clearError();
    
    try {
        QJsonArray resourcesArray = resourcesToJsonArray(resources);
        
        QJsonObject root;
        root["version"] = "1.0";
        root["type"] = "resources";
        root["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        root["count"] = resourcesArray.size();
        root["data"] = resourcesArray;
        
        QJsonDocument document(root);
        return writeJsonToFile(m_resourcesFile, document);
        
    } catch (const std::exception& e) {
        setError(QString("Failed to save resources: %1").arg(e.what()));
        return false;
    }
}

/**
 * @brief Load resources from JSON file
 */
bool PersistenceService::loadResources(std::vector<std::unique_ptr<Resource>>& resources) {
    clearError();
    
    try {
        QJsonDocument document;
        if (!readJsonFromFile(m_resourcesFile, document)) {
            return false;
        }
        
        if (!validateJsonStructure(document, "resources")) {
            return false;
        }
        
        QJsonObject root = document.object();
        QJsonArray resourcesArray = root["data"].toArray();
        
        return jsonArrayToResources(resourcesArray, resources);
        
    } catch (const std::exception& e) {
        setError(QString("Failed to load resources: %1").arg(e.what()));
        return false;
    }
}

/**
 * @brief Save users to JSON file
 */
bool PersistenceService::saveUsers(const std::vector<std::unique_ptr<User>>& users) {
    clearError();
    
    try {
        QJsonArray usersArray = usersToJsonArray(users);
        
        QJsonObject root;
        root["version"] = "1.0";
        root["type"] = "users";
        root["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        root["count"] = usersArray.size();
        root["data"] = usersArray;
        
        QJsonDocument document(root);
        return writeJsonToFile(m_usersFile, document);
        
    } catch (const std::exception& e) {
        setError(QString("Failed to save users: %1").arg(e.what()));
        return false;
    }
}

/**
 * @brief Load users from JSON file
 */
bool PersistenceService::loadUsers(std::vector<std::unique_ptr<User>>& users) {
    clearError();
    
    try {
        QJsonDocument document;
        if (!readJsonFromFile(m_usersFile, document)) {
            return false;
        }
        
        if (!validateJsonStructure(document, "users")) {
            return false;
        }
        
        QJsonObject root = document.object();
        QJsonArray usersArray = root["data"].toArray();
        
        return jsonArrayToUsers(usersArray, users);
        
    } catch (const std::exception& e) {
        setError(QString("Failed to load users: %1").arg(e.what()));
        return false;
    }
}

/**
 * @brief Save loans to JSON file
 */
bool PersistenceService::saveLoans(const std::vector<std::unique_ptr<Loan>>& activeLoans,
                                  const std::vector<std::unique_ptr<Loan>>& loanHistory) {
    clearError();
    
    try {
        QJsonArray activeLoansArray = loansToJsonArray(activeLoans);
        QJsonArray loanHistoryArray = loansToJsonArray(loanHistory);
        
        QJsonObject root;
        root["version"] = "1.0";
        root["type"] = "loans";
        root["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        root["activeLoansCount"] = activeLoansArray.size();
        root["loanHistoryCount"] = loanHistoryArray.size();
        root["activeLoans"] = activeLoansArray;
        root["loanHistory"] = loanHistoryArray;
        
        QJsonDocument document(root);
        return writeJsonToFile(m_loansFile, document);
        
    } catch (const std::exception& e) {
        setError(QString("Failed to save loans: %1").arg(e.what()));
        return false;
    }
}

/**
 * @brief Load loans from JSON file
 */
bool PersistenceService::loadLoans(std::vector<std::unique_ptr<Loan>>& activeLoans,
                                  std::vector<std::unique_ptr<Loan>>& loanHistory) {
    clearError();
    
    try {
        QJsonDocument document;
        if (!readJsonFromFile(m_loansFile, document)) {
            return false;
        }
        
        if (!validateJsonStructure(document, "loans")) {
            return false;
        }
        
        QJsonObject root = document.object();
        QJsonArray activeLoansArray = root["activeLoans"].toArray();
        QJsonArray loanHistoryArray = root["loanHistory"].toArray();
        
        bool success = true;
        success &= jsonArrayToLoans(activeLoansArray, activeLoans);
        success &= jsonArrayToLoans(loanHistoryArray, loanHistory);
        
        return success;
        
    } catch (const std::exception& e) {
        setError(QString("Failed to load loans: %1").arg(e.what()));
        return false;
    }
}

/**
 * @brief Save reservations to JSON file
 */
bool PersistenceService::saveReservations(const std::vector<std::unique_ptr<Reservation>>& activeReservations,
                                          const std::vector<std::unique_ptr<Reservation>>& reservationHistory) {
    clearError();
    
    try {
        QJsonArray activeReservationsArray = reservationsToJsonArray(activeReservations);
        QJsonArray reservationHistoryArray = reservationsToJsonArray(reservationHistory);
        
        QJsonObject root;
        root["version"] = "1.0";
        root["type"] = "reservations";
        root["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        root["activeReservationsCount"] = activeReservationsArray.size();
        root["reservationHistoryCount"] = reservationHistoryArray.size();
        root["activeReservations"] = activeReservationsArray;
        root["reservationHistory"] = reservationHistoryArray;
        
        QJsonDocument document(root);
        return writeJsonToFile(m_reservationsFile, document);
        
    } catch (const std::exception& e) {
        setError(QString("Failed to save reservations: %1").arg(e.what()));
        return false;
    }
}

/**
 * @brief Load reservations from JSON file
 */
bool PersistenceService::loadReservations(std::vector<std::unique_ptr<Reservation>>& activeReservations,
                                          std::vector<std::unique_ptr<Reservation>>& reservationHistory) {
    clearError();
    
    try {
        QJsonDocument document;
        if (!readJsonFromFile(m_reservationsFile, document)) {
            return false;
        }
        
        if (!validateJsonStructure(document, "reservations")) {
            return false;
        }
        
        QJsonObject root = document.object();
        QJsonArray activeReservationsArray = root["activeReservations"].toArray();
        QJsonArray reservationHistoryArray = root["reservationHistory"].toArray();
        
        bool success = true;
        success &= jsonArrayToReservations(activeReservationsArray, activeReservations);
        success &= jsonArrayToReservations(reservationHistoryArray, reservationHistory);
        
        return success;
        
    } catch (const std::exception& e) {
        setError(QString("Failed to load reservations: %1").arg(e.what()));
        return false;
    }
}

/**
 * @brief Save configuration to JSON file
 */
bool PersistenceService::saveConfiguration(const QJsonObject& config) {
    clearError();
    
    try {
        QJsonObject root;
        root["version"] = "1.0";
        root["type"] = "configuration";
        root["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        root["data"] = config;
        
        QJsonDocument document(root);
        return writeJsonToFile(m_configFile, document);
        
    } catch (const std::exception& e) {
        setError(QString("Failed to save configuration: %1").arg(e.what()));
        return false;
    }
}

/**
 * @brief Load configuration from JSON file
 */
bool PersistenceService::loadConfiguration(QJsonObject& config) {
    clearError();
    
    try {
        QJsonDocument document;
        if (!readJsonFromFile(m_configFile, document)) {
            return false;
        }
        
        if (!validateJsonStructure(document, "configuration")) {
            return false;
        }
        
        QJsonObject root = document.object();
        config = root["data"].toObject();
        
        return true;
        
    } catch (const std::exception& e) {
        setError(QString("Failed to load configuration: %1").arg(e.what()));
        return false;
    }
}

/**
 * @brief Initialize data directory
 */
bool PersistenceService::initializeDataDirectory() {
    QDir dir;
    if (!dir.exists(m_dataDirectory)) {
        if (!dir.mkpath(m_dataDirectory)) {
            setError("Failed to create data directory: " + m_dataDirectory);
            return false;
        }
    }
    return true;
}

/**
 * @brief Backup data files
 */
bool PersistenceService::backupData(const QString& backupSuffix) {
    QString suffix = backupSuffix.isEmpty() ? 
                    QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") : 
                    backupSuffix;
    
    QStringList filesToBackup = {m_resourcesFile, m_usersFile, m_loansFile, m_configFile};
    
    for (const QString& file : filesToBackup) {
        if (QFile::exists(file)) {
            QString backupFile = file + ".backup_" + suffix;
            if (!QFile::copy(file, backupFile)) {
                setError("Failed to backup file: " + file);
                return false;
            }
        }
    }
    
    return true;
}

/**
 * @brief Restore from backup
 */
bool PersistenceService::restoreFromBackup(const QString& backupSuffix) {
    QStringList filesToRestore = {m_resourcesFile, m_usersFile, m_loansFile, m_configFile};
    
    for (const QString& file : filesToRestore) {
        QString backupFile = file + ".backup_" + backupSuffix;
        if (QFile::exists(backupFile)) {
            if (QFile::exists(file)) {
                QFile::remove(file);
            }
            if (!QFile::copy(backupFile, file)) {
                setError("Failed to restore file: " + file);
                return false;
            }
        }
    }
    
    return true;
}

/**
 * @brief Validate JSON structure
 */
bool PersistenceService::validateJsonStructure(const QJsonDocument& doc, const QString& expectedType) {
    if (!doc.isObject()) {
        setError("Invalid JSON document: not an object");
        return false;
    }
    
    QJsonObject root = doc.object();
    
    if (!root.contains("type") || root["type"].toString() != expectedType) {
        setError("Invalid JSON type: expected " + expectedType);
        return false;
    }
    
    if (!root.contains("version")) {
        setError("Missing version information");
        return false;
    }
    
    return true;
}

/**
 * @brief Attempt data recovery
 */
bool PersistenceService::attemptDataRecovery() {
    // Try to find the most recent backup
    QDir dataDir(m_dataDirectory);
    QStringList backupFiles = dataDir.entryList(QStringList() << "*.backup_*", QDir::Files);
    
    if (backupFiles.isEmpty()) {
        setError("No backup files found for recovery");
        return false;
    }
    
    // Find the most recent backup (this is simplified)
    QString mostRecentBackup;
    for (const QString& backup : backupFiles) {
        if (backup.contains("config.json.backup_")) {
            mostRecentBackup = backup.mid(backup.indexOf("backup_") + 7);
            break;
        }
    }
    
    if (mostRecentBackup.isEmpty()) {
        setError("Could not determine backup suffix");
        return false;
    }
    
    return restoreFromBackup(mostRecentBackup);
}

/*
bool PersistenceService::createSampleData() {
    // This creates sample data when no data files exist
    // In a real application, this would only be called on first run
    
    try {
        // Create sample configuration
        QJsonObject config;
        config["libraryName"] = "ENSIARY Library Management System";
        config["operatingHours"] = "Monday-Friday: 8:00 AM - 8:00 PM, Saturday-Sunday: 10:00 AM - 6:00 PM";
        config["dailyFineRate"] = 0.50;
        config["defaultLoanPeriod"] = 14;
        config["firstRun"] = true;
        
        QJsonArray events;
        events.append("Book Club Meeting - June 15, 2025");
        events.append("Research Workshop - June 20, 2025");
        events.append("Digital Resources Training - June 25, 2025");
        config["upcomingEvents"] = events;
        
        return saveConfiguration(config);
          } catch (const std::exception& e) {
        setError(QString("Failed to create sample data: %1").arg(e.what()));
        return false;
    }
}
*/

/**
 * @brief Validate resource JSON object
 */
bool PersistenceService::validateResourceJson(const QJsonObject& json) {
    // Check required fields for a resource
    QStringList requiredFields = {"id", "title", "type", "author", "year", "isAvailable"};
    
    for (const QString& field : requiredFields) {
        if (!json.contains(field)) {
            setError(QString("Missing required field in resource JSON: %1").arg(field));
            return false;
        }
    }
    
    return true;
}

/**
 * @brief Validate user JSON object
 */
bool PersistenceService::validateUserJson(const QJsonObject& json) {
    // Check required fields for a user
    QStringList requiredFields = {"id", "name", "email", "type", "status", "registrationDate"};
    
    for (const QString& field : requiredFields) {
        if (!json.contains(field)) {
            setError(QString("Missing required field in user JSON: %1").arg(field));
            return false;
        }
    }
    
    return true;
}

/**
 * @brief Validate loan JSON object
 */
bool PersistenceService::validateLoanJson(const QJsonObject& json) {
    // Check required fields for a loan
    QStringList requiredFields = {"id", "userId", "resourceId", "loanDate", "dueDate", "status"};
    
    for (const QString& field : requiredFields) {
        if (!json.contains(field)) {
            setError(QString("Missing required field in loan JSON: %1").arg(field));
            return false;
        }
    }
    
    return true;
}

/**
 * @brief Validate reservation JSON object
 */
bool PersistenceService::validateReservationJson(const QJsonObject& json) {
    // Check required fields for a reservation
    QStringList requiredFields = {"id", "userId", "resourceId", "reservationDate", "expiryDate", "status"};
    
    for (const QString& field : requiredFields) {
        if (!json.contains(field)) {
            setError(QString("Missing required field in reservation JSON: %1").arg(field));
            return false;
        }
    }
    
    return true;
}

// Private helper methods

/**
 * @brief Write JSON document to file
 */
bool PersistenceService::writeJsonToFile(const QString& filePath, const QJsonDocument& document) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        setError("Cannot open file for writing: " + filePath);
        return false;
    }
    
    qint64 bytesWritten = file.write(document.toJson());
    if (bytesWritten == -1) {
        setError("Failed to write to file: " + filePath);
        return false;
    }
    
    return true;
}

/**
 * @brief Read JSON document from file
 */
bool PersistenceService::readJsonFromFile(const QString& filePath, QJsonDocument& document) {
    QFile file(filePath);
    if (!file.exists()) {
        setError("File does not exist: " + filePath);
        return false;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        setError("Cannot open file for reading: " + filePath);
        return false;
    }
    
    QByteArray data = file.readAll();
    QJsonParseError error;
    document = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        setError("JSON parse error: " + error.errorString());
        return false;
    }
    
    return true;
}

/**
 * @brief Convert resources vector to JSON array
 */
QJsonArray PersistenceService::resourcesToJsonArray(const std::vector<std::unique_ptr<Resource>>& resources) {
    QJsonArray array;
    for (const auto& resource : resources) {
        array.append(resource->toJson());
    }
    return array;
}

/**
 * @brief Convert JSON array to resources vector
 */
bool PersistenceService::jsonArrayToResources(const QJsonArray& jsonArray, std::vector<std::unique_ptr<Resource>>& resources) {
    resources.clear();
    
    for (const QJsonValue& value : jsonArray) {
        if (!value.isObject()) {
            setError("Invalid resource JSON: not an object");
            return false;
        }
        
        QJsonObject resourceJson = value.toObject();
        auto resource = createResourceFromJson(resourceJson);
        if (resource) {
            resources.push_back(std::move(resource));
        } else {
            setError("Failed to create resource from JSON");
            return false;
        }
    }
    
    return true;
}

/**
 * @brief Convert users vector to JSON array
 */
QJsonArray PersistenceService::usersToJsonArray(const std::vector<std::unique_ptr<User>>& users) {
    QJsonArray array;
    for (const auto& user : users) {
        array.append(user->toJson());
    }
    return array;
}

/**
 * @brief Convert JSON array to users vector
 */
bool PersistenceService::jsonArrayToUsers(const QJsonArray& jsonArray, std::vector<std::unique_ptr<User>>& users) {
    users.clear();
    
    for (const QJsonValue& value : jsonArray) {
        if (!value.isObject()) {
            setError("Invalid user JSON: not an object");
            return false;
        }
        
        QJsonObject userJson = value.toObject();
        auto user = createUserFromJson(userJson);
        if (user) {
            users.push_back(std::move(user));
        } else {
            setError("Failed to create user from JSON");
            return false;
        }
    }
    
    return true;
}

/**
 * @brief Convert loans vector to JSON array
 */
QJsonArray PersistenceService::loansToJsonArray(const std::vector<std::unique_ptr<Loan>>& loans) {
    QJsonArray array;
    for (const auto& loan : loans) {
        array.append(loan->toJson());
    }
    return array;
}

/**
 * @brief Convert JSON array to loans vector
 */
bool PersistenceService::jsonArrayToLoans(const QJsonArray& jsonArray, std::vector<std::unique_ptr<Loan>>& loans) {
    loans.clear();
    
    for (const QJsonValue& value : jsonArray) {
        if (!value.isObject()) {
            setError("Invalid loan JSON: not an object");
            return false;
        }
        
        QJsonObject loanJson = value.toObject();
        auto loan = createLoanFromJson(loanJson);
        if (loan) {
            loans.push_back(std::move(loan));
        } else {
            setError("Failed to create loan from JSON");
            return false;
        }
    }
    
    return true;
}

/**
 * @brief Convert reservations vector to JSON array
 */
QJsonArray PersistenceService::reservationsToJsonArray(const std::vector<std::unique_ptr<Reservation>>& reservations) {
    QJsonArray array;
    for (const auto& reservation : reservations) {
        array.append(reservation->toJson());
    }
    return array;
}

/**
 * @brief Convert JSON array to reservations vector
 */
bool PersistenceService::jsonArrayToReservations(const QJsonArray& jsonArray, std::vector<std::unique_ptr<Reservation>>& reservations) {
    reservations.clear();
    
    for (const QJsonValue& value : jsonArray) {
        if (!value.isObject()) {
            setError("Invalid reservation JSON: not an object");
            return false;
        }
        
        QJsonObject reservationJson = value.toObject();
        auto reservation = createReservationFromJson(reservationJson);
        if (reservation) {
            reservations.push_back(std::move(reservation));
        } else {
            setError("Failed to create reservation from JSON");
            return false;
        }
    }
    
    return true;
}

// Static utility functions

/**
 * @brief Create resource JSON object
 */
QJsonObject PersistenceService::createResourceJson(const Resource& resource) {
    return resource.toJson();
}

/**
 * @brief Create resource from JSON object
 */
std::unique_ptr<Resource> PersistenceService::createResourceFromJson(const QJsonObject& json) {
    QString type = json["type"].toString();
      if (type == "Book") {
        auto book = std::make_unique<Book>("", "", "", 2000, "", "");
        book->fromJson(json);
        return std::move(book);
    } else if (type == "Article") {
        auto article = std::make_unique<Article>("", "", "", 2000, "");
        article->fromJson(json);
        return std::move(article);    } else if (type == "Thesis") {
        auto thesis = std::make_unique<Thesis>("", "", "", 2000);
        thesis->fromJson(json);
        return std::move(thesis);
    } else if (type == "Digital Content") {
        auto digitalContent = std::make_unique<DigitalContent>("", "", "", 2000);
        digitalContent->fromJson(json);
        return std::move(digitalContent);
    }
    
    return nullptr;
}

/**
 * @brief Create user JSON object
 */
QJsonObject PersistenceService::createUserJson(const User& user) {
    return user.toJson();
}

/**
 * @brief Create user from JSON object
 */
std::unique_ptr<User> PersistenceService::createUserFromJson(const QJsonObject& json) {
    // Extract required fields first to avoid validation issues
    QString userId = json["userId"].toString();
    QString firstName = json["firstName"].toString();
    QString lastName = json["lastName"].toString();
    QString email = json["email"].toString();
    
    // Create user with extracted values
    auto user = std::make_unique<User>(userId, firstName, lastName, email);
      // Set additional fields from JSON
    user->setPhoneNumber(json["phoneNumber"].toString());
    user->setAddress(json["address"].toString());
    user->setUserType(User::stringToUserType(json["userType"].toString()));
    user->setStatus(User::stringToStatus(json["status"].toString()));
    user->setMaxBorrowLimit(json["maxBorrowLimit"].toInt());
    user->setNotes(json["notes"].toString());
    user->setYear(json["year"].toInt(-1));
    
    // Handle loans if present (keep this part from fromJson)
    QJsonArray currentLoansArray = json["currentLoans"].toArray();
    for (const QJsonValue& value : currentLoansArray) {
        auto loan = std::make_unique<Loan>("", "", "", "", QDateTime(), QDateTime());
        loan->fromJson(value.toObject());
        user->addCurrentLoan(std::move(loan));
    }
    
    return user;
}

/**
 * @brief Create loan JSON object
 */
QJsonObject PersistenceService::createLoanJson(const Loan& loan) {
    return loan.toJson();
}

/**
 * @brief Create loan from JSON object
 */
std::unique_ptr<Loan> PersistenceService::createLoanFromJson(const QJsonObject& json) {
    auto loan = std::make_unique<Loan>("", "", "", "", QDateTime(), QDateTime());
    loan->fromJson(json);
    return loan;
}

/**
 * @brief Create reservation JSON object
 */
QJsonObject PersistenceService::createReservationJson(const Reservation& reservation) {
    return reservation.toJson();
}

/**
 * @brief Create reservation from JSON object
 */
std::unique_ptr<Reservation> PersistenceService::createReservationFromJson(const QJsonObject& json) {
    auto reservation = std::make_unique<Reservation>("", "", "", 7);
    reservation->fromJson(json);
    return reservation;
}

/**
 * @brief Set error message
 */
void PersistenceService::setError(const QString& error) {
    m_lastError = error;
    qDebug() << "PersistenceService Error:" << error;
}

/**
 * @brief Clear error message
 */
void PersistenceService::clearError() {
    m_lastError.clear();
}

/*
void PersistenceService::createSampleDataForLibraryManager(LibraryManager& libraryManager) {
    qDebug() << "Creating sample users and resources...";
    
    // Create sample users
    auto user1 = std::make_unique<User>("USR001", "John", "Doe", "john.doe@school.edu", User::UserType::Student);
    user1->setPhone("123-456-7890");
    user1->setAddress("123 Student Lane");
    user1->setMaxLoans(5);
    libraryManager.addUser(std::move(user1));
    
    auto user2 = std::make_unique<User>("USR002", "Jane", "Smith", "jane.smith@school.edu", User::UserType::Teacher);
    user2->setPhone("123-456-7891");
    user2->setAddress("456 Faculty Ave");
    user2->setMaxLoans(10);
    libraryManager.addUser(std::move(user2));
    
    auto user3 = std::make_unique<User>("USR003", "Bob", "Wilson", "bob.wilson@school.edu", User::UserType::Staff);
    user3->setPhone("123-456-7892");
    user3->setAddress("789 Staff Street");
    user3->setMaxLoans(8);
    libraryManager.addUser(std::move(user3));
      // Create sample books
    auto book1 = std::make_unique<Book>("BK001", "Introduction to Programming", "John Smith", 2023, "978-0123456789", "Tech Publishing");
    book1->setGenre("Computer Science");
    book1->setPageCount(450);
    libraryManager.addResource(std::move(book1));
    
    auto book2 = std::make_unique<Book>("BK002", "Mathematics for Engineers", "Sarah Johnson", 2022, "978-0987654321", "Academic Press");
    book2->setGenre("Mathematics");
    book2->setPageCount(680);
    libraryManager.addResource(std::move(book2));
    
    auto book3 = std::make_unique<Book>("BK003", "History of Science", "Michael Brown", 2024, "978-0456789123", "History Books Inc");
    book3->setGenre("Science History");
    book3->setPageCount(320);
    libraryManager.addResource(std::move(book3));
    
    // Create sample articles
    auto article1 = std::make_unique<Article>("ART001", "Climate Change Research", "Dr. Emily Davis", 2024, "Environmental Science Quarterly");
    article1->setVolume(45);
    article1->setIssue(3);
    article1->setPageRange("23-45");
    libraryManager.addResource(std::move(article1));
    
    auto article2 = std::make_unique<Article>("ART002", "Modern Educational Methods", "Prof. David Lee", 2023, "Education Today");
    article2->setVolume(12);
    article2->setIssue(7);
    article2->setPageRange("156-172");
    libraryManager.addResource(std::move(article2));
      qDebug() << "Sample data created successfully!";
}
*/
