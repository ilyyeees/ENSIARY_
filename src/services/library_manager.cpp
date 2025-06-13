#include "library_manager.h"
#include "../models/resource.h"
#include "../models/book.h"
#include "../models/article.h"
#include "../models/user.h"
#include "../models/loan.h"
#include "../models/reservation.h"
#include <algorithm>
#include <QUuid>
#include <QDebug>

/**
 * @brief Constructor for LibraryManager
 */
LibraryManager::LibraryManager(QObject* parent)
    : QObject(parent), m_libraryName("ENSIARY Library Management System"),
      m_operatingHours("Monday-Friday: 8:00 AM - 8:00 PM, Saturday-Sunday: 10:00 AM - 6:00 PM"),
      m_defaultLoanPeriodDays(14) {
    
    // Events list is empty by default - users can add their own events
}

/**
 * @brief Add a resource to the library
 */
void LibraryManager::addResource(std::unique_ptr<Resource> resource) {
    if (!resource) {
        throw LibraryManagerException("Cannot add null resource");
    }
    
    validateResourceData(*resource);
    
    // Check for duplicate resource ID using linear search
    if (findResourceById(resource->getId()) != nullptr) {
        throw LibraryManagerException("Resource with ID " + resource->getId() + " already exists");
    }
    
    QString resourceId = resource->getId();
    m_resources.push_back(std::move(resource));
    
    emit resourceAdded(resourceId);
}

/**
 * @brief Remove a resource from the library
 */
bool LibraryManager::removeResource(const QString& resourceId) {
    auto it = findResourceIterator(resourceId);
    if (it == m_resources.end()) {
        return false;
    }
    
    // Check if resource is currently borrowed
    Resource* resource = it->get();
    if (resource->isBorrowed()) {
        throw LibraryManagerException("Cannot remove resource that is currently borrowed");
    }
    
    m_resources.erase(it);
    emit resourceRemoved(resourceId);
    return true;
}

/**
 * @brief Find resource by ID using linear search
 */
Resource* LibraryManager::findResourceById(const QString& resourceId) {
    auto it = std::find_if(m_resources.begin(), m_resources.end(),
                          [&resourceId](const std::unique_ptr<Resource>& resource) {
                              return resource->getId() == resourceId;
                          });
    
    return (it != m_resources.end()) ? it->get() : nullptr;
}

/**
 * @brief Find resource by ID using linear search (const version)
 */
const Resource* LibraryManager::findResourceById(const QString& resourceId) const {
    auto it = std::find_if(m_resources.begin(), m_resources.end(),
                          [&resourceId](const std::unique_ptr<Resource>& resource) {
                              return resource->getId() == resourceId;
                          });
    
    return (it != m_resources.end()) ? it->get() : nullptr;
}

/**
 * @brief Get all resources
 */
std::vector<Resource*> LibraryManager::getAllResources() {
    std::vector<Resource*> resources;
    for (const auto& resource : m_resources) {
        resources.push_back(resource.get());
    }
    return resources;
}

/**
 * @brief Get all resources (const version)
 */
std::vector<const Resource*> LibraryManager::getAllResources() const {
    std::vector<const Resource*> resources;
    for (const auto& resource : m_resources) {
        resources.push_back(resource.get());
    }
    return resources;
}

/**
 * @brief Search resources by query using linear search
 */
std::vector<Resource*> LibraryManager::searchResources(const QString& query) {
    std::vector<Resource*> results;
    
    for (const auto& resource : m_resources) {
        if (matchesSearchQuery(*resource, query)) {
            results.push_back(resource.get());
        }
    }
    
    return results;
}

/**
 * @brief Filter resources by category
 */
std::vector<Resource*> LibraryManager::filterResourcesByCategory(Resource::Category category) {
    std::vector<Resource*> results;
    
    for (const auto& resource : m_resources) {
        if (resource->getCategory() == category) {
            results.push_back(resource.get());
        }
    }
    
    return results;
}

/**
 * @brief Filter resources by status
 */
std::vector<Resource*> LibraryManager::filterResourcesByStatus(Resource::Status status) {
    std::vector<Resource*> results;
    
    for (const auto& resource : m_resources) {
        if (resource->getStatus() == status) {
            results.push_back(resource.get());
        }
    }
    
    return results;
}

/**
 * @brief Get available resources
 */
std::vector<Resource*> LibraryManager::getAvailableResources() {
    return filterResourcesByStatus(Resource::Status::Available);
}

/**
 * @brief Add a user to the system
 */
void LibraryManager::addUser(std::unique_ptr<User> user) {
    if (!user) {
        throw LibraryManagerException("Cannot add null user");
    }
    
    validateUserData(*user);
    
    // Check for duplicate user ID using linear search
    if (findUserById(user->getUserId()) != nullptr) {
        throw LibraryManagerException("User with ID " + user->getUserId() + " already exists");
    }
    
    // Check for duplicate email using linear search
    if (findUserByEmail(user->getEmail()) != nullptr) {
        throw LibraryManagerException("User with email " + user->getEmail() + " already exists");
    }
    
    QString userId = user->getUserId();
    m_users.push_back(std::move(user));
    
    emit userAdded(userId);
}

/**
 * @brief Remove a user from the system
 */
bool LibraryManager::removeUser(const QString& userId) {
    auto it = findUserIterator(userId);
    if (it == m_users.end()) {
        return false;
    }
    
    // Check if user has active loans
    User* user = it->get();
    if (user->getCurrentLoanCount() > 0) {
        throw LibraryManagerException("Cannot remove user with active loans");
    }
    
    m_users.erase(it);
    emit userRemoved(userId);
    return true;
}

/**
 * @brief Find user by ID using linear search
 */
User* LibraryManager::findUserById(const QString& userId) {
    auto it = std::find_if(m_users.begin(), m_users.end(),
                          [&userId](const std::unique_ptr<User>& user) {
                              return user->getUserId() == userId;
                          });
    
    return (it != m_users.end()) ? it->get() : nullptr;
}

/**
 * @brief Find user by ID using linear search (const version)
 */
const User* LibraryManager::findUserById(const QString& userId) const {
    auto it = std::find_if(m_users.begin(), m_users.end(),
                          [&userId](const std::unique_ptr<User>& user) {
                              return user->getUserId() == userId;
                          });
    
    return (it != m_users.end()) ? it->get() : nullptr;
}

/**
 * @brief Find user by email using linear search
 */
User* LibraryManager::findUserByEmail(const QString& email) {
    auto it = std::find_if(m_users.begin(), m_users.end(),
                          [&email](const std::unique_ptr<User>& user) {
                              return user->getEmail() == email;
                          });
    
    return (it != m_users.end()) ? it->get() : nullptr;
}

/**
 * @brief Get all users
 */
std::vector<User*> LibraryManager::getAllUsers() {
    std::vector<User*> users;
    for (const auto& user : m_users) {
        users.push_back(user.get());
    }
    return users;
}

/**
 * @brief Get all users (const version)
 */
std::vector<const User*> LibraryManager::getAllUsers() const {
    std::vector<const User*> users;
    for (const auto& user : m_users) {
        users.push_back(user.get());
    }
    return users;
}

/**
 * @brief Search users by query using linear search
 */
std::vector<User*> LibraryManager::searchUsers(const QString& query) {
    std::vector<User*> results;
    
    for (const auto& user : m_users) {
        if (matchesSearchQuery(*user, query)) {
            results.push_back(user.get());
        }
    }
    
    return results;
}

/**
 * @brief Get users with overdue items
 */
std::vector<User*> LibraryManager::getUsersWithOverdueItems() {
    std::vector<User*> results;
    
    for (const auto& user : m_users) {
        if (user->hasOverdueItems()) {        results.push_back(user.get());
        }
    }
    
    return results;
}

/**
 * @brief Borrow a resource
 */
QString LibraryManager::borrowResource(const QString& userId, const QString& resourceId) {
    User* user = findUserById(userId);
    if (!user) {
        throw LibraryManagerException("User not found: " + userId);
    }
    
    Resource* resource = findResourceById(resourceId);
    if (!resource) {
        throw LibraryManagerException("Resource not found: " + resourceId);
    }
    
    if (!user->canBorrow()) {
        throw LibraryManagerException("User cannot borrow items at this time");
    }
    
    if (!resource->isAvailable()) {
        throw LibraryManagerException("Resource is not available for borrowing");
    }
    
    // Create loan
    QString loanId = generateLoanId();
    QDateTime borrowDate = QDateTime::currentDateTime();
    QDateTime dueDate = calculateDueDate();
    
    auto loan = std::make_unique<Loan>(loanId, userId, resourceId, resource->getTitle(),
                                      borrowDate, dueDate);
    
    // Update resource status
    resource->setStatus(Resource::Status::Borrowed);
    
    // Add loan to user's current loans
    user->addCurrentLoan(std::unique_ptr<Loan>(new Loan(*loan)));
    
    // Add loan to active loans
    m_activeLoans.push_back(std::move(loan));
    
    emit resourceBorrowed(loanId, userId, resourceId);
    return loanId;
}

/**
 * @brief Return a resource
 */
bool LibraryManager::returnResource(const QString& loanId) {
    auto it = findLoanIterator(loanId);
    if (it == m_activeLoans.end()) {
        return false;
    }
    
    Loan* loan = it->get();
    QString resourceId = loan->getResourceId();
    
    // Update resource status
    Resource* resource = findResourceById(resourceId);
    if (resource) {
        resource->setStatus(Resource::Status::Available);
    }
    
    // Process loan return
    processLoanReturn(*loan);
    
    // Move loan to history
    moveLoanToHistory(loanId);
    
    emit resourceReturned(loanId, loan->getUserId(), resourceId);
    
    // Check if anyone has this resource reserved and notify them
    notifyWhenResourceAvailable(resourceId);
    
    return true;
}

/**
 * @brief Renew a loan
 */
QDateTime LibraryManager::renewLoan(const QString& loanId, int additionalDays) {
    auto it = findLoanIterator(loanId);
    if (it == m_activeLoans.end()) {
        return QDateTime();
    }
    
    Loan* loan = it->get();
    
    if (!loan->canBeRenewed()) {
        throw LibraryManagerException("Loan cannot be renewed");
    }
    
    if (loan->renewLoan(additionalDays)) {
        emit loanRenewed(loanId, loan->getDueDate());
        return loan->getDueDate();
    }
    
    return QDateTime();
}

/**
 * @brief Get active loans
 */
std::vector<Loan*> LibraryManager::getActiveLoans() {
    std::vector<Loan*> loans;
    for (const auto& loan : m_activeLoans) {
        loans.push_back(loan.get());
    }
    return loans;
}

/**
 * @brief Get active loans (const version)
 */
std::vector<const Loan*> LibraryManager::getActiveLoans() const {
    std::vector<const Loan*> loans;
    for (const auto& loan : m_activeLoans) {
        loans.push_back(loan.get());
    }
    return loans;
}

/**
 * @brief Get overdue loans
 */
std::vector<Loan*> LibraryManager::getOverdueLoans() {
    std::vector<Loan*> overdueLoans;
    
    for (const auto& loan : m_activeLoans) {
        if (loan->isOverdue()) {
            overdueLoans.push_back(loan.get());
        }
    }
    
    return overdueLoans;
}

/**
 * @brief Get loan history
 */
std::vector<Loan*> LibraryManager::getLoanHistory() {
    std::vector<Loan*> loans;
    for (const auto& loan : m_loanHistory) {
        loans.push_back(loan.get());
    }
    return loans;
}

/**
 * @brief Get loan history (const version)
 */
std::vector<const Loan*> LibraryManager::getLoanHistory() const {
    std::vector<const Loan*> loans;
    for (const auto& loan : m_loanHistory) {
        loans.push_back(loan.get());
    }
    return loans;
}

/**
 * @brief Get reservation history
 */
std::vector<Reservation*> LibraryManager::getReservationHistory() {
    std::vector<Reservation*> reservations;
    for (const auto& reservation : m_reservationHistory) {
        reservations.push_back(reservation.get());
    }
    return reservations;
}

/**
 * @brief Get reservation history (const version)
 */
std::vector<const Reservation*> LibraryManager::getReservationHistory() const {
    std::vector<const Reservation*> reservations;
    for (const auto& reservation : m_reservationHistory) {
        reservations.push_back(reservation.get());
    }
    return reservations;
}

/**
 * @brief Get user loans using linear search
 */
std::vector<Loan*> LibraryManager::getUserLoans(const QString& userId) {
    std::vector<Loan*> userLoans;
    
    for (const auto& loan : m_activeLoans) {
        if (loan->getUserId() == userId) {
            userLoans.push_back(loan.get());
        }
    }
    
    return userLoans;
}

/**
 * @brief Get resource loans using linear search
 */
std::vector<Loan*> LibraryManager::getResourceLoans(const QString& resourceId) {
    std::vector<Loan*> resourceLoans;
    
    // Check active loans
    for (const auto& loan : m_activeLoans) {
        if (loan->getResourceId() == resourceId) {
            resourceLoans.push_back(loan.get());
        }
    }
    
    // Check loan history
    for (const auto& loan : m_loanHistory) {
        if (loan->getResourceId() == resourceId) {
            resourceLoans.push_back(loan.get());
        }
    }
    
    return resourceLoans;
}

/**
 * @brief Reserve a resource for a user
 */
QString LibraryManager::reserveResource(const QString& userId, const QString& resourceId) {
    // Validate user exists
    User* user = findUserById(userId);
    if (!user) {
        throw LibraryManagerException("User not found: " + userId);
    }
    
    // Validate resource exists
    Resource* resource = findResourceById(resourceId);
    if (!resource) {
        throw LibraryManagerException("Resource not found: " + resourceId);
    }
    
    // Check if user can borrow (active status, etc.)
    if (!user->canBorrow()) {
        throw LibraryManagerException("User cannot make reservations");
    }
    
    // Check if resource is already reserved by this user
    for (const auto& reservation : m_activeReservations) {
        if (reservation->getUserId() == userId && reservation->getResourceId() == resourceId && 
            reservation->isActive()) {
            throw LibraryManagerException("User already has an active reservation for this resource");
        }
    }
    
    // If resource is available, no need to reserve - they can borrow directly
    if (resource->isAvailable()) {
        throw LibraryManagerException("Resource is available for immediate borrowing - no reservation needed");
    }
    
    // Create the reservation
    auto reservation = std::make_unique<Reservation>(userId, resourceId, resource->getTitle());
    QString reservationId = reservation->getReservationId();
    
    m_activeReservations.push_back(std::move(reservation));
    
    // Emit signal for notification
    emit resourceReserved(reservationId, userId, resourceId);
    
    return reservationId;
}

/**
 * @brief Cancel reservation by reservation ID
 */
bool LibraryManager::cancelReservation(const QString& reservationId) {
    auto it = std::find_if(m_activeReservations.begin(), m_activeReservations.end(),
                          [&reservationId](const std::unique_ptr<Reservation>& reservation) {
                              return reservation->getReservationId() == reservationId;
                          });
    
    if (it != m_activeReservations.end()) {
        QString userId = (*it)->getUserId();
        QString resourceId = (*it)->getResourceId();
        
        (*it)->cancelReservation();
        
        // Move to history
        m_reservationHistory.push_back(std::move(*it));
        m_activeReservations.erase(it);
        
        emit reservationCancelled(reservationId, userId, resourceId);
        return true;
    }
    
    return false;
}

/**
 * @brief Cancel reservation by user and resource
 */
bool LibraryManager::cancelUserReservation(const QString& userId, const QString& resourceId) {
    auto it = std::find_if(m_activeReservations.begin(), m_activeReservations.end(),
                          [&userId, &resourceId](const std::unique_ptr<Reservation>& reservation) {
                              return reservation->getUserId() == userId && 
                                     reservation->getResourceId() == resourceId &&
                                     reservation->isActive();
                          });
    
    if (it != m_activeReservations.end()) {
        QString reservationId = (*it)->getReservationId();
        return cancelReservation(reservationId);
    }
    
    return false;
}

/**
 * @brief Get all active reservations
 */
std::vector<Reservation*> LibraryManager::getActiveReservations() {
    std::vector<Reservation*> reservations;
    for (const auto& reservation : m_activeReservations) {
        if (reservation->isActive()) {
            reservations.push_back(reservation.get());
        }
    }
    return reservations;
}

/**
 * @brief Get all active reservations (const version)
 */
std::vector<const Reservation*> LibraryManager::getActiveReservations() const {
    std::vector<const Reservation*> reservations;
    for (const auto& reservation : m_activeReservations) {
        if (reservation->isActive()) {
            reservations.push_back(reservation.get());
        }
    }
    return reservations;
}

/**
 * @brief Get reservations for a specific user
 */
std::vector<Reservation*> LibraryManager::getUserReservations(const QString& userId) {
    std::vector<Reservation*> reservations;
    for (const auto& reservation : m_activeReservations) {
        if (reservation->getUserId() == userId) {
            reservations.push_back(reservation.get());
        }
    }
    return reservations;
}

/**
 * @brief Get reservations for a specific resource
 */
std::vector<Reservation*> LibraryManager::getResourceReservations(const QString& resourceId) {
    std::vector<Reservation*> reservations;
    for (const auto& reservation : m_activeReservations) {
        if (reservation->getResourceId() == resourceId && reservation->isActive()) {
            reservations.push_back(reservation.get());
        }
    }
    // Sort by reservation date (earliest first)
    std::sort(reservations.begin(), reservations.end(),
              [](const Reservation* a, const Reservation* b) {
                  return a->getReservationDate() < b->getReservationDate();
              });
    return reservations;
}

/**
 * @brief Get expired reservations
 */
std::vector<Reservation*> LibraryManager::getExpiredReservations() {
    std::vector<Reservation*> reservations;
    for (const auto& reservation : m_activeReservations) {
        if (reservation->isExpired()) {
            reservations.push_back(reservation.get());
        }
    }
    return reservations;
}

/**
 * @brief Find reservation by ID
 */
Reservation* LibraryManager::findReservationById(const QString& reservationId) {
    auto it = std::find_if(m_activeReservations.begin(), m_activeReservations.end(),
                          [&reservationId](const std::unique_ptr<Reservation>& reservation) {
                              return reservation->getReservationId() == reservationId;
                          });
    
    return (it != m_activeReservations.end()) ? it->get() : nullptr;
}

/**
 * @brief Process expired reservations
 */
bool LibraryManager::processExpiredReservations() {
    bool hasExpired = false;
    auto it = m_activeReservations.begin();
    
    while (it != m_activeReservations.end()) {
        if ((*it)->isExpired()) {
            QString reservationId = (*it)->getReservationId();
            QString userId = (*it)->getUserId();
            QString resourceId = (*it)->getResourceId();
            
            (*it)->setStatus(Reservation::Status::Expired);
            
            // Move to history
            m_reservationHistory.push_back(std::move(*it));
            it = m_activeReservations.erase(it);
            
            emit reservationExpired(reservationId, userId, resourceId);
            hasExpired = true;
        } else {
            ++it;
        }
    }
    
    return hasExpired;
}

/**
 * @brief Notify users when a reserved resource becomes available
 */
void LibraryManager::notifyWhenResourceAvailable(const QString& resourceId) {
    // Get reservations for this resource (sorted by date)
    auto reservations = getResourceReservations(resourceId);
    
    if (!reservations.empty()) {
        // Notify the first user in the reservation queue
        Reservation* firstReservation = reservations[0];
        if (firstReservation->canBeFulfilled()) {
            emit reservedResourceAvailable(firstReservation->getReservationId(),
                                         firstReservation->getUserId(),
                                         resourceId);
        }
    }
}

/**
 * @brief Get completed loans (loan history)
 */
std::vector<Loan*> LibraryManager::getCompletedLoans() {
    std::vector<Loan*> loans;
    for (const auto& loan : m_loanHistory) {
        loans.push_back(loan.get());
    }
    return loans;
}

/**
 * @brief Get total resource count
 */
int LibraryManager::getTotalResourceCount() const {
    return static_cast<int>(m_resources.size());
}

/**
 * @brief Get available resource count
 */
int LibraryManager::getAvailableResourceCount() const {
    return static_cast<int>(std::count_if(m_resources.begin(), m_resources.end(),
                                         [](const std::unique_ptr<Resource>& resource) {
                                             return resource->isAvailable();
                                         }));
}

/**
 * @brief Get total user count
 */
int LibraryManager::getTotalUserCount() const {
    return static_cast<int>(m_users.size());
}

/**
 * @brief Get active user count
 */
int LibraryManager::getActiveUserCount() const {
    return static_cast<int>(std::count_if(m_users.begin(), m_users.end(),
                                         [](const std::unique_ptr<User>& user) {
                                             return user->getStatus() == User::Status::Active;
                                         }));
}

/**
 * @brief Get total active loans
 */
int LibraryManager::getTotalActiveLoans() const {
    return static_cast<int>(m_activeLoans.size());
}

/**
 * @brief Get total overdue loans
 */
int LibraryManager::getTotalOverdueLoans() const {
    return static_cast<int>(std::count_if(m_activeLoans.begin(), m_activeLoans.end(),
                                         [](const std::unique_ptr<Loan>& loan) {
                                             return loan->isOverdue();
                                         }));
}

/**
 * @brief Get most borrowed resources (placeholder implementation)
 */
std::vector<Resource*> LibraryManager::getMostBorrowedResources(int count) {
    // This would require tracking borrow counts - simplified implementation
    std::vector<Resource*> resources = getAllResources();
    if (resources.size() > static_cast<size_t>(count)) {
        resources.resize(count);
    }
    return resources;
}

/**
 * @brief Get most active users (placeholder implementation)
 */
std::vector<User*> LibraryManager::getMostActiveUsers(int count) {
    // This would require tracking activity - simplified implementation
    std::vector<User*> users = getAllUsers();
    if (users.size() > static_cast<size_t>(count)) {
        users.resize(count);
    }
    return users;
}

/**
 * @brief Add upcoming event
 */
void LibraryManager::addUpcomingEvent(const QString& event) {
    if (!event.isEmpty() && std::find(m_upcomingEvents.begin(), m_upcomingEvents.end(), event) == m_upcomingEvents.end()) {
        m_upcomingEvents.push_back(event);
    }
}

/**
 * @brief Remove upcoming event
 */
void LibraryManager::removeUpcomingEvent(const QString& event) {
    m_upcomingEvents.erase(std::remove(m_upcomingEvents.begin(), m_upcomingEvents.end(), event),
                          m_upcomingEvents.end());
}

/**
 * @brief Check if resource ID is valid
 */
bool LibraryManager::isValidResourceId(const QString& resourceId) const {
    return findResourceById(resourceId) != nullptr;
}

/**
 * @brief Check if user ID is valid
 */
bool LibraryManager::isValidUserId(const QString& userId) const {
    return findUserById(userId) != nullptr;
}

/**
 * @brief Check if loan ID is valid
 */
bool LibraryManager::isValidLoanId(const QString& loanId) const {
    return std::find_if(m_activeLoans.begin(), m_activeLoans.end(),
                       [&loanId](const std::unique_ptr<Loan>& loan) {
                           return loan->getLoanId() == loanId;
                       }) != m_activeLoans.end();
}

/**
 * @brief Check if user can borrow
 */
bool LibraryManager::canUserBorrow(const QString& userId) const {
    const User* user = findUserById(userId);
    return user ? user->canBorrow() : false;
}

/**
 * @brief Check if resource is available
 */
bool LibraryManager::isResourceAvailable(const QString& resourceId) const {
    const Resource* resource = findResourceById(resourceId);
    return resource ? resource->isAvailable() : false;
}

/**
 * @brief Generate resource ID
 */
QString LibraryManager::generateResourceId(const QString& prefix) {
    return prefix + "_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
}

/**
 * @brief Generate user ID
 */
QString LibraryManager::generateUserId(const QString& prefix) {
    return prefix + "_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
}

/**
 * @brief Generate loan ID
 */
QString LibraryManager::generateLoanId() {
    return Loan::generateLoanId();
}

/**
 * @brief Perform daily maintenance tasks
 */
void LibraryManager::performDailyMaintenance() {
    updateResourceAvailability();
    
    // Process expired reservations
    processExpiredReservations();
    
    // Emit overdue notifications
    for (const auto& loan : m_activeLoans) {
        if (loan->isOverdue()) {
            emit itemOverdue(loan->getLoanId(), loan->getUserId(), loan->getResourceId());
        }
    }
}

/**
 * @brief Update resource availability
 */
void LibraryManager::updateResourceAvailability() {
    // This method could be used to sync resource status with actual availability
    // For now, it's a placeholder for future implementation
}

// Private helper methods

/**
 * @brief Find resource iterator using linear search
 */
std::vector<std::unique_ptr<Resource>>::iterator LibraryManager::findResourceIterator(const QString& resourceId) {
    return std::find_if(m_resources.begin(), m_resources.end(),
                       [&resourceId](const std::unique_ptr<Resource>& resource) {
                           return resource->getId() == resourceId;
                       });
}

/**
 * @brief Find user iterator using linear search
 */
std::vector<std::unique_ptr<User>>::iterator LibraryManager::findUserIterator(const QString& userId) {
    return std::find_if(m_users.begin(), m_users.end(),
                       [&userId](const std::unique_ptr<User>& user) {
                           return user->getUserId() == userId;
                       });
}

/**
 * @brief Find loan iterator using linear search
 */
std::vector<std::unique_ptr<Loan>>::iterator LibraryManager::findLoanIterator(const QString& loanId) {
    return std::find_if(m_activeLoans.begin(), m_activeLoans.end(),
                       [&loanId](const std::unique_ptr<Loan>& loan) {
                           return loan->getLoanId() == loanId;
                       });
}

/**
 * @brief Check if resource matches search query
 */
bool LibraryManager::matchesSearchQuery(const Resource& resource, const QString& query) const {
    QString lowerQuery = query.toLower();
    return resource.getTitle().toLower().contains(lowerQuery) ||
           resource.getAuthor().toLower().contains(lowerQuery) ||
           resource.getDescription().toLower().contains(lowerQuery);
}

/**
 * @brief Check if user matches search query
 */
bool LibraryManager::matchesSearchQuery(const User& user, const QString& query) const {
    QString lowerQuery = query.toLower();
    return user.getFirstName().toLower().contains(lowerQuery) ||
           user.getLastName().toLower().contains(lowerQuery) ||
           user.getEmail().toLower().contains(lowerQuery) ||
           user.getUserId().toLower().contains(lowerQuery);
}

/**
 * @brief Calculate due date
 */
QDateTime LibraryManager::calculateDueDate(int loanPeriodDays) const {
    int days = (loanPeriodDays > 0) ? loanPeriodDays : m_defaultLoanPeriodDays;
    return QDateTime::currentDateTime().addDays(days);
}

/**
 * @brief Process loan return
 */
void LibraryManager::processLoanReturn(Loan& loan) {
    loan.returnItem();
    
    // Update user's loan collections
    User* user = findUserById(loan.getUserId());
    if (user) {
        user->moveLoanToHistory(loan.getLoanId());
        
        // Note: Fine system has been removed for school library
        // Overdue items are handled through other administrative means
    }
}

/**
 * @brief Move loan from active to history
 */
void LibraryManager::moveLoanToHistory(const QString& loanId) {
    auto it = findLoanIterator(loanId);
    if (it != m_activeLoans.end()) {
        m_loanHistory.push_back(std::move(*it));
        m_activeLoans.erase(it);
    }
}

/**
 * @brief Validate resource data
 */
void LibraryManager::validateResourceData(const Resource& resource) const {
    if (resource.getId().isEmpty()) {
        throw LibraryManagerException("Resource ID cannot be empty");
    }
    if (resource.getTitle().isEmpty()) {
        throw LibraryManagerException("Resource title cannot be empty");
    }
    if (resource.getAuthor().isEmpty()) {
        throw LibraryManagerException("Resource author cannot be empty");
    }
}

/**
 * @brief Validate user data
 */
void LibraryManager::validateUserData(const User& user) const {
    if (user.getUserId().isEmpty()) {
        throw LibraryManagerException("User ID cannot be empty");
    }
    if (user.getFirstName().isEmpty()) {
        throw LibraryManagerException("User first name cannot be empty");
    }
    if (user.getLastName().isEmpty()) {
        throw LibraryManagerException("User last name cannot be empty");
    }
    if (user.getEmail().isEmpty()) {
        throw LibraryManagerException("User email cannot be empty");
    }
}

/**
 * @brief Validate loan data
 */
void LibraryManager::validateLoanData(const Loan& loan) const {
    if (loan.getLoanId().isEmpty()) {
        throw LibraryManagerException("Loan ID cannot be empty");
    }
    if (loan.getUserId().isEmpty()) {
        throw LibraryManagerException("Loan user ID cannot be empty");
    }
    if (loan.getResourceId().isEmpty()) {
        throw LibraryManagerException("Loan resource ID cannot be empty");
    }
}

/**
 * @brief Add active loan (for persistence loading)
 */
void LibraryManager::addActiveLoan(std::unique_ptr<Loan> loan) {
    if (loan) {
        m_activeLoans.push_back(std::move(loan));
    }
}

/**
 * @brief Add loan to history (for persistence loading)
 */
void LibraryManager::addLoanHistory(std::unique_ptr<Loan> loan) {
    if (loan) {
        m_loanHistory.push_back(std::move(loan));
    }
}

/**
 * @brief Add active reservation (for persistence loading)
 */
void LibraryManager::addActiveReservation(std::unique_ptr<Reservation> reservation) {
    if (reservation) {
        m_activeReservations.push_back(std::move(reservation));
    }
}

/**
 * @brief Add reservation to history (for persistence loading)
 */
void LibraryManager::addReservationHistory(std::unique_ptr<Reservation> reservation) {
    if (reservation) {
        m_reservationHistory.push_back(std::move(reservation));
    }
}
