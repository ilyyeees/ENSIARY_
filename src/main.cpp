/**
 * @file main.cpp
 * @brief Main entry point for the ENSIARY Library Management System
 * @author ENSIARY Development Team
 * @date 2025
 * @version 1.0.0
 */

#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QLocale>
#include <QTranslator>

#include "mainwindow.h"

/**
 * @brief Sets up the application directories and ensures they exist
 * @return true if directories were created successfully, false otherwise
 */
bool setupApplicationDirectories() {
    QDir appDir;
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    
    if (!appDir.exists(dataPath)) {
        if (!appDir.mkpath(dataPath)) {
            QMessageBox::critical(nullptr, "ENSIARY - Error", 
                                "Failed to create application data directory.\n" +
                                dataPath);
            return false;
        }
    }
    
    return true;
}

/**
 * @brief Configures the application appearance and style
 * @param app Reference to the QApplication instance
 */
void configureApplicationStyle(QApplication& app) {
    // Set application properties
    app.setApplicationName("ENSIARY");
    app.setApplicationVersion("1.0.0");
    app.setApplicationDisplayName("ENSIARY - Library Management System");
    app.setOrganizationName("ENSIARY Development Team");
    app.setOrganizationDomain("ensiary.edu");
      // Set modern application style
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // Apply modern light theme compatible with UI
    QPalette lightPalette;
    lightPalette.setColor(QPalette::Window, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::WindowText, QColor(30, 58, 138)); // #1e3a8a
    lightPalette.setColor(QPalette::Base, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::AlternateBase, QColor(254, 243, 199)); // #fef3c7
    lightPalette.setColor(QPalette::ToolTipBase, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::ToolTipText, QColor(30, 58, 138));
    lightPalette.setColor(QPalette::Text, QColor(30, 58, 138)); // #1e3a8a
    lightPalette.setColor(QPalette::Button, QColor(249, 115, 22)); // #f97316
    lightPalette.setColor(QPalette::ButtonText, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::BrightText, Qt::red);
    lightPalette.setColor(QPalette::Link, QColor(249, 115, 22));
    lightPalette.setColor(QPalette::Highlight, QColor(249, 115, 22));
    lightPalette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    
    app.setPalette(lightPalette);
    
    // Set application icon (if available)
    // app.setWindowIcon(QIcon(":/icons/ensiary.png"));
}

/**
 * @brief Main function - Entry point of the ENSIARY application
 * @param argc Number of command line arguments
 * @param argv Command line arguments array
 * @return Application exit code
 */
int main(int argc, char *argv[])
{    // Create the main application instance
    QApplication app(argc, argv);    // Set application icon
    app.setWindowIcon(QIcon(":/icon.png"));
    
    // Configure application properties and style
    configureApplicationStyle(app);
    
    // Setup application directories
    if (!setupApplicationDirectories()) {
        return -1;
    }
    
    // Setup localization (optional - for future internationalization)
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "ENSIARY_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }
    
    try {
        // Create and show the main window
        MainWindow window;
        window.show();
        
        // Enter the main event loop
        return app.exec();
    }
    catch (const std::exception& e) {
        // Handle any unhandled exceptions
        QMessageBox::critical(nullptr, "ENSIARY - Critical Error", 
                            QString("An unexpected error occurred:\n%1\n\n"
                                   "The application will now exit.").arg(e.what()));
        return -1;
    }
    catch (...) {
        // Handle any other unhandled exceptions
        QMessageBox::critical(nullptr, "ENSIARY - Critical Error", 
                            "An unknown error occurred.\n"
                            "The application will now exit.");
        return -1;
    }
}
