#include "user.h"
#include "loan.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <algorithm>

/**
 * @brief Constructor for User class
 */
User::User(const QString& userId, const QString& firstName, const QString& lastName,
           const QString& email, UserType userType)    : m_userId(userId), m_firstName(firstName), m_lastName(lastName),
      m_email(email), m_userType(userType), m_status(Status::Active),
      m_registrationDate(QDateTime::currentDateTime()),
      m_lastActivity(QDateTime::currentDateTime()), 
      m_year(userType == UserType::Student ? 1 : -1) { // Default to 1st year for students, -1 for others
    
    validateUserData();
    setDefaultBorrowLimit();
    updateLastActivity();
}

/**
 * @brief Copy constructor for deep copying loans
 */
User::User(const User& other)    : m_userId(other.m_userId), m_firstName(other.m_firstName), m_lastName(other.m_lastName),
      m_email(other.m_email), m_phoneNumber(other.m_phoneNumber), m_address(other.m_address),
      m_userType(other.m_userType), m_status(other.m_status),
      m_registrationDate(other.m_registrationDate), m_lastActivity(other.m_lastActivity),
      m_maxBorrowLimit(other.m_maxBorrowLimit), m_notes(other.m_notes), m_year(other.m_year) {
    
    // Deep copy current loans
    for (const auto& loan : other.m_currentLoans) {
        m_currentLoans.push_back(cloneLoan(*loan));
    }
    
    // Deep copy loan history
    for (const auto& loan : other.m_loanHistory) {
        m_loanHistory.push_back(cloneLoan(*loan));
    }
}

/**
 * @brief Assignment operator for deep copying loans
 */
User& User::operator=(const User& other) {
    if (this != &other) {
        m_userId = other.m_userId;
        m_firstName = other.m_firstName;        m_lastName = other.m_lastName;
        m_email = other.m_email;
        m_phoneNumber = other.m_phoneNumber;
        m_address = other.m_address;
        m_userType = other.m_userType;
        m_status = other.m_status;
        m_registrationDate = other.m_registrationDate;        m_lastActivity = other.m_lastActivity;
        m_maxBorrowLimit = other.m_maxBorrowLimit;
        m_notes = other.m_notes;
        m_year = other.m_year;
        
        // Clear existing loans
        m_currentLoans.clear();
        m_loanHistory.clear();
        
        // Deep copy current loans
        for (const auto& loan : other.m_currentLoans) {
            m_currentLoans.push_back(cloneLoan(*loan));
        }
        
        // Deep copy loan history
        for (const auto& loan : other.m_loanHistory) {
            m_loanHistory.push_back(cloneLoan(*loan));
        }
    }
    return *this;
}

/**
 * @brief Set first name with validation
 */
void User::setFirstName(const QString& firstName) {
    if (firstName.isEmpty()) {
        throw UserException("First name cannot be empty");
    }
    m_firstName = firstName;
}

/**
 * @brief Set last name with validation
 */
void User::setLastName(const QString& lastName) {
    if (lastName.isEmpty()) {
        throw UserException("Last name cannot be empty");
    }
    m_lastName = lastName;
}

/**
 * @brief Set email with validation
 */
void User::setEmail(const QString& email) {
    if (!isValidEmail(email)) {
        throw UserException("Invalid email format");
    }
    m_email = email;
}

/**
 * @brief Set phone number
 */
void User::setPhoneNumber(const QString& phoneNumber) {
    m_phoneNumber = phoneNumber;
}

/**
 * @brief Set address
 */
void User::setAddress(const QString& address) {
    m_address = address;
}

/**
 * @brief Set user type and update borrow limit
 */
void User::setUserType(UserType userType) {
    m_userType = userType;
    setDefaultBorrowLimit();
}

/**
 * @brief Set user status
 */
void User::setStatus(Status status) {
    m_status = status;
}

/**
 * @brief Set maximum borrow limit
 */
void User::setMaxBorrowLimit(int limit) {
    if (limit < 0) {
        throw UserException("Borrow limit cannot be negative");
    }
    m_maxBorrowLimit = limit;
}

/**
 * @brief Set notes
 */
void User::setNotes(const QString& notes) {
    m_notes = notes;
}

/**
 * @brief Set year for students (1-5), or -1 for non-students
 */
void User::setYear(int year) {
    if (m_userType == UserType::Student) {
        if (year < 1 || year > 5) {
            throw UserException("Student year must be between 1 and 5");
        }
        m_year = year;
    } else {
        m_year = -1; // Non-students don't have a year
    }
}

/**
 * @brief Update last activity to current time
 */
void User::updateLastActivity() {
    m_lastActivity = QDateTime::currentDateTime();
}

/**
 * @brief Add a current loan
 */
void User::addCurrentLoan(std::unique_ptr<Loan> loan) {
    if (!loan) {
        throw UserException("Cannot add null loan");
    }
    
    if (hasMaxLoansReached()) {
        throw UserException("User has reached maximum loan limit");
    }
    
    m_currentLoans.push_back(std::move(loan));
    updateLastActivity();
}

/**
 * @brief Move loan from current to history
 */
void User::moveLoanToHistory(const QString& loanId) {
    auto it = std::find_if(m_currentLoans.begin(), m_currentLoans.end(),
                          [&loanId](const std::unique_ptr<Loan>& loan) {
                              return loan->getLoanId() == loanId;
                          });
    
    if (it != m_currentLoans.end()) {
        m_loanHistory.push_back(std::move(*it));
        m_currentLoans.erase(it);
        updateLastActivity();
    }
}

/**
 * @brief Get current loans as raw pointers
 */
std::vector<Loan*> User::getCurrentLoans() const {
    std::vector<Loan*> loans;
    for (const auto& loan : m_currentLoans) {
        loans.push_back(loan.get());
    }
    return loans;
}

/**
 * @brief Get loan history as raw pointers
 */
std::vector<Loan*> User::getLoanHistory() const {
    std::vector<Loan*> loans;
    for (const auto& loan : m_loanHistory) {
        loans.push_back(loan.get());
    }
    return loans;
}

/**
 * @brief Find current loan by ID
 */
Loan* User::findCurrentLoan(const QString& loanId) {
    auto it = std::find_if(m_currentLoans.begin(), m_currentLoans.end(),
                          [&loanId](const std::unique_ptr<Loan>& loan) {
                              return loan->getLoanId() == loanId;
                          });
    
    return (it != m_currentLoans.end()) ? it->get() : nullptr;
}

/**
 * @brief Find loan in history by ID
 */
Loan* User::findLoanInHistory(const QString& loanId) {
    auto it = std::find_if(m_loanHistory.begin(), m_loanHistory.end(),
                          [&loanId](const std::unique_ptr<Loan>& loan) {
                              return loan->getLoanId() == loanId;
                          });
    
    return (it != m_loanHistory.end()) ? it->get() : nullptr;
}

/**
 * @brief Check if user can borrow items
 */
bool User::canBorrow() const {
    return m_status == Status::Active && 
           !hasMaxLoansReached();
}

/**
 * @brief Check if user has overdue items
 */
bool User::hasOverdueItems() const {
    return std::any_of(m_currentLoans.begin(), m_currentLoans.end(),
                      [](const std::unique_ptr<Loan>& loan) {
                          return loan->isOverdue();
                      });
}

/**
 * @brief Check if user has reached maximum loans
 */
bool User::hasMaxLoansReached() const {
    return static_cast<int>(m_currentLoans.size()) >= m_maxBorrowLimit;
}

/**
 * @brief Get current loan count
 */
int User::getCurrentLoanCount() const {
    return static_cast<int>(m_currentLoans.size());
}

/**
 * @brief Convert user to JSON object
 */
QJsonObject User::toJson() const {
    QJsonObject json;
    json["userId"] = m_userId;
    json["firstName"] = m_firstName;
    json["lastName"] = m_lastName;
    json["email"] = m_email;
    json["phoneNumber"] = m_phoneNumber;
    json["address"] = m_address;
    json["userType"] = userTypeToString(m_userType);
    json["status"] = statusToString(m_status);
    json["registrationDate"] = m_registrationDate.toString(Qt::ISODate);    json["lastActivity"] = m_lastActivity.toString(Qt::ISODate);    json["maxBorrowLimit"] = m_maxBorrowLimit;
    json["notes"] = m_notes;
    json["year"] = m_year;
    
    // Convert current loans to JSON array
    QJsonArray currentLoansArray;
    for (const auto& loan : m_currentLoans) {
        currentLoansArray.append(loan->toJson());
    }
    json["currentLoans"] = currentLoansArray;
    
    // Convert loan history to JSON array
    QJsonArray loanHistoryArray;
    for (const auto& loan : m_loanHistory) {
        loanHistoryArray.append(loan->toJson());
    }
    json["loanHistory"] = loanHistoryArray;
    
    return json;
}

/**
 * @brief Load user from JSON object
 */
void User::fromJson(const QJsonObject& json) {
    m_userId = json["userId"].toString();
    m_firstName = json["firstName"].toString();
    m_lastName = json["lastName"].toString();
    m_email = json["email"].toString();
    m_phoneNumber = json["phoneNumber"].toString();
    m_address = json["address"].toString();
    m_userType = stringToUserType(json["userType"].toString());    m_status = stringToStatus(json["status"].toString());
    m_registrationDate = QDateTime::fromString(json["registrationDate"].toString(), Qt::ISODate);
    m_lastActivity = QDateTime::fromString(json["lastActivity"].toString(), Qt::ISODate);    m_maxBorrowLimit = json["maxBorrowLimit"].toInt();
    m_notes = json["notes"].toString();
    m_year = json["year"].toInt(-1); // Default to -1 if not specified
    
    // Load current loans
    m_currentLoans.clear();
    QJsonArray currentLoansArray = json["currentLoans"].toArray();
    for (const QJsonValue& value : currentLoansArray) {
        auto loan = std::make_unique<Loan>("", "", "", "", QDateTime(), QDateTime());
        loan->fromJson(value.toObject());
        m_currentLoans.push_back(std::move(loan));
    }
    
    // Load loan history
    m_loanHistory.clear();
    QJsonArray loanHistoryArray = json["loanHistory"].toArray();
    for (const QJsonValue& value : loanHistoryArray) {
        auto loan = std::make_unique<Loan>("", "", "", "", QDateTime(), QDateTime());
        loan->fromJson(value.toObject());
        m_loanHistory.push_back(std::move(loan));
    }
}

/**
 * @brief Get formatted user information
 */
QString User::getFormattedInfo() const {
    QString info;
    info += QString("Name: %1\n").arg(getFullName());    info += QString("Email: %1\n").arg(m_email);
    info += QString("User Type: %1\n").arg(getUserTypeString());
    info += QString("Status: %1\n").arg(getStatusString());
    info += QString("Current Loans: %1/%2\n").arg(getCurrentLoanCount()).arg(m_maxBorrowLimit);
    
    if (hasOverdueItems()) {
        info += "⚠️ Has overdue items\n";
    }
    
    return info;
}

/**
 * @brief Get user type as string
 */
QString User::getUserTypeString() const {
    return userTypeToString(m_userType);
}

/**
 * @brief Get status as string
 */
QString User::getStatusString() const {
    return statusToString(m_status);
}

/**
 * @brief Convert UserType enum to string
 */
QString User::userTypeToString(UserType type) {
    switch (type) {
        case UserType::Student: return "Student";
        case UserType::Teacher: return "Teacher";
        case UserType::Staff: return "Staff";
        case UserType::Administrator: return "Administrator";
        case UserType::Guest: return "Guest";
        default: return "Unknown";
    }
}

/**
 * @brief Convert string to UserType enum
 */
User::UserType User::stringToUserType(const QString& typeStr) {
    if (typeStr == "Student") return UserType::Student;
    if (typeStr == "Teacher") return UserType::Teacher;
    if (typeStr == "Staff") return UserType::Staff;
    if (typeStr == "Administrator") return UserType::Administrator;
    if (typeStr == "Guest") return UserType::Guest;
    return UserType::Student; // Default fallback
}

/**
 * @brief Convert Status enum to string
 */
QString User::statusToString(Status status) {
    switch (status) {
        case Status::Active: return "Active";
        case Status::Inactive: return "Inactive";
        case Status::Suspended: return "Suspended";
        case Status::Expired: return "Expired";
        default: return "Unknown";
    }
}

/**
 * @brief Convert string to Status enum
 */
User::Status User::stringToStatus(const QString& statusStr) {
    if (statusStr == "Active") return Status::Active;
    if (statusStr == "Inactive") return Status::Inactive;
    if (statusStr == "Suspended") return Status::Suspended;
    if (statusStr == "Expired") return Status::Expired;
    return Status::Active; // Default fallback
}

/**
 * @brief Validate email format
 */
bool User::isValidEmail(const QString& email) {
    if (email.isEmpty()) return false;
    
    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    return emailRegex.match(email).hasMatch();
}

/**
 * @brief Equality comparison operator
 */
bool User::operator==(const User& other) const {
    return m_userId == other.m_userId;
}

/**
 * @brief Less than comparison operator (sort by last name, then first name)
 */
bool User::operator<(const User& other) const {
    if (m_lastName != other.m_lastName) {
        return m_lastName < other.m_lastName;
    }
    return m_firstName < other.m_firstName;
}

/**
 * @brief Validate user data
 */
void User::validateUserData() const {
    if (m_userId.isEmpty()) {
        throw UserException("User ID cannot be empty");
    }
    if (m_firstName.isEmpty()) {
        throw UserException("First name cannot be empty");
    }
    if (m_lastName.isEmpty()) {
        throw UserException("Last name cannot be empty");
    }
    if (!isValidEmail(m_email)) {
        throw UserException("Invalid email format");
    }
}

/**
 * @brief Set default borrow limit based on user type
 */
void User::setDefaultBorrowLimit() {
    switch (m_userType) {        case UserType::Student:
            m_maxBorrowLimit = 5;
            break;
        case UserType::Teacher:
            m_maxBorrowLimit = 20;
            break;
        case UserType::Staff:
            m_maxBorrowLimit = 10;
            break;
        case UserType::Administrator:
            m_maxBorrowLimit = 15;
            break;
        case UserType::Guest:
            m_maxBorrowLimit = 2;
            break;
    }
}

/**
 * @brief Clone a loan for deep copying
 */
std::unique_ptr<Loan> User::cloneLoan(const Loan& loan) const {
    auto clonedLoan = std::make_unique<Loan>(
        loan.getLoanId(), loan.getUserId(), loan.getResourceId(),
        loan.getResourceTitle(), loan.getBorrowDate(), loan.getDueDate(),
        loan.getMaxRenewals()
    );
    
    clonedLoan->setReturnDate(loan.getReturnDate());
    clonedLoan->setStatus(loan.getStatus());
    clonedLoan->setFineAmount(loan.getFineAmount());
    clonedLoan->setNotes(loan.getNotes());
    
    return clonedLoan;
}
