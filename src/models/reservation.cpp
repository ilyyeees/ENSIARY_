#include "reservation.h"
#include <QUuid>

/**
 * @brief Constructor for Reservation class
 */
Reservation::Reservation(const QString& userId, const QString& resourceId, 
                        const QString& resourceTitle, int expirationDays)
    : m_userId(userId), m_resourceId(resourceId), m_resourceTitle(resourceTitle),
      m_status(Status::Active), m_reservationDate(QDateTime::currentDateTime()) {
    
    m_reservationId = generateReservationId();
    m_expirationDate = m_reservationDate.addDays(expirationDays);
}

/**
 * @brief Set reservation status
 */
void Reservation::setStatus(Status status) {
    m_status = status;
}

/**
 * @brief Set notes
 */
void Reservation::setNotes(const QString& notes) {
    m_notes = notes;
}

/**
 * @brief Set expiration date
 */
void Reservation::setExpirationDate(const QDateTime& expirationDate) {
    m_expirationDate = expirationDate;
}

/**
 * @brief Check if reservation is expired
 */
bool Reservation::isExpired() const {
    return QDateTime::currentDateTime() > m_expirationDate && m_status == Status::Active;
}

/**
 * @brief Check if reservation can be fulfilled
 */
bool Reservation::canBeFulfilled() const {
    return m_status == Status::Active && !isExpired();
}

/**
 * @brief Fulfill the reservation (when resource becomes available)
 */
void Reservation::fulfillReservation() {
    if (!canBeFulfilled()) {
        throw ReservationException("Cannot fulfill reservation: either expired or not active");
    }
    m_status = Status::Fulfilled;
}

/**
 * @brief Cancel the reservation
 */
void Reservation::cancelReservation() {
    if (m_status == Status::Fulfilled) {
        throw ReservationException("Cannot cancel fulfilled reservation");
    }
    m_status = Status::Cancelled;
}

/**
 * @brief Extend reservation by additional days
 */
void Reservation::extendReservation(int additionalDays) {
    if (m_status != Status::Active) {
        throw ReservationException("Can only extend active reservations");
    }
    m_expirationDate = m_expirationDate.addDays(additionalDays);
}

/**
 * @brief Get days until expiration
 */
int Reservation::getDaysUntilExpiration() const {
    QDateTime now = QDateTime::currentDateTime();
    return now.daysTo(m_expirationDate);
}

/**
 * @brief Get formatted reservation information
 */
QString Reservation::getFormattedInfo() const {
    QString info;
    info += QString("Reservation ID: %1\n").arg(m_reservationId);
    info += QString("Resource: %1\n").arg(m_resourceTitle);
    info += QString("Reserved Date: %1\n").arg(m_reservationDate.toString("yyyy-MM-dd hh:mm"));
    info += QString("Expires: %1\n").arg(m_expirationDate.toString("yyyy-MM-dd hh:mm"));
    info += QString("Status: %1\n").arg(statusToString(m_status));
    
    if (m_status == Status::Active) {
        int daysLeft = getDaysUntilExpiration();
        if (daysLeft >= 0) {
            info += QString("Days until expiration: %1\n").arg(daysLeft);
        } else {
            info += "⚠️ EXPIRED\n";
        }
    }
    
    if (!m_notes.isEmpty()) {
        info += QString("Notes: %1\n").arg(m_notes);
    }
    
    return info;
}

/**
 * @brief Convert reservation to JSON object
 */
QJsonObject Reservation::toJson() const {
    QJsonObject json;
    json["reservationId"] = m_reservationId;
    json["userId"] = m_userId;
    json["resourceId"] = m_resourceId;
    json["resourceTitle"] = m_resourceTitle;
    json["reservationDate"] = m_reservationDate.toString(Qt::ISODate);
    json["expirationDate"] = m_expirationDate.toString(Qt::ISODate);
    json["status"] = statusToString(m_status);
    json["notes"] = m_notes;
    
    return json;
}

/**
 * @brief Load reservation from JSON object
 */
void Reservation::fromJson(const QJsonObject& json) {
    m_reservationId = json["reservationId"].toString();
    m_userId = json["userId"].toString();
    m_resourceId = json["resourceId"].toString();
    m_resourceTitle = json["resourceTitle"].toString();
    m_reservationDate = QDateTime::fromString(json["reservationDate"].toString(), Qt::ISODate);
    m_expirationDate = QDateTime::fromString(json["expirationDate"].toString(), Qt::ISODate);
    m_status = stringToStatus(json["status"].toString());
    m_notes = json["notes"].toString();
}

/**
 * @brief Generate unique reservation ID
 */
QString Reservation::generateReservationId() {
    return "RES_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
}

/**
 * @brief Convert Status enum to string
 */
QString Reservation::statusToString(Status status) {
    switch (status) {
        case Status::Active: return "Active";
        case Status::Fulfilled: return "Fulfilled";
        case Status::Expired: return "Expired";
        case Status::Cancelled: return "Cancelled";
        default: return "Unknown";
    }
}

/**
 * @brief Convert string to Status enum
 */
Reservation::Status Reservation::stringToStatus(const QString& statusStr) {
    if (statusStr == "Active") return Status::Active;
    if (statusStr == "Fulfilled") return Status::Fulfilled;
    if (statusStr == "Expired") return Status::Expired;
    if (statusStr == "Cancelled") return Status::Cancelled;
    return Status::Active; // Default fallback
}

/**
 * @brief Equality comparison operator
 */
bool Reservation::operator==(const Reservation& other) const {
    return m_reservationId == other.m_reservationId;
}

/**
 * @brief Less than comparison operator (sort by reservation date)
 */
bool Reservation::operator<(const Reservation& other) const {
    return m_reservationDate < other.m_reservationDate;
}
