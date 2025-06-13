#ifndef RESERVATION_H
#define RESERVATION_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QUuid>

/**
 * @brief Represents a reservation in the library system
 * 
 * This class manages resource reservations, tracking which user has
 * reserved which resource and when the reservation expires.
 */
class Reservation {
public:
    enum class Status {
        Active,     // Reservation is active and waiting
        Fulfilled,  // Resource has been borrowed by the user
        Expired,    // Reservation has expired
        Cancelled   // Reservation was cancelled
    };

private:
    QString m_reservationId;
    QString m_userId;
    QString m_resourceId;
    QString m_resourceTitle;
    QDateTime m_reservationDate;
    QDateTime m_expirationDate;
    Status m_status;
    QString m_notes;

public:
    // Constructors
    Reservation(const QString& userId, const QString& resourceId, 
                const QString& resourceTitle, int expirationDays = 7);
    
    // Getters
    QString getReservationId() const { return m_reservationId; }
    QString getId() const { return m_reservationId; } // Alias for consistency
    QString getUserId() const { return m_userId; }
    QString getResourceId() const { return m_resourceId; }
    QString getResourceTitle() const { return m_resourceTitle; }
    QDateTime getReservationDate() const { return m_reservationDate; }
    QDateTime getExpirationDate() const { return m_expirationDate; }
    Status getStatus() const { return m_status; }
    QString getNotes() const { return m_notes; }
    
    // Setters
    void setStatus(Status status);
    void setNotes(const QString& notes);
    void setExpirationDate(const QDateTime& expirationDate);
    
    // Status checks
    bool isActive() const { return m_status == Status::Active; }
    bool isExpired() const;
    bool canBeFulfilled() const;
    
    // Operations
    void fulfillReservation();
    void cancelReservation();
    void extendReservation(int additionalDays);
    
    // Utility functions
    int getDaysUntilExpiration() const;
    QString getFormattedInfo() const;
    
    // JSON serialization
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);
    
    // Static utility functions
    static QString generateReservationId();
    static QString statusToString(Status status);
    static Status stringToStatus(const QString& statusStr);
    
    // Comparison operators
    bool operator==(const Reservation& other) const;
    bool operator<(const Reservation& other) const; // Sort by reservation date
};

// Custom exception class for Reservation-related errors
class ReservationException : public std::exception {
private:
    QString m_message;

public:
    explicit ReservationException(const QString& message) : m_message(message) {}
    const char* what() const noexcept override {
        return m_message.toStdString().c_str();
    }
    QString getMessage() const { return m_message; }
};

#endif // RESERVATION_H
