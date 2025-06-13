#ifndef USER_LOANS_DIALOG_H
#define USER_LOANS_DIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPushButton>
#include <QGroupBox>
#include <QTextEdit>
#include <QSplitter>
#include <QHeaderView>
#include <QString>

#include "../models/user.h"
#include "../models/loan.h"
#include "../services/library_manager.h"

/**
 * @brief Dialog for viewing and managing a user's loans
 * 
 * This dialog displays all current and past loans for a specific user,
 * along with loan details and management options.
 */
class UserLoansDialog : public QDialog {
    Q_OBJECT

public:
    explicit UserLoansDialog(User* user, LibraryManager* libraryManager, QWidget* parent = nullptr);

private slots:
    void onLoanSelectionChanged();
    void onReturnLoan();
    void onRenewLoan();
    void onRefresh();

private:
    void setupUI();
    void setupConnections();
    void loadLoanData();
    void populateCurrentLoans();
    void populateLoanHistory();
    void updateLoanDetails();
    void updateButtonStates();
    
    QString formatLoanStatus(const Loan& loan) const;
    QString formatDueDate(const QDateTime& dueDate) const;
    int calculateDaysOverdue(const QDateTime& dueDate) const;

    // UI Components
    QVBoxLayout* m_mainLayout;
    QSplitter* m_splitter;
    
    // User info section
    QGroupBox* m_userInfoGroup;
    QLabel* m_userNameLabel;
    QLabel* m_userEmailLabel;
    QLabel* m_userTypeLabel;
    QLabel* m_userStatusLabel;
    QLabel* m_currentLoansCountLabel;
    
    // Current loans section
    QGroupBox* m_currentLoansGroup;
    QTableWidget* m_currentLoansTable;
    
    // Loan history section
    QGroupBox* m_historyGroup;
    QTableWidget* m_historyTable;
    
    // Loan details section
    QGroupBox* m_detailsGroup;
    QTextEdit* m_detailsText;
    
    // Action buttons
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_returnBtn;
    QPushButton* m_renewBtn;
    QPushButton* m_refreshBtn;
    QPushButton* m_closeBtn;
    
    // Data
    User* m_user;
    LibraryManager* m_libraryManager;
    QString m_selectedLoanId;
    std::vector<std::unique_ptr<Loan>> m_currentLoans;
    std::vector<std::unique_ptr<Loan>> m_loanHistory;
};

#endif // USER_LOANS_DIALOG_H
