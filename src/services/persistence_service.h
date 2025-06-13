#ifndef PERSISTENCE_SERVICE_H
#define PERSISTENCE_SERVICE_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <vector>
#include <memory>

// Forward declarations
class Resource;
class Book;
class Article;
class User;
class Loan;
class Reservation;
class LibraryManager;

/**
 * @brief Service class for handling data persistence using JSON files
 * 
 * This class manages saving and loading of all library data to/from JSON files.
 * It provides methods for serializing and deserializing library objects.
 */
class PersistenceService {
private:
    QString m_dataDirectory;
    QString m_resourcesFile;
    QString m_usersFile;
    QString m_loansFile;
    QString m_reservationsFile;
    QString m_configFile;

public:
    // Constructor
    explicit PersistenceService(const QString& dataDirectory = "data");
    
    // Main persistence operations
    bool saveLibraryData(const LibraryManager& libraryManager);
    bool loadLibraryData(LibraryManager& libraryManager);
    
    // Individual data type operations
    bool saveResources(const std::vector<std::unique_ptr<Resource>>& resources);
    bool loadResources(std::vector<std::unique_ptr<Resource>>& resources);
    
    bool saveUsers(const std::vector<std::unique_ptr<User>>& users);
    bool loadUsers(std::vector<std::unique_ptr<User>>& users);
    
    bool saveLoans(const std::vector<std::unique_ptr<Loan>>& activeLoans,
                   const std::vector<std::unique_ptr<Loan>>& loanHistory);
    bool loadLoans(std::vector<std::unique_ptr<Loan>>& activeLoans,
                   std::vector<std::unique_ptr<Loan>>& loanHistory);
    
    bool saveReservations(const std::vector<std::unique_ptr<Reservation>>& activeReservations,
                         const std::vector<std::unique_ptr<Reservation>>& reservationHistory);
    bool loadReservations(std::vector<std::unique_ptr<Reservation>>& activeReservations,
                         std::vector<std::unique_ptr<Reservation>>& reservationHistory);
    
    bool saveConfiguration(const QJsonObject& config);
    bool loadConfiguration(QJsonObject& config);
    
    // File management
    bool initializeDataDirectory();
    bool backupData(const QString& backupSuffix = "");
    bool restoreFromBackup(const QString& backupSuffix = "");
    
    // Validation and error handling
    bool validateJsonStructure(const QJsonDocument& doc, const QString& expectedType);
    QString getLastError() const { return m_lastError; }
    
    // Utility functions
    QString getResourcesFilePath() const { return m_resourcesFile; }
    QString getUsersFilePath() const { return m_usersFile; }
    QString getLoansFilePath() const { return m_loansFile; }
    QString getReservationsFilePath() const { return m_reservationsFile; }
    QString getConfigFilePath() const { return m_configFile; }
    
    // Static utility functions
    static QJsonObject createResourceJson(const Resource& resource);
    static std::unique_ptr<Resource> createResourceFromJson(const QJsonObject& json);
    static QJsonObject createUserJson(const User& user);
    static std::unique_ptr<User> createUserFromJson(const QJsonObject& json);
    static QJsonObject createLoanJson(const Loan& loan);
    static std::unique_ptr<Loan> createLoanFromJson(const QJsonObject& json);
    static QJsonObject createReservationJson(const Reservation& reservation);
    static std::unique_ptr<Reservation> createReservationFromJson(const QJsonObject& json);
      // Error recovery
    bool attemptDataRecovery();

private:
    QString m_lastError;
    
    // File I/O helpers
    bool writeJsonToFile(const QString& filePath, const QJsonDocument& document);
    bool readJsonFromFile(const QString& filePath, QJsonDocument& document);
    
    // JSON processing helpers
    QJsonArray resourcesToJsonArray(const std::vector<std::unique_ptr<Resource>>& resources);
    bool jsonArrayToResources(const QJsonArray& jsonArray, std::vector<std::unique_ptr<Resource>>& resources);
    
    QJsonArray usersToJsonArray(const std::vector<std::unique_ptr<User>>& users);
    bool jsonArrayToUsers(const QJsonArray& jsonArray, std::vector<std::unique_ptr<User>>& users);
    
    QJsonArray loansToJsonArray(const std::vector<std::unique_ptr<Loan>>& loans);
    bool jsonArrayToLoans(const QJsonArray& jsonArray, std::vector<std::unique_ptr<Loan>>& loans);
    
    QJsonArray reservationsToJsonArray(const std::vector<std::unique_ptr<Reservation>>& reservations);
    bool jsonArrayToReservations(const QJsonArray& jsonArray, std::vector<std::unique_ptr<Reservation>>& reservations);
    
    // Validation helpers
    bool validateResourceJson(const QJsonObject& json);
    bool validateUserJson(const QJsonObject& json);
    bool validateLoanJson(const QJsonObject& json);
    bool validateReservationJson(const QJsonObject& json);
    
    // Error handling
    void setError(const QString& error);
    void clearError();
};

// Custom exception class for persistence-related errors
class PersistenceException : public std::exception {
private:
    QString m_message;

public:
    explicit PersistenceException(const QString& message) : m_message(message) {}
    const char* what() const noexcept override {
        return m_message.toStdString().c_str();
    }
    QString getMessage() const { return m_message; }
};

#endif // PERSISTENCE_SERVICE_H
