# Contributing to ENSIARY

Thank you for your interest in contributing to ENSIARY! This document provides guidelines and information for contributors.

## 📋 Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Contributing Guidelines](#contributing-guidelines)
- [Pull Request Process](#pull-request-process)
- [Coding Standards](#coding-standards)
- [Issue Reporting](#issue-reporting)

## 🤝 Code of Conduct

This project and everyone participating in it is governed by our commitment to creating a welcoming and inclusive environment. Please be respectful and constructive in all interactions.

## 🚀 Getting Started

### Prerequisites

- Qt 6.8.0 or later
- C++20 compatible compiler
- Git for version control
- Qt Creator (recommended IDE)

### Development Setup

1. **Fork the Repository**
   ```bash
   # Fork on GitHub, then clone your fork
   git clone https://github.com/YOUR_USERNAME/ENSIARY.git
   cd ENSIARY
   ```

2. **Set up Development Environment**
   ```bash
   # Add upstream remote
   git remote add upstream https://github.com/ORIGINAL_OWNER/ENSIARY.git
   
   # Create a new branch for your feature
   git checkout -b feature/your-feature-name
   ```

3. **Build the Project**
   ```bash
   # Using Qt Creator (recommended)
   # Open ENSIARY.pro in Qt Creator and build
   
   # Or using command line
   qmake ENSIARY.pro
   make
   ```

## 🛠️ Contributing Guidelines

### Types of Contributions

We welcome various types of contributions:

- **Bug Fixes**: Fix existing issues and improve stability
- **Feature Enhancements**: Add new functionality or improve existing features
- **Documentation**: Improve README, code comments, or add tutorials
- **Performance Improvements**: Optimize code for better performance
- **UI/UX Improvements**: Enhance user interface and user experience
- **Testing**: Add unit tests or improve test coverage

### Before You Start

1. **Check Existing Issues**: Look for existing issues or feature requests
2. **Discuss Major Changes**: For significant changes, create an issue first to discuss
3. **Follow Coding Standards**: Ensure your code follows the project's coding standards

## 🔄 Pull Request Process

### Creating a Pull Request

1. **Update Your Branch**
   ```bash
   git fetch upstream
   git checkout main
   git merge upstream/main
   git checkout feature/your-feature-name
   git rebase main
   ```

2. **Make Your Changes**
   - Write clean, well-documented code
   - Follow the existing code style
   - Add appropriate comments
   - Update documentation if needed

3. **Test Your Changes**
   - Build the project successfully
   - Test all functionality thoroughly
   - Ensure no regressions are introduced

4. **Commit Your Changes**
   ```bash
   git add .
   git commit -m "feat: add your descriptive commit message"
   ```

5. **Push and Create PR**
   ```bash
   git push origin feature/your-feature-name
   ```
   Then create a pull request on GitHub.

### Pull Request Requirements

- **Clear Description**: Explain what the PR does and why
- **Link Issues**: Reference any related issues
- **Screenshots**: Include screenshots for UI changes
- **Testing**: Describe how you tested the changes
- **Documentation**: Update relevant documentation

### Commit Message Convention

We follow conventional commit messages:

- `feat:` New feature
- `fix:` Bug fix
- `docs:` Documentation changes
- `style:` Code style changes (formatting, etc.)
- `refactor:` Code refactoring
- `test:` Adding or updating tests
- `chore:` Maintenance tasks

Example:
```
feat: add reservation expiration notification system

- Implement automatic notification for expiring reservations
- Add email template for reservation expiry warnings
- Update user interface to show notification status
- Add configuration options for notification timing

Closes #123
```

## 📝 Coding Standards

### C++ Guidelines

- **Modern C++20**: Use modern C++ features appropriately
- **Naming Conventions**:
  - Classes: `PascalCase` (e.g., `LibraryManager`)
  - Functions/Methods: `camelCase` (e.g., `getUserById`)
  - Variables: `camelCase` (e.g., `currentUser`)
  - Member variables: `m_` prefix (e.g., `m_userId`)
  - Constants: `UPPER_CASE` (e.g., `MAX_LOAN_PERIOD`)

- **Code Style**:
  - Use 4 spaces for indentation
  - Maximum line length: 100 characters
  - Place opening braces on the same line
  - Use meaningful variable and function names

### Qt Guidelines

- **Signal/Slot Connections**: Use new-style syntax with function pointers
- **Memory Management**: Use smart pointers and RAII principles
- **UI Design**: Use Qt Designer for UI layouts
- **Internationalization**: Prepare strings for translation with `tr()`

### Documentation

- **Header Files**: Document all public interfaces
- **Complex Functions**: Add detailed comments for complex algorithms
- **Class Documentation**: Include class purpose and usage examples
- **README Updates**: Update documentation for new features

Example:
```cpp
/**
 * @brief Processes a book loan for a specific user
 * 
 * This function handles the complete loan process including:
 * - Availability checking
 * - User eligibility verification
 * - Due date calculation
 * - Database updates
 * 
 * @param userId The unique identifier of the borrowing user
 * @param resourceId The unique identifier of the resource to loan
 * @param loanPeriod Optional custom loan period in days
 * @return LoanResult containing success status and loan details
 * @throws LoanException if the loan cannot be processed
 */
LoanResult processLoan(const QString& userId, const QString& resourceId, int loanPeriod = -1);
```

## 🐛 Issue Reporting

### Bug Reports

When reporting bugs, please include:

- **Environment**: OS, Qt version, compiler version
- **Steps to Reproduce**: Clear steps to reproduce the issue
- **Expected Behavior**: What you expected to happen
- **Actual Behavior**: What actually happened
- **Screenshots**: If applicable, add screenshots
- **Additional Context**: Any other relevant information

### Feature Requests

For feature requests, please include:

- **Use Case**: Describe the problem you're trying to solve
- **Proposed Solution**: Your suggested approach
- **Alternatives**: Any alternative solutions you've considered
- **Additional Context**: Any other relevant information

### Issue Labels

We use the following labels to categorize issues:

- `bug`: Something isn't working
- `enhancement`: New feature or request
- `documentation`: Improvements or additions to documentation
- `good first issue`: Good for newcomers
- `help wanted`: Extra attention is needed
- `invalid`: This doesn't seem right
- `question`: Further information is requested
- `wontfix`: This will not be worked on

## 🎯 Development Workflow

### Branch Strategy

- `main`: Stable production branch
- `develop`: Integration branch for features
- `feature/*`: Feature development branches
- `hotfix/*`: Critical bug fixes
- `release/*`: Release preparation branches

### Testing

- **Manual Testing**: Test all functionality thoroughly
- **Integration Testing**: Ensure components work together
- **Regression Testing**: Verify existing functionality still works
- **Performance Testing**: Check for performance regressions

### Code Review

All contributions go through code review:

- **Peer Review**: At least one other developer reviews the code
- **Automated Checks**: GitHub Actions runs automated tests
- **Documentation Review**: Ensure documentation is updated
- **User Experience Review**: Consider impact on user experience

## 📚 Resources

### Documentation

- [Qt Documentation](https://doc.qt.io/)
- [C++20 Reference](https://en.cppreference.com/)
- [Git Workflow Guide](https://www.atlassian.com/git/tutorials)

### Tools

- [Qt Creator](https://www.qt.io/product/development-tools)
- [Git](https://git-scm.com/)
- [GitHub Desktop](https://desktop.github.com/) (optional)

## 🤔 Questions?

If you have questions about contributing:

1. Check existing issues and discussions
2. Create a new issue with the `question` label
3. Reach out to the maintainers

## 🙏 Recognition

Contributors will be recognized in:

- README.md contributor section
- Release notes for significant contributions
- GitHub contributor graphs

Thank you for contributing to ENSIARY! Your efforts help make this project better for everyone.

---

**Happy Coding!** 🚀

*ENSIARY Development Team*
