#ifndef ARTICLE_H
#define ARTICLE_H

#include "resource.h"
#include <QString>

/**
 * @brief Concrete implementation of Resource for articles
 * 
 * This class represents an article in the library system and demonstrates
 * inheritance from the abstract Resource base class.
 */
class Article : public Resource {
private:
    QString m_journal;
    int m_volume;
    int m_issue;
    QString m_pageRange;
    QString m_doi; // Digital Object Identifier
    QString m_abstract;
    QStringList m_keywords;
    QString m_researchField;

public:
    // Constructor
    Article(const QString& id, const QString& title, const QString& author,
            int publicationYear, const QString& journal, int volume = 0,
            int issue = 0, const QString& pageRange = "",
            const QString& doi = "", const QString& researchField = "");

    // Override virtual functions from Resource
    QString getDetails() const override;
    QString getResourceType() const override;
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& json) override;    // Article-specific getters
    QString getJournal() const { return m_journal; }
    QString getVolume() const { return QString::number(m_volume); }
    QString getIssue() const { return QString::number(m_issue); }
    QString getPageRange() const { return m_pageRange; }
    QString getDoi() const { return m_doi; }
    QString getAbstract() const { return m_abstract; }
    QStringList getKeywords() const { return m_keywords; }
    QString getResearchField() const { return m_researchField; }

    // Article-specific setters with validation
    void setJournal(const QString& journal);
    void setVolume(int volume);
    void setIssue(int issue);
    void setPageRange(const QString& pageRange);
    void setDoi(const QString& doi);
    void setAbstract(const QString& abstract);
    void setKeywords(const QStringList& keywords);
    void addKeyword(const QString& keyword);
    void removeKeyword(const QString& keyword);
    void setResearchField(const QString& researchField);

    // Utility functions
    QString getFormattedDetails() const;
    QString getCitation() const;
    bool isValidDoi(const QString& doi) const;

private:
    void validateArticleData() const;
};

#endif // ARTICLE_H
