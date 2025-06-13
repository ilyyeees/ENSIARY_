#include "loan.h"
#include <QUuid>
#include <QJsonObject>

/**
 * @brief Constructor for Loan class
 */
Loan::Loan(const QString& loanId, const QString& userId, const QString& resourceId,
           const QString& resourceTitle, const QDateTime& borrowDate,
           const QDateTime& dueDate, int maxRenewals)
    : m_loanId(loanId), m_userId(userId), m_resourceId(resourceId),
      m_resourceTitle(resourceTitle), m_borrowDate(borrowDate), m_dueDate(dueDate),
      m_status(Status::Active), m_renewalCount(0), m_maxRenewals(maxRenewals),
      m_fineAmount(0.0) {
    
    validateLoanData();
    updateStatus();
}

/**
 * @brief Set due date with validation
 */
void Loan::setDueDate(const QDateTime& dueDate) {
    if (dueDate <= m_borrowDate) {
        throw LoanException("Due date must be after borrow date");
    }
    m_dueDate = dueDate;
    updateStatus();
}

/**
 * @brief Set return date
 */
void Loan::setReturnDate(const QDateTime& returnDate) {
    if (returnDate < m_borrowDate) {
        throw LoanException("Return date cannot be before borrow date");
    }
    m_returnDate = returnDate;
    updateStatus();
}

/**
 * @brief Set loan status
 */
void Loan::setStatus(Status status) {
    m_status = status;
}

/**
 * @brief Set fine amount
 */
void Loan::setFineAmount(double amount) {
    if (amount < 0.0) {
        throw LoanException("Fine amount cannot be negative");
    }
    m_fineAmount = amount;
}

/**
 * @brief Set notes
 */
void Loan::setNotes(const QString& notes) {
    m_notes = notes;
}

/**
 * @brief Renew the loan
 */
bool Loan::renewLoan(int daysToExtend) {
    if (!canBeRenewed()) {
        return false;
    }
    
    m_dueDate = m_dueDate.addDays(daysToExtend);
    m_renewalCount++;
    m_status = Status::Renewed;
    
    return true;
}

/**
 * @brief Return the item
 */
void Loan::returnItem() {
    m_returnDate = QDateTime::currentDateTime();
    m_status = Status::Returned;
    
    // Calculate fine if returned late
    if (isOverdue()) {
        calculateFine();
    }
}

/**
 * @brief Mark item as lost
 */
void Loan::markAsLost() {
    m_status = Status::Lost;
    // Typically, a substantial fine would be applied for lost items
    m_fineAmount += 50.0; // Example replacement cost
}

/**
 * @brief Calculate fine for overdue item
 */
void Loan::calculateFine(double dailyFineRate) {
    int daysOverdue = getDaysOverdue();
    if (daysOverdue > 0) {
        m_fineAmount = daysOverdue * dailyFineRate;
    }
}

/**
 * @brief Check if loan is overdue
 */
bool Loan::isOverdue() const {
    if (m_status == Status::Returned || m_status == Status::Lost) {
        return false; // No longer overdue once returned or lost
    }
    
    return QDateTime::currentDateTime() > m_dueDate;
}

/**
 * @brief Check if loan can be renewed
 */
bool Loan::canBeRenewed() const {
    return (m_status == Status::Active || m_status == Status::Renewed) &&
           m_renewalCount < m_maxRenewals &&
           !isOverdue();
}

/**
 * @brief Get days overdue
 */
int Loan::getDaysOverdue() const {
    if (!isOverdue()) {
        return 0;
    }
    
    return m_dueDate.daysTo(QDateTime::currentDateTime());
}

/**
 * @brief Get days until due
 */
int Loan::getDaysUntilDue() const {
    if (m_status == Status::Returned || m_status == Status::Lost) {
        return 0;
    }
    
    int daysUntil = QDateTime::currentDateTime().daysTo(m_dueDate);
    return qMax(0, daysUntil);
}

/**
 * @brief Convert loan to JSON object
 */
QJsonObject Loan::toJson() const {
    QJsonObject json;
    json["loanId"] = m_loanId;
    json["userId"] = m_userId;
    json["resourceId"] = m_resourceId;
    json["resourceTitle"] = m_resourceTitle;
    json["borrowDate"] = m_borrowDate.toString(Qt::ISODate);
    json["dueDate"] = m_dueDate.toString(Qt::ISODate);
    json["returnDate"] = m_returnDate.toString(Qt::ISODate);
    json["status"] = statusToString(m_status);
    json["renewalCount"] = m_renewalCount;
    json["maxRenewals"] = m_maxRenewals;
    json["fineAmount"] = m_fineAmount;
    json["notes"] = m_notes;
    
    return json;
}

/**
 * @brief Load loan from JSON object
 */
void Loan::fromJson(const QJsonObject& json) {
    m_loanId = json["loanId"].toString();
    m_userId = json["userId"].toString();
    m_resourceId = json["resourceId"].toString();
    m_resourceTitle = json["resourceTitle"].toString();
    m_borrowDate = QDateTime::fromString(json["borrowDate"].toString(), Qt::ISODate);
    m_dueDate = QDateTime::fromString(json["dueDate"].toString(), Qt::ISODate);
    m_returnDate = QDateTime::fromString(json["returnDate"].toString(), Qt::ISODate);
    m_status = stringToStatus(json["status"].toString());
    m_renewalCount = json["renewalCount"].toInt();
    m_maxRenewals = json["maxRenewals"].toInt();
    m_fineAmount = json["fineAmount"].toDouble();
    m_notes = json["notes"].toString();
}

/**
 * @brief Get formatted information for display
 */
QString Loan::getFormattedInfo() const {
    QString info = QString("%1 - %2")
                   .arg(m_resourceTitle)
                   .arg(getStatusString());
    
    if (m_status == Status::Active || m_status == Status::Renewed) {
        info += QString(" (Due: %1)").arg(m_dueDate.toString("yyyy-MM-dd"));
        
        if (isOverdue()) {
            info += QString(" - OVERDUE by %1 days").arg(getDaysOverdue());
        }
    } else if (m_status == Status::Returned) {
        info += QString(" (Returned: %1)").arg(m_returnDate.toString("yyyy-MM-dd"));
    }
    
    if (m_fineAmount > 0.0) {
        info += QString(" - Fine: $%.2f").arg(m_fineAmount);
    }
    
    return info;
}

/**
 * @brief Get status as string
 */
QString Loan::getStatusString() const {
    return statusToString(m_status);
}

/**
 * @brief Get loan duration as string
 */
QString Loan::getDurationString() const {
    if (m_status == Status::Returned) {
        int days = m_borrowDate.daysTo(m_returnDate);
        return QString("%1 days").arg(days);
    } else {
        int days = m_borrowDate.daysTo(QDateTime::currentDateTime());
        return QString("%1 days (ongoing)").arg(days);
    }
}

/**
 * @brief Convert Status enum to string
 */
QString Loan::statusToString(Status status) {
    switch (status) {
        case Status::Active: return "Active";
        case Status::Returned: return "Returned";
        case Status::Overdue: return "Overdue";
        case Status::Renewed: return "Renewed";
        case Status::Lost: return "Lost";
        default: return "Unknown";
    }
}

/**
 * @brief Convert string to Status enum
 */
Loan::Status Loan::stringToStatus(const QString& statusStr) {
    if (statusStr == "Active") return Status::Active;
    if (statusStr == "Returned") return Status::Returned;
    if (statusStr == "Overdue") return Status::Overdue;
    if (statusStr == "Renewed") return Status::Renewed;
    if (statusStr == "Lost") return Status::Lost;
    return Status::Active; // Default fallback
}

/**
 * @brief Generate a unique loan ID
 */
QString Loan::generateLoanId() {
    return "LOAN_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
}

/**
 * @brief Equality comparison operator
 */
bool Loan::operator==(const Loan& other) const {
    return m_loanId == other.m_loanId;
}

/**
 * @brief Less than comparison operator (sort by due date)
 */
bool Loan::operator<(const Loan& other) const {
    return m_dueDate < other.m_dueDate;
}

/**
 * @brief Validate loan data
 */
void Loan::validateLoanData() const {
    if (m_loanId.isEmpty()) {
        throw LoanException("Loan ID cannot be empty");
    }
    if (m_userId.isEmpty()) {
        throw LoanException("User ID cannot be empty");
    }
    if (m_resourceId.isEmpty()) {
        throw LoanException("Resource ID cannot be empty");
    }
    if (m_dueDate <= m_borrowDate) {
        throw LoanException("Due date must be after borrow date");
    }
    if (m_maxRenewals < 0) {
        throw LoanException("Maximum renewals cannot be negative");
    }
}

/**
 * @brief Update status based on current conditions
 */
void Loan::updateStatus() {
    if (m_status == Status::Returned || m_status == Status::Lost) {
        return; // These are final states
    }
    
    if (isOverdue()) {
        m_status = Status::Overdue;
    }
}
