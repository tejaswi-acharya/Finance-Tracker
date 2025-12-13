
#include "raylib.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <cmath>
#include <cstring>
#include <iomanip> 

using namespace std;

//---------------global declaration for the colors used by buttons and other draw functions------------------------------------
Color BG = Color{ 237,234,255,255 };
Color BTN = Color{ 255,205,233, 230 };
Color BTN_HOVER = Color{ 255,223,186, 255 };
Color BTN_TEXT = DARKGRAY;
Color INFO_BOX = Color{ 247, 229, 181, 255 }; // New color for info boxes and input fields

// ------------------- Expense Struct -------------------

struct Expense {
    string category;
    float amount;
};

// ------------------- Button Struct -------------------
struct Button {
    Rectangle rect;
    string label;
    //-----------------------------------------------------------------

    void Draw(bool hovered, Font font) const {
        DrawRectangleRounded(rect, 0.4f, 12, hovered ? BTN_HOVER : BTN);
        DrawTextEx(font, label.c_str(), { rect.x + 10, rect.y + 8 }, 20, 1, BTN_TEXT);
    }

    bool IsHovered(Vector2 mouse) {
        return CheckCollisionPointRec(mouse, rect);
    }
};

// -------------------- USER LOGIN SYSTEM CLASS --------------------
class User {
public:
    string currentUsername;

    // Registers a new user, checks for the user limit and duplicate users too. 
    bool register_user(const string& username, const string& password) {
        ifstream readFile("users.txt");
        string existingUser, existingPass;
        int userCount = 0;

        
        while (readFile >> existingUser >> existingPass) {
            if (existingUser == username) {
                readFile.close();
                return false; 
            }
            userCount++;
        }
        readFile.close();

        
        if (userCount >= 8) {
            return false; 
        }

        // Register the new user
        ofstream writeFile("users.txt", ios::app);
        if (!writeFile.is_open()) {
            return false; 
        }
        writeFile << username << " " << password << endl;
        writeFile.close();

        currentUsername = username;

        // Reset the tracker for the new user before loading.
        tracker.ResetForNewMonth();
        tracker.income = 0; // Ensure income is also reset
        tracker.totalSavings = 0; // Ensure savings are also reset

        return true;
    }

    // Logs in a user.
    bool login(const string& username, const string& password) {
        ifstream readFile("users.txt");
        if (!readFile.is_open()) {
            return false; // No user file found
        }
        string existingUser, existingPass;

        while (readFile >> existingUser >> existingPass) {
            if (existingUser == username && existingPass == password) {
                currentUsername = username;
                readFile.close();

                // Check if the user's data file exists
                ifstream userFile(currentUsername + "_finance.txt");
                if (!userFile.is_open()) {
                    // If the file does not exist, reset the tracker to a clean slate.
                    tracker.income = 0;
                    tracker.totalSavings = 0;
                    tracker.monthlySavings = 0;
                    tracker.expenses.clear();
                    tracker.budgets.clear();
                }
                userFile.close();

                // Now, load the data. If the file exists, it will be loaded. 
                // If not, the tracker is already clean.
                tracker.Load(currentUsername);

                return true; // Login successful
            }
        }
        readFile.close();
        return false; // Invalid credentials
    }
};

// ------------------- FINANCE TRACKER CLASS -------------------
class FinanceTracker {
public:
    float income = 0;
    float totalSavings = 0; // New variable to track total savings over time
    float monthlySavings = 0; // New variable for savings this month
    vector<Expense> expenses;   //vector for saving the items in a perfect order
    map<string, float> budgets;//maps for accessing the items easily as it has a key value pair feature.

    void SetBudget(const string& cat, float amt) {
        budgets[cat] = amt;
    }

    void AddIncome(float amt) { income = amt; }

    void AddExpense(string cat, float amt) { expenses.push_back({ cat, amt }); }

    // New function to handle saving money
    void SaveMoney(float amt) {
        if (income >= amt) {
            income -= amt;
            monthlySavings += amt;
            totalSavings += amt;
        }
    }

    void Save(const string& username) {
        ofstream file(username + "_finance.txt");
        if (!file.is_open()) {
            return;
        }
        file << income << "\n";
        file << totalSavings << "\n"; // Save total savings
        file << monthlySavings << "\n"; // Save monthly savings
        file << budgets.size() << "\n";
        for (auto const& pair : budgets) {
            file << pair.first << " " << pair.second << "\n";
        }
        file << expenses.size() << "\n";
        for (auto& e : expenses)
            file << e.category << " " << e.amount << "\n";
        file.close();
    }


    void Load(const string& username) {
        ifstream file(username + "_finance.txt");
        if (!file.is_open()) return;

        expenses.clear();
        budgets.clear();

        file >> income;
        file >> totalSavings; // Load total savings
        file >> monthlySavings; // Load monthly savings

        size_t numBudgets;
        file >> numBudgets;
        for (size_t i = 0; i < numBudgets; ++i) {
            string cat;
            float amt;
            file >> cat >> amt;
            budgets[cat] = amt;
        }

        size_t numExpenses;
        file >> numExpenses;
        string cat; float amt;
        for (size_t i = 0; i < numExpenses; ++i) {
            file >> cat >> amt;
            expenses.push_back({ cat, amt });
        }
        file.close();
    }

    // Calculates and returns the total expenses for each category.
    map<string, float> GetCategoryTotals() {
        map<string, float> totals;
        for (auto& e : expenses) totals[e.category] += e.amount;
        return totals;
    }

    // Calculates and returns the total of all expenses.
    float GetTotalExpense() {
        float total = 0;
        for (auto& e : expenses) total += e.amount;
        return total;
    }

    // Returns a constant reference to the budgets map.
    const map<string, float>& GetBudgets() const {
        return budgets;
    }

    // Checks if a budget is set for a specific category
    bool IsBudgetSet(const string& category) {
        return budgets.count(category) && budgets[category] > 0;
    }

    // Resets the income, expenses, and budgets for a new month.
    void ResetForNewMonth() {
        income = 0;
        expenses.clear();
        budgets.clear();
        monthlySavings = 0; // Reset monthly savings but not total savings
    }
};


// ================= GLOBAL VARIABLES AND ENUMS---------------------------------------------------
Font customFont;
FinanceTracker tracker;
User user;

// ------------------- Global State Variables -------------------
bool showStartupSummary = true;
string inputText = "";
string currentAction = "";
vector<string> categories = { "Food", "Transport", "Entertainment", "Health", "Bills", "Other" };

// ------------------- Page Enum -------------------
enum Page {
    LOGIN,
    HOME,
    PAGE_ADD_INCOME,
    PAGE_ADD_EXPENSE,
    PAGE_ANALYTICS,
    PAGE_SET_BUDGET,
    PAGE_SUGGESTIONS,
    PAGE_CONFIRM_RESET,
    PAGE_LOGOUT_CONFIRM,
    PAGE_CONFIRM_SAVE // New page for save confirmation
};

Page currentPage = LOGIN;
//-------------------------------------------------------------
string message = ""; // Global message for user feedback

// ------------------- Input Tracking for Multiple Categories (Expenses) -------------------
map<string, bool> expenseCategorySelected;
map<string, string> expenseInputText; 
string selectedExpenseCategoryInput = ""; 

// ------------------- Input Tracking for Multiple Categories (Budgets) -------------------
map<string, bool> budgetCategorySelected;
map<string, string> budgetInputText;
string selectedBudgetCategoryInput = "";

// Draws the login/registration page,  a simple function that draws the screen
static void DrawLoginScreen(bool& loggedIn) {
    static char username[30] = "";
    static char password[30] = "";
    static int inputMode = 0;
    static bool isRegistering = true;
    static string loginMessage = "";

    Color loginColor = { 183, 225, 245, 255 };
    ClearBackground(loginColor);

    // Draw the title based on whether the user is logging in or registering.
    DrawTextEx(customFont, isRegistering ? "Register" : "Login", { 250, 30 }, 45, 1, PINK);

    // Draw the username input box.
    DrawTextEx(customFont, "Username:", { 100, 100 }, 30, 1, BLACK);
    Rectangle usernameBox = { 220, 95, 200, 30 };
    Color BoxColor = { 247, 229, 181, 255 };
    DrawRectangleRec(usernameBox, BoxColor);
    DrawTextEx(customFont, username, { 225, 100 }, 20, 1, BLACK);

    // Draw the password input box, with text hidden by asterisks.
    DrawTextEx(customFont, "Password:", { 100, 150 }, 30, 1, BLACK);
    Rectangle passwordBox = { 220, 145, 200, 30 };
    DrawRectangleRec(passwordBox, BoxColor);
    string hidden(strlen(password), '*');
    DrawTextEx(customFont, hidden.c_str(), { 225, 150 }, 20, 1, BLACK);

    // Handle user clicks to select an input box.
    if (CheckCollisionPointRec(GetMousePosition(), usernameBox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) inputMode = 1;
    if (CheckCollisionPointRec(GetMousePosition(), passwordBox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) inputMode = 2;

    // Handle text input from the keyboard.
    int key = GetCharPressed();
    while (key > 0) {
        if ((key >= 32) && (key <= 125)) {
            if (inputMode == 1 && strlen(username) < 29) {
                int len = strlen(username);
                username[len] = (char)key;
                username[len + 1] = '\0';
            }
            else if (inputMode == 2 && strlen(password) < 29) {
                int len = strlen(password);
                password[len] = (char)key;
                password[len + 1] = '\0';
            }
        }
        key = GetCharPressed();
    }

    // Handle backspace key.
    if (IsKeyPressed(KEY_BACKSPACE)) {
        if (inputMode == 1 && strlen(username) > 0)
            username[strlen(username) - 1] = '\0';
        else if (inputMode == 2 && strlen(password) > 0)
            password[strlen(password) - 1] = '\0';
    }

    // Draw the submit and switch buttons.
    Rectangle submitBtn = { 220, 200, 100, 30 };
    Rectangle switchBtn = { 330, 200, 120, 30 };

    DrawRectangleRec(submitBtn, DARKGRAY);
    DrawTextEx(customFont, isRegistering ? "Register" : "Login", { submitBtn.x + 10, submitBtn.y + 5 }, 20, 1, WHITE);

    DrawRectangleRec(switchBtn, GRAY);
    DrawTextEx(customFont, isRegistering ? "To Login" : "To Register", { switchBtn.x + 10, switchBtn.y + 5 }, 20, 1, BLACK);

    // Handle button clicks.
    if (CheckCollisionPointRec(GetMousePosition(), submitBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (strlen(username) == 0 || strlen(password) == 0) {
            loginMessage = "Please fill both fields.";
        }
        else {
            if (isRegistering) {
                // Call the registration logic.
                if (user.register_user(username, password)) {
                    loggedIn = true;
                    currentPage = HOME; // Change page on successful login
                    tracker.Load(user.currentUsername); // Load data for the new user
                }
                else {
                    loginMessage = "User exists or account limit reached (max 8).";
                }
            }
            else {
                // Call the login logic.
                if (user.login(username, password)) {
                    loggedIn = true;
                    currentPage = HOME; // Change page on successful login
                    tracker.Load(user.currentUsername); // Load data for the logged-in user
                }
                else {
                    loginMessage = "Invalid username or password.";
                }
            }
        }
    }

    // Handle the switch button click to toggle between login and registration.
    if (CheckCollisionPointRec(GetMousePosition(), switchBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        isRegistering = !isRegistering;
        loginMessage = "";
        memset(username, 0, sizeof(username)); // Clear inputs on switch
        memset(password, 0, sizeof(password));
    }

    // Draw the warning message.
    DrawTextEx(customFont, loginMessage.c_str(), { 180, 250 }, 20, 1, RED);
}

// Draws a customizable input box with a prompt.
static void DrawInputBox(const string& prompt, Rectangle box, Font font, string& input) {
    DrawRectangleRounded(box, 0.2f, 10, WHITE);
    DrawRectangleLinesEx(box, 2, GRAY);
    DrawTextEx(customFont, prompt.c_str(), { box.x + 10, box.y - 30 }, 20, 1, DARKGRAY);
    DrawTextEx(customFont, input.c_str(), { box.x + 10, box.y + 10 }, 20, 1, DARKGRAY);

    int key = GetCharPressed();
    while (key > 0) {
        if ((key >= 48 && key <= 57) || key == 46) {
            if (input.length() < 15) input += (char)key;
        }
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && !input.empty()) {
        input.pop_back();
    }
}

// Draws a simple message box for user feedback
static void ShowMessage(const string& msg, int x, int y, Color color) {
    DrawTextEx(customFont, msg.c_str(), { (float)x, (float)y }, 20, 1, color);
}

// Draws checkboxes and input fields for multiple expense categories.
static void DrawExpenseInputs(int boxX, int boxY, Font font, const vector<string>& categories) {
    int yOffset = boxY + 90;

    for (const string& cat : categories) {
        Rectangle checkboxRect = { (float)boxX + 50, (float)yOffset, 20, 20 };
        DrawRectangleLinesEx(checkboxRect, 2, GRAY);
        if (expenseCategorySelected.count(cat) && expenseCategorySelected[cat]) {
            DrawLine(checkboxRect.x, checkboxRect.y, checkboxRect.x + 20, checkboxRect.y + 20, DARKGREEN);
            DrawLine(checkboxRect.x, checkboxRect.y + 20, checkboxRect.x + 20, checkboxRect.y, DARKGREEN);
        }

        if (CheckCollisionPointRec(GetMousePosition(), checkboxRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            selectedExpenseCategoryInput = cat;
            expenseCategorySelected[cat] = !expenseCategorySelected[cat];
        }

        DrawTextEx(font, cat.c_str(), { checkboxRect.x + 30, checkboxRect.y - 2 }, 20, 1, DARKGRAY);

        if (expenseCategorySelected.count(cat) && expenseCategorySelected[cat]) {
            Rectangle inputRect = { checkboxRect.x + 180, checkboxRect.y - 5, 150, 30 };
            DrawRectangleRounded(inputRect, 0.2f, 6, INFO_BOX);
            DrawRectangleLinesEx(inputRect, 1, DARKGRAY);

            if (expenseInputText.find(cat) == expenseInputText.end()) {
                expenseInputText[cat] = "";
            }

            DrawTextEx(font, expenseInputText[cat].c_str(), { inputRect.x + 5, inputRect.y + 5 }, 20, 1, BLACK);

            if (CheckCollisionPointRec(GetMousePosition(), inputRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                selectedExpenseCategoryInput = cat;
            }

            if (selectedExpenseCategoryInput == cat) {
                int key = GetCharPressed();
                while (key > 0) {
                    if ((key >= 48 && key <= 57) || key == 46) {
                        if (expenseInputText[cat].length() < 15) {
                            expenseInputText[cat] += (char)key;
                        }
                    }
                    key = GetCharPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE) && !expenseInputText[cat].empty()) {
                    expenseInputText[cat].pop_back();
                }
            }
        }
        yOffset += 40;
    }
}

// Draws checkboxes and input fields for multiple budget categories.
static void DrawBudgetInputs(int boxX, int boxY, Font font, const vector<string>& categories) {
    int yOffset = boxY + 90;

    for (const string& cat : categories) {
        Rectangle checkboxRect = { (float)boxX + 50, (float)yOffset, 20, 20 };
        DrawRectangleLinesEx(checkboxRect, 2, GRAY);
        if (budgetCategorySelected.count(cat) && budgetCategorySelected[cat]) {
            DrawLine(checkboxRect.x, checkboxRect.y, checkboxRect.x + 20, checkboxRect.y + 20, DARKGREEN);
            DrawLine(checkboxRect.x, checkboxRect.y + 20, checkboxRect.x + 20, checkboxRect.y, DARKGREEN);
        }

        if (CheckCollisionPointRec(GetMousePosition(), checkboxRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            selectedBudgetCategoryInput = cat;
            budgetCategorySelected[cat] = !budgetCategorySelected[cat];
        }

        DrawTextEx(font, cat.c_str(), { checkboxRect.x + 30, checkboxRect.y - 2 }, 20, 1, DARKGRAY);

        if (budgetCategorySelected.count(cat) && budgetCategorySelected[cat]) {
            Rectangle inputRect = { checkboxRect.x + 180, checkboxRect.y - 5, 150, 30 };
            DrawRectangleRounded(inputRect, 0.2f, 6, INFO_BOX);
            DrawRectangleLinesEx(inputRect, 1, DARKGRAY);

            if (budgetInputText.find(cat) == budgetInputText.end()) {
                budgetInputText[cat] = "";
            }

            DrawTextEx(font, budgetInputText[cat].c_str(), { inputRect.x + 5, inputRect.y + 5 }, 20, 1, BLACK);

            if (CheckCollisionPointRec(GetMousePosition(), inputRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                selectedBudgetCategoryInput = cat;
            }

            if (selectedBudgetCategoryInput == cat) {
                int key = GetCharPressed();
                while (key > 0) {
                    if ((key >= 48 && key <= 57) || key == 46) {
                        if (budgetInputText[cat].length() < 15) {
                            budgetInputText[cat] += (char)key;
                        }
                    }
                    key = GetCharPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE) && !budgetInputText[cat].empty()) {
                    budgetInputText[cat].pop_back();
                }
            }
        }
        yOffset += 40;
    }
}

// Draws a live summary box displaying income, total expenses, and remaining balance with centered text.
static void DrawLiveSummaryBox(FinanceTracker& tracker, Font font, int screenWidth) {
    float boxWidth = 450;
    float boxHeight = 180;
    Rectangle box = {
        screenWidth / 2.0f - boxWidth / 2.0f,
        150,
        boxWidth,
        boxHeight
    };

    DrawRectangleRounded(box, 0.15f, 8, WHITE);
    DrawRectangleRoundedLines(box, 0.15f, 8, GRAY);

    float totalExpense = tracker.GetTotalExpense();
    float remaining = tracker.income - totalExpense;

    stringstream ss;
    ss << fixed << setprecision(2);

    string titleText = "Monthly Overview";
    float titleWidth = MeasureTextEx(font, titleText.c_str(), 22, 1).x;
    DrawTextEx(font, titleText.c_str(), { box.x + (boxWidth - titleWidth) / 2, box.y + 10 }, 22, 1, DARKGRAY);

    ss.str(""); ss << "Income: Rs " << tracker.income;
    string incomeText = ss.str();
    float incomeWidth = MeasureTextEx(font, incomeText.c_str(), 18, 1).x;
    DrawTextEx(font, incomeText.c_str(), { box.x + (boxWidth - incomeWidth) / 2, box.y + 45 }, 18, 1, DARKGRAY);

    ss.str(""); ss << "Expense: Rs " << totalExpense;
    string expenseText = ss.str();
    float expenseWidth = MeasureTextEx(font, expenseText.c_str(), 18, 1).x;
    DrawTextEx(font, expenseText.c_str(), { box.x + (boxWidth - expenseWidth) / 2, box.y + 70 }, 18, 1, DARKGRAY);

    ss.str(""); ss << "Remaining: Rs " << remaining;
    string remainingText = ss.str();
    float remainingWidth = MeasureTextEx(font, remainingText.c_str(), 18, 1).x;
    DrawTextEx(font, remainingText.c_str(), { box.x + (boxWidth - remainingWidth) / 2, box.y + 95 }, 18, 1, DARKGRAY);

    ss.str(""); ss << "Saved This Month: Rs " << tracker.monthlySavings;
    string monthlySavingsText = ss.str();
    float monthlySavingsWidth = MeasureTextEx(font, monthlySavingsText.c_str(), 18, 1).x;
    DrawTextEx(font, monthlySavingsText.c_str(), { box.x + (boxWidth - monthlySavingsWidth) / 2, box.y + 120 }, 18, 1, DARKGREEN);

    ss.str(""); ss << "Total Savings: Rs " << tracker.totalSavings;
    string totalSavingsText = ss.str();
    float totalSavingsWidth = MeasureTextEx(font, totalSavingsText.c_str(), 18, 1).x;
    DrawTextEx(font, totalSavingsText.c_str(), { box.x + (boxWidth - totalSavingsWidth) / 2, box.y + 145 }, 18, 1, DARKGREEN);
}

// Draws a pie chart representing expense breakdown by category.
static void DrawPieChart(map<string, float> data, int centerX, int centerY, float radius) {
    float total = 0;
    for (auto& e : data) total += e.second;

    float startAngle = 0;
    int colorIndex = 0;
    vector<Color> pieColors = {
        RED, ORANGE, GOLD, GREEN, BLUE, PURPLE, VIOLET, PINK, LIME, SKYBLUE
    };

    if (total == 0) {
        DrawText("No expenses to display.", centerX - 100, centerY - 10, 20, DARKGRAY);
        return;
    }

    stringstream ss;
    ss << fixed << setprecision(2);

    for (auto& entry : data) {
        float percentage = entry.second / total;
        float sweepAngle = percentage * 360;

        Color color = pieColors[colorIndex % pieColors.size()];
        DrawCircleSector({ (float)centerX, (float)centerY }, radius, startAngle, startAngle + sweepAngle, 60, color);

        float midAngle = (startAngle + sweepAngle / 2) * DEG2RAD;
        float labelX = centerX + cos(midAngle) * (radius + 15);
        float labelY = centerY + sin(midAngle) * (radius + 15);

        ss.str(""); ss << entry.first << "\nRs " << entry.second;
        string label = ss.str();
        Vector2 textSize = MeasureTextEx(customFont, label.c_str(), 16, 1);
        DrawTextEx(customFont, label.c_str(), { (labelX - textSize.x / 2),(labelY - textSize.y / 2) }, 16, 1, DARKGRAY);

        startAngle += sweepAngle;
        colorIndex++;
    }
}
//drawing the suggestion page
static void DrawSuggestionsPage(FinanceTracker& tracker, Font font, int screenWidth, int screenHeight) {
    float boxWidth = 600;
    float boxHeight = 400;
    float boxX = screenWidth / 2.0f - boxWidth / 2.0f;
    float boxY = screenHeight / 2.0f - boxHeight / 2.0f;

    DrawRectangleRounded({ boxX, boxY, boxWidth, boxHeight }, 0.2f, 10, WHITE);
    DrawTextEx(font, "Financial Suggestions", { boxX + 50, boxY + 30 }, 24, 1, DARKGRAY);

    float remaining = tracker.income - tracker.GetTotalExpense() - tracker.monthlySavings;
    vector<string> suggestions;
    int currentY = boxY + 80;

    stringstream ss;
    ss << fixed << setprecision(2);

    if (tracker.income == 0 && tracker.GetTotalExpense() == 0 && tracker.monthlySavings == 0) {
        suggestions.push_back("Welcome! Enter your income and expenses to receive personalized financial suggestions.");
    }
    else if (remaining > tracker.income * 0.3) {
        suggestions.push_back("You have a large surplus! Consider investing a portion or setting aside money for a long-term goal.");
    }
    else if (remaining > 0) {
        suggestions.push_back("Your finances are stable, but review your spending to increase your savings.");
    }
    else {
        suggestions.push_back("You've overspent this month. It's time to re-evaluate your budget and find areas to cut back.");
    }

    if (tracker.monthlySavings == 0 && tracker.income > 0) {
        float suggestedSaving = tracker.income * 0.10f;
        ss.str(""); ss << "You have an income of Rs " << tracker.income << ". Consider saving 10% (Rs " << suggestedSaving << ") this month.";
        string saveSuggestion = ss.str();
        suggestions.push_back(saveSuggestion);
    }
    else if (tracker.monthlySavings > 0) {
        suggestions.push_back("You have already saved for this month!");
    }

    map<string, float> categoryTotals = tracker.GetCategoryTotals();
    for (auto const& pair : tracker.GetBudgets()) {
        string category = pair.first;
        float budgetAmount = pair.second;
        float spentAmount = 0;
        if (categoryTotals.count(category)) {
            spentAmount = categoryTotals.at(category);
        }

        if (spentAmount > budgetAmount) {
            ss.str(""); ss << "You are over budget for " << category << " by Rs " << spentAmount - budgetAmount << ". Try to reduce spending in this area.";
            string overBudgetMsg = ss.str();
            suggestions.push_back(overBudgetMsg);
        }
        else if (spentAmount > budgetAmount * 0.8) {
            ss.str(""); ss << "You are close to your budget for " << category << " (spent Rs " << spentAmount << " of Rs " << budgetAmount << "). Be mindful of your spending.";
            string closeToBudgetMsg = ss.str();
            suggestions.push_back(closeToBudgetMsg);
        }
    }

    for (const auto& suggestion : suggestions) {
        DrawTextEx(font, suggestion.c_str(), { boxX + 50, (float)currentY }, 18, 1, DARKGRAY);
        currentY += 40;
    }

    if (suggestions.empty()) {
        DrawTextEx(font, "No specific suggestions at this time. All budgets are in good standing.", { boxX + 50, (float)currentY }, 18, 1, DARKGRAY);
    }

    Rectangle saveBtn = { boxX + 50, boxY + boxHeight - 60, 200, 40 };
    if (tracker.monthlySavings == 0 && tracker.income > 0) {
        DrawRectangleRounded(saveBtn, 0.3f, 6, GREEN);
        DrawText("Save 10% of Income", saveBtn.x + 10, saveBtn.y + 10, 20, WHITE);
        if (CheckCollisionPointRec(GetMousePosition(), saveBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            currentPage = PAGE_CONFIRM_SAVE;
        }
    }

    Rectangle backBtn = { boxX + boxWidth - 150, boxY + boxHeight - 60, 100, 40 };
    DrawRectangleRounded(backBtn, 0.3f, 6, MAROON);
    DrawText("Back", backBtn.x + 25, backBtn.y + 10, 20, WHITE);
    if (CheckCollisionPointRec(GetMousePosition(), backBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        currentPage = HOME;
    }
}

// Function to handle the confirmation for saving a percentage of income
static void DrawConfirmSavePage(FinanceTracker& tracker, Font font, int screenWidth, int screenHeight) {
    float boxWidth = 500;
    float boxHeight = 200;
    float boxX = screenWidth / 2.0f - boxWidth / 2.0f;
    float boxY = screenHeight / 2.0f - boxHeight / 2.0f;

    DrawRectangleRounded({ boxX, boxY, boxWidth, boxHeight }, 0.2f, 10, WHITE);
    DrawTextEx(font, "Confirm Save", { boxX + 50, boxY + 30 }, 24, 1, DARKGRAY);

    stringstream ss;
    ss << fixed << setprecision(2);
    ss << "Save 10% of your income (Rs " << tracker.income * 0.10f << ")?";
    string msg = ss.str();
    DrawTextEx(font, msg.c_str(), { boxX + 50, boxY + 80 }, 18, 1, DARKGRAY);

    Rectangle yesBtn = { boxX + 50, boxY + 120, 100, 40 };
    Rectangle noBtn = { boxX + boxWidth - 150, boxY + 120, 100, 40 };
    DrawRectangleRounded(yesBtn, 0.3f, 6, GREEN);
    DrawRectangleRounded(noBtn, 0.3f, 6, MAROON);
    DrawText("Yes", yesBtn.x + 30, yesBtn.y + 10, 20, WHITE);
    DrawText("No", noBtn.x + 30, noBtn.y + 10, 20, WHITE);

    if (CheckCollisionPointRec(GetMousePosition(), yesBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        tracker.SaveMoney(tracker.income * 0.10f);
        tracker.Save(user.currentUsername);
        currentPage = HOME;
    }
    if (CheckCollisionPointRec(GetMousePosition(), noBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        currentPage = PAGE_SUGGESTIONS;
    }
}

// Function to handle the confirmation for starting a new month

static void DrawConfirmResetPage(FinanceTracker& tracker, Font font, int screenWidth, int screenHeight) {
    float boxWidth = 500;
    float boxHeight = 200;
    float boxX = screenWidth / 2.0f - boxWidth / 2.0f;
    float boxY = screenHeight / 2.0f - boxHeight / 2.0f;

    DrawRectangleRounded({ boxX, boxY, boxWidth, boxHeight }, 0.2f, 10, WHITE);
    DrawTextEx(font, "Confirm Reset", { boxX + 50, boxY + 30 }, 24, 1, DARKGRAY);

    string msg = "Are you sure you want to reset for a new month?";
    DrawTextEx(font, msg.c_str(), { boxX + 50, boxY + 80 }, 18, 1, DARKGRAY);

    Rectangle confirmBtn = { boxX + 50, boxY + 120, 150, 40 };
    Rectangle cancelBtn = { boxX + boxWidth - 175, boxY + 120, 150, 40 };

    DrawRectangleRounded(confirmBtn, 0.3f, 6, GREEN);
    DrawRectangleRounded(cancelBtn, 0.3f, 6, MAROON);

    DrawText("Confirm Reset", confirmBtn.x + 10, confirmBtn.y + 10, 18, WHITE);
    DrawText("Cancel", cancelBtn.x + 40, cancelBtn.y + 10, 18, WHITE);

    // Check for button clicks
    if (CheckCollisionPointRec(GetMousePosition(), confirmBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // Reset for the new month without affecting savings.
        tracker.ResetForNewMonth();
        tracker.Save(user.currentUsername);
        currentPage = HOME;
    }

    if (CheckCollisionPointRec(GetMousePosition(), cancelBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // Cancel the operation and go back to the home page.
        currentPage = HOME;
    }
}


// Function to draw the analytics page
static void DrawAnalyticsPage(FinanceTracker& tracker, Font font, int screenWidth, int screenHeight) {
    DrawTextEx(font, "Expense Analytics", { (float)screenWidth / 2 - MeasureTextEx(font, "Expense Analytics", 24, 1).x / 2.0f, 100 }, 24, 1, DARKGRAY);

    DrawPieChart(tracker.GetCategoryTotals(), screenWidth - 250, screenHeight / 2, 150);

    map<string, float> categoryTotals = tracker.GetCategoryTotals();
    const map<string, float>& budgets = tracker.GetBudgets();
    int startX = 50;
    int startY = 150;
    int spacingY = 20;
    

    DrawTextEx(font, "Category", { (float)startX, (float)startY }, 20, 1, DARKGRAY);
    DrawTextEx(font, "Budget", { (float)startX + 150, (float)startY }, 20, 1, DARKGRAY);
    DrawTextEx(font, "Spent", { (float)startX + 300, (float)startY }, 20, 1, DARKGRAY);
    DrawTextEx(font, "Status", { (float)startX + 450, (float)startY }, 20, 1, DARKGRAY);

    startY += spacingY;

    stringstream ss;
    ss << fixed << setprecision(2);

    for (const string& cat : categories) {
        float budgetAmount = 0.0f;
        if (budgets.count(cat)) {
            budgetAmount = budgets.at(cat);
        }
        float spentAmount = 0.0f;
        if (categoryTotals.count(cat)) {
            spentAmount = categoryTotals.at(cat);
        }

        Color statusColor = GRAY;
        string statusText;

        ss.str("");

        if (budgetAmount > 0) {
            float remaining = budgetAmount - spentAmount;
            if (remaining > 0) {
                statusColor = DARKGREEN;
                ss << "Rs " << remaining << " left";
                statusText = ss.str();
            }
            else {
                statusColor = RED;
                ss << "Over by Rs " << -remaining;
                statusText = ss.str();
            }
        }
        else {
            statusText = "No budget set";
        }

        ss.str(""); ss << "Rs " << budgetAmount;
        string budgetText = ss.str();

        ss.str(""); ss << "Rs " << spentAmount;
        string spentText = ss.str();

        DrawTextEx(font, cat.c_str(), { (float)startX, (float)startY }, 18, 1, BLACK);
        DrawTextEx(font, budgetText.c_str(), { (float)startX + 150, (float)startY }, 18, 1, DARKGRAY);
        DrawTextEx(font, spentText.c_str(), { (float)startX + 300, (float)startY }, 18, 1, DARKGRAY);
        DrawTextEx(font, statusText.c_str(), { (float)startX + 450, (float)startY }, 18, 1, statusColor);

        startY += spacingY;
    }

    Rectangle backBtn = { (float)screenWidth / 2 - 50, screenHeight - 60, 100, 40 };
    DrawRectangleRounded(backBtn, 0.3f, 6, MAROON);
    DrawText("Back", backBtn.x + 25, backBtn.y + 10, 20, WHITE);
    if (CheckCollisionPointRec(GetMousePosition(), backBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        currentPage = HOME;
    }
}

// Function to draw the set budget page
static void DrawSetBudgetPage(FinanceTracker& tracker, Font font, int screenWidth, int screenHeight) {
    float boxWidth = 500;
    float boxHeight = 400;
    float boxX = screenWidth / 2.0f - boxWidth / 2.0f;
    float boxY = screenHeight / 2.0f - boxHeight / 2.0f;

    DrawRectangleRounded({ boxX, boxY, boxWidth, boxHeight }, 0.2f, 10, WHITE);
    DrawTextEx(font, "Set Budgets", { boxX + 50, boxY + 30 }, 24, 1, DARKGRAY);

    DrawBudgetInputs(boxX, boxY, font, categories);

    Rectangle saveBtn = { boxX + 50, boxY + boxHeight - 60, 100, 40 };
    Rectangle backBtn = { boxX + boxWidth - 150, boxY + boxHeight - 60, 100, 40 };
    DrawRectangleRounded(saveBtn, 0.3f, 6, GREEN);
    DrawRectangleRounded(backBtn, 0.3f, 6, MAROON);
    DrawText("Save", saveBtn.x + 25, saveBtn.y + 10, 20, WHITE);
    DrawText("Back", backBtn.x + 25, backBtn.y + 10, 20, WHITE);

    if (CheckCollisionPointRec(GetMousePosition(), saveBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        bool allSaved = true;
        for (const auto& pair : budgetInputText) {
            try {
                if (budgetCategorySelected[pair.first]) {
                    tracker.SetBudget(pair.first, stof(pair.second));
                }
            }
            catch (...) {
                message = "Error in budget amount for " + pair.first;
                allSaved = false;
            }
        }
        if (allSaved) {
            tracker.Save(user.currentUsername);
            currentPage = HOME;
        }
    }
    if (CheckCollisionPointRec(GetMousePosition(), backBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        currentPage = HOME;
    }
    if (!message.empty()) {
        ShowMessage(message, boxX + 50, boxY + boxHeight - 100, RED);
    }
}

// Function to draw the add expense page
static void DrawAddExpensePage(FinanceTracker& tracker, Font font, int screenWidth, int screenHeight) {
    float boxWidth = 500;
    float boxHeight = 400;
    float boxX = screenWidth / 2.0f - boxWidth / 2.0f;
    float boxY = screenHeight / 2.0f - boxHeight / 2.0f;

    DrawRectangleRounded({ boxX, boxY, boxWidth, boxHeight }, 0.2f, 10, WHITE);
    DrawTextEx(font, "Add Expenses", { boxX + 50, boxY + 30 }, 24, 1, DARKGRAY);

    DrawExpenseInputs(boxX, boxY, font, categories);

    Rectangle saveBtn = { boxX + 50, boxY + boxHeight - 60, 100, 40 };
    Rectangle backBtn = { boxX + boxWidth - 150, boxY + boxHeight - 60, 100, 40 };
    DrawRectangleRounded(saveBtn, 0.3f, 6, GREEN);
    DrawRectangleRounded(backBtn, 0.3f, 6, MAROON);
    DrawText("Save", saveBtn.x + 25, saveBtn.y + 10, 20, WHITE);
    DrawText("Back", backBtn.x + 25, backBtn.y + 10, 20, WHITE);

    if (CheckCollisionPointRec(GetMousePosition(), saveBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        bool allSaved = true;
        for (const auto& pair : expenseInputText) {
            try {
                if (expenseCategorySelected[pair.first]) {
                    tracker.AddExpense(pair.first, stof(pair.second));
                }
            }
            catch (...) {
                message = "Error in expense amount for " + pair.first;
                allSaved = false;
            }
        }
        if (allSaved) {
            tracker.Save(user.currentUsername);
            currentPage = HOME;
        }
    }
    if (CheckCollisionPointRec(GetMousePosition(), backBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        currentPage = HOME;
    }
    if (!message.empty()) {
        ShowMessage(message, boxX + 50, boxY + boxHeight - 100, RED);
    }
}



// Function to handle the confirmation for logging out
static void DrawLogoutConfirmPage(Font font, int screenWidth, int screenHeight) {
    float boxWidth = 500;
    float boxHeight = 200;
    float boxX = screenWidth / 2.0f - boxWidth / 2.0f;
    float boxY = screenHeight / 2.0f - boxHeight / 2.0f;

    DrawRectangleRounded({ boxX, boxY, boxWidth, boxHeight }, 0.2f, 10, WHITE);
    DrawTextEx(font, "Confirm Logout", { boxX + 50, boxY + 30 }, 24, 1, DARKGRAY);

    string msg = "Are you sure you want to log out?";
    DrawTextEx(font, msg.c_str(), { boxX + 50, boxY + 80 }, 18, 1, DARKGRAY);

    Rectangle yesBtn = { boxX + 50, boxY + 120, 100, 40 };
    Rectangle noBtn = { boxX + boxWidth - 150, boxY + 120, 100, 40 };
    DrawRectangleRounded(yesBtn, 0.3f, 6, GREEN);
    DrawRectangleRounded(noBtn, 0.3f, 6, MAROON);
    DrawText("Yes", yesBtn.x + 30, yesBtn.y + 10, 20, WHITE);
    DrawText("No", noBtn.x + 30, noBtn.y + 10, 20, WHITE);

    if (CheckCollisionPointRec(GetMousePosition(), yesBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        currentPage = LOGIN;
    }
    if (CheckCollisionPointRec(GetMousePosition(), noBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        currentPage = HOME;
    }
}

// Main application loop function
static void MainApplicationLoop() {
    bool loggedIn = false;
    const int screenWidth = 1050;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Finance Tracker GUI");
    SetTargetFPS(60);
    customFont = LoadFont("OpenSans_Condensed-Bold.ttf");

    float buttonStartX = 30;
    float buttonStartY = 150;
    float buttonWidth = 180;
    float buttonHeight = 40;
    float verticalSpacing = 15;

    vector<Button> buttons = {
        { {buttonStartX, buttonStartY + 0 * (buttonHeight + verticalSpacing), buttonWidth, buttonHeight}, "Add Income" },
        { {buttonStartX, buttonStartY + 1 * (buttonHeight + verticalSpacing), buttonWidth, buttonHeight}, "Add Expense" },
        { {buttonStartX, buttonStartY + 2 * (buttonHeight + verticalSpacing), buttonWidth, buttonHeight}, "Set Budget" },
        { {buttonStartX, buttonStartY + 3 * (buttonHeight + verticalSpacing), buttonWidth, buttonHeight}, "View Analytics" },
        { {buttonStartX, buttonStartY + 4 * (buttonHeight + verticalSpacing), buttonWidth, buttonHeight}, "View Suggestions" },
        { {buttonStartX, buttonStartY + 5 * (buttonHeight + verticalSpacing), buttonWidth, buttonHeight}, "Start New Month" }
    };
    Button logoutBtn = { { (float)screenWidth - 150, 20, 120, 40 }, "Logout" };

    while (!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();
        BeginDrawing();
        ClearBackground(BG);

        int boxW = 500, boxH = 400;
        int boxX = screenWidth / 2 - boxW / 2;
        int boxY = screenHeight / 2 - boxH / 2;

        switch (currentPage) {
        case LOGIN: {
            DrawLoginScreen(loggedIn);
            break;
        }
        case HOME: {
            DrawLiveSummaryBox(tracker, customFont, screenWidth);
            DrawTextEx(customFont, "Budgeting App", { screenWidth / 2.0f - MeasureTextEx(customFont, "Budgeting App", 32, 1).x / 2.0f, 70 }, 32, 1, DARKGRAY);

            for (auto& btn : buttons) {
                bool hover = btn.IsHovered(mouse);
                btn.Draw(hover, customFont);

                if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    inputText = "";
                    selectedExpenseCategoryInput = "";
                    selectedBudgetCategoryInput = "";
                    message = "";

                    if (btn.label == "Add Income") currentPage = PAGE_ADD_INCOME;
                    else if (btn.label == "Add Expense") currentPage = PAGE_ADD_EXPENSE;
                    else if (btn.label == "Set Budget") currentPage = PAGE_SET_BUDGET;
                    else if (btn.label == "View Analytics") currentPage = PAGE_ANALYTICS;
                    else if (btn.label == "View Suggestions") currentPage = PAGE_SUGGESTIONS;
                    else if (btn.label == "Start New Month") currentPage = PAGE_CONFIRM_RESET;
                }
            }

            bool logoutHover = logoutBtn.IsHovered(mouse);
            logoutBtn.Draw(logoutHover, customFont);

            if (logoutHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                currentPage = PAGE_LOGOUT_CONFIRM;
            }
            break;
        }

        case PAGE_ADD_INCOME: {
            DrawRectangleRounded({ (float)boxX, (float)boxY, (float)boxW, (float)boxH }, 0.2f, 10, WHITE);
            DrawTextEx(customFont, "Enter Monthly Income", { (float)boxX + 50, (float)boxY + 30 }, 24, 1, DARKGRAY);

            Rectangle inputRect = { (float)boxX + 50, (float)boxY + 100, (float)boxW - 100, 50 };
            DrawInputBox("Amount:", inputRect, customFont, inputText);

            Rectangle saveBtn = { (float)boxX + 50, (float)boxY + 200, 100, 40 };
            Rectangle backBtn = { (float)boxX + boxW - 150, (float)boxY + 200, 100, 40 };
            DrawRectangleRounded(saveBtn, 0.3f, 6, GREEN);
            DrawRectangleRounded(backBtn, 0.3f, 6, MAROON);
            DrawText("Save", saveBtn.x + 25, saveBtn.y + 10, 20, WHITE);
            DrawText("Back", backBtn.x + 25, backBtn.y + 10, 20, WHITE);

            if (CheckCollisionPointRec(mouse, saveBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (!inputText.empty()) {
                    try {
                        tracker.AddIncome(stof(inputText));
                        tracker.Save(user.currentUsername);
                        tracker.Load(user.currentUsername);
                        inputText = "";
                        currentPage = HOME;
                    }
                    catch (...) {
                        message = "Invalid number format.";
                    }
                }
                else {
                    message = "Please enter an amount.";
                }
            }
            if (CheckCollisionPointRec(mouse, backBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                inputText = "";
                message = "";
                currentPage = HOME;
            }
            if (!message.empty()) {
                ShowMessage(message, boxX + 50, boxY + 250, RED);
            }
            break;
        }

        case PAGE_ADD_EXPENSE: {
            DrawAddExpensePage(tracker, customFont, screenWidth, screenHeight);
            break;
        }

        case PAGE_SET_BUDGET: {
            DrawSetBudgetPage(tracker, customFont, screenWidth, screenHeight);
            break;
        }
        case PAGE_ANALYTICS: {
            DrawAnalyticsPage(tracker, customFont, screenWidth, screenHeight);
            break;
        }
        case PAGE_SUGGESTIONS: {
            DrawSuggestionsPage(tracker, customFont, screenWidth, screenHeight);
            break;
        }
        case PAGE_CONFIRM_SAVE: {
            DrawConfirmSavePage(tracker, customFont, screenWidth, screenHeight);
            break;
        }     
        case PAGE_CONFIRM_RESET: {
            DrawConfirmResetPage(tracker, customFont, screenWidth, screenHeight);
            break;
        }
        case PAGE_LOGOUT_CONFIRM: {
            DrawLogoutConfirmPage(customFont, screenWidth, screenHeight);
            break;
        }
        }
        EndDrawing();
    }
    UnloadFont(customFont);
    CloseWindow();
}

// =============================================================================
// ENTRY POINT
// =============================================================================
int main() {
    MainApplicationLoop();
    return 0;
}