#ifndef USER_DIALOG_H
#define USER_DIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QLabel>
#include <QDateEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QString>
#include <QMessageBox>
#include <QRegularExpressionValidator>

#include "../models/user.h"

/**
 * @brief Dialog for adding and editing library users
 * 
 * This dialog provides a form interface for creating new users or editing
 * existing ones with proper validation for email addresses and other fields.
 */
class UserDialog : public QDialog {
    Q_OBJECT

public:
    enum class Mode {
        Add,
        Edit
    };

    // Constructors
    explicit UserDialog(Mode mode, QWidget* parent = nullptr);
    explicit UserDialog(User* user, QWidget* parent = nullptr);

    // Get the created/edited user
    std::unique_ptr<User> getUser() const;

private slots:
    void onAccepted();
    void validateAndAccept();
    void onEmailChanged();
    void onPhoneChanged();
    void onUserTypeChanged(); // New slot for handling user type changes

private:
    void setupUI();
    void setupConnections();
    void setupValidation();
    void populateFields(User* user);
    bool validateInput();
    void showValidationError(const QString& message);
    bool isValidEmail(const QString& email) const;
    bool isValidPhone(const QString& phone) const;

    // UI Components
    QVBoxLayout* m_mainLayout;
    QFormLayout* m_formLayout;
    QDialogButtonBox* m_buttonBox;
    
    // User fields
    QLineEdit* m_idEdit;
    QLineEdit* m_nameEdit;
    QLineEdit* m_emailEdit;
    QLineEdit* m_phoneEdit;
    QLineEdit* m_addressEdit;    QComboBox* m_typeComboBox;
    QComboBox* m_statusComboBox;
    QSpinBox* m_maxLoansSpinBox;
    QTextEdit* m_notesEdit;
    QComboBox* m_yearComboBox; // Year field for students
    QLabel* m_yearLabel; // Label for year field
    
    // Validation labels
    QLabel* m_emailValidationLabel;
    QLabel* m_phoneValidationLabel;
    
    // State
    Mode m_mode;
    User* m_originalUser;
    mutable std::unique_ptr<User> m_resultUser;
};

#endif // USER_DIALOG_H
