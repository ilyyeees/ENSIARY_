#ifndef THESIS_H
#define THESIS_H

#include "resource.h"
#include <QString>
#include <QDateTime>

/**
 * @brief Represents a thesis resource in the library
 * 
 * This class extends Resource to handle thesis-specific information
 * such as supervisor, university, and degree level.
 */
class Thesis : public Resource {
public:
    enum class DegreeLevel {
        Bachelors,
        Masters,
        PhD,
        Postdoc
    };

private:
    QString m_supervisor;
    QString m_university;
    QString m_department;
    DegreeLevel m_degreeLevel;
    QString m_keywords;

public:    // Constructor
    Thesis(const QString& id, const QString& title, const QString& author, 
           int publicationYear = 2024);

    // Destructor
    ~Thesis() override = default;

    // Copy constructor and assignment operator
    Thesis(const Thesis& other);
    Thesis& operator=(const Thesis& other);

    // Move constructor and assignment operator
    Thesis(Thesis&& other) noexcept = default;
    Thesis& operator=(Thesis&& other) noexcept = default;

    // Thesis-specific getters
    QString getSupervisor() const { return m_supervisor; }
    QString getUniversity() const { return m_university; }
    QString getDepartment() const { return m_department; }
    DegreeLevel getDegreeLevel() const { return m_degreeLevel; }
    QString getKeywords() const { return m_keywords; }

    // Thesis-specific setters with validation
    void setSupervisor(const QString& supervisor);
    void setUniversity(const QString& university);
    void setDepartment(const QString& department);
    void setDegreeLevel(DegreeLevel level);
    void setKeywords(const QString& keywords);    // Override base class methods
    QString getDetails() const override;
    QString getResourceType() const override { return "Thesis"; }

    // JSON serialization
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& json) override;

    // Static utility functions
    static QString degreeLevelToString(DegreeLevel level);
    static DegreeLevel stringToDegreeLevel(const QString& levelStr);

    // Validation
    bool isValidThesis() const;

private:
    void initializeThesis();
    void validateThesisData() const;
};

// Custom exception class for Thesis-related errors
class ThesisException : public std::exception {
private:
    QString m_message;

public:
    explicit ThesisException(const QString& message) : m_message(message) {}
    const char* what() const noexcept override {
        return m_message.toStdString().c_str();
    }
    QString getMessage() const { return m_message; }
};

#endif // THESIS_H
