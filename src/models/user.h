#ifndef USER_H
#define USER_H

#include <QString>
#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QJsonObject>
#include <vector>
#include <memory>

#include "loan.h"

/**
 * @brief Represents a library user
 * 
 * This class manages user information and demonstrates composition
 * by containing a collection of Loan objects.
 */
class User {
public:    enum class UserType {
        Student,
        Teacher,
        Staff,
        Administrator,
        Guest
    };
    
    // Alias for compatibility
    using Type = UserType;

    enum class Status {
        Active,
        Inactive,
        Suspended,
        Expired
    };

private:
    QString m_userId;    QString m_firstName;
    QString m_lastName;
    QString m_email;
    QString m_phoneNumber;
    QString m_address;
    UserType m_userType;
    Status m_status;
    QDateTime m_registrationDate;
    QDateTime m_lastActivity;    int m_maxBorrowLimit;
    QString m_notes;
    int m_year; // For students: 1st-5th year, -1 for non-students
    
    // Composition: User "has-a" collection of loans
    std::vector<std::unique_ptr<Loan>> m_currentLoans;
    std::vector<std::unique_ptr<Loan>> m_loanHistory;

public:    // Constructor
    User(const QString& userId, const QString& firstName, const QString& lastName,
         const QString& email, UserType userType = UserType::Student);

    // Destructor
    ~User() = default;

    // Copy constructor and assignment operator (for deep copying loans)
    User(const User& other);
    User& operator=(const User& other);

    // Move constructor and assignment operator
    User(User&& other) noexcept = default;
    User& operator=(User&& other) noexcept = default;

    // Getters
    QString getUserId() const { return m_userId; }
    QString getFirstName() const { return m_firstName; }
    QString getLastName() const { return m_lastName; }
    QString getFullName() const { return m_firstName + " " + m_lastName; }
    QString getEmail() const { return m_email; }
    QString getPhoneNumber() const { return m_phoneNumber; }
    QString getAddress() const { return m_address; }
    UserType getUserType() const { return m_userType; }
    Status getStatus() const { return m_status; }    QDateTime getRegistrationDate() const { return m_registrationDate; }
    QDateTime getLastActivity() const { return m_lastActivity; }    int getMaxBorrowLimit() const { return m_maxBorrowLimit; }
    QString getNotes() const { return m_notes; }
    int getYear() const { return m_year; }
      // Additional getters for dialog compatibility
    QString getId() const { return m_userId; }
    QString getName() const { return getFullName(); }
    QString getPhone() const { return m_phoneNumber; }
    int getMaxLoans() const { return m_maxBorrowLimit; }

    // Setters with validation
    void setFirstName(const QString& firstName);
    void setLastName(const QString& lastName);
    void setEmail(const QString& email);
    void setPhoneNumber(const QString& phoneNumber);    void setAddress(const QString& address);
    void setUserType(UserType userType);
    void setStatus(Status status);
    void setMaxBorrowLimit(int limit);
    void setMaxLoans(int limit) { setMaxBorrowLimit(limit); } // Alias for compatibility
    void setNotes(const QString& notes);
    void setYear(int year); // For students: 1-5, for others: -1
    void updateLastActivity();
    
    // Additional setters for dialog compatibility
    void setPhone(const QString& phone) { setPhoneNumber(phone); }

    // Loan management
    void addCurrentLoan(std::unique_ptr<Loan> loan);
    void moveLoanToHistory(const QString& loanId);
    std::vector<Loan*> getCurrentLoans() const;
    std::vector<Loan*> getLoanHistory() const;
    Loan* findCurrentLoan(const QString& loanId);
    Loan* findLoanInHistory(const QString& loanId);
      // User status checks
    bool canBorrow() const;
    bool hasOverdueItems() const;
    bool hasMaxLoansReached() const;
    int getCurrentLoanCount() const;

    // JSON serialization
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);

    // Utility functions
    QString getFormattedInfo() const;
    QString getUserTypeString() const;
    QString getStatusString() const;    // Static utility functions
    static QString userTypeToString(UserType type);
    static UserType stringToUserType(const QString& typeStr);
    static QString statusToString(Status status);
    static Status stringToStatus(const QString& statusStr);
    static bool isValidEmail(const QString& email);
    
    // Convenience methods for dialog compatibility
    static QString typeToString(Type type) { return userTypeToString(type); }
    Type getType() const { return m_userType; }

    // Comparison operators
    bool operator==(const User& other) const;
    bool operator<(const User& other) const;

private:
    void validateUserData() const;
    void setDefaultBorrowLimit();
    std::unique_ptr<Loan> cloneLoan(const Loan& loan) const;
};

// Custom exception class for User-related errors
class UserException : public std::exception {
private:
    QString m_message;

public:
    explicit UserException(const QString& message) : m_message(message) {}
    const char* what() const noexcept override {
        return m_message.toStdString().c_str();
    }
    QString getMessage() const { return m_message; }
};

#endif // USER_H
