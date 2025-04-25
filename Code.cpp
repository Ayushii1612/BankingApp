#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>
#include <map>
#include <vector>
#include <queue>
#include <algorithm>
#include <cmath>
#include <limits>
#include <ctime>
using namespace std;

// Structure for storing transaction details
struct Transaction {
    string type;
    double amount;
    string timestamp;
    Transaction* next;

    Transaction(string t, double amt, string time) : type(t), amount(amt), timestamp(time), next(nullptr) {}
};

// Stack class for managing transaction history
class TransactionStack {
private:
    Transaction* top;

public:
    TransactionStack() : top(nullptr) {}

    ~TransactionStack() {
        while (top) {
            Transaction* temp = top;
            top = top->next;
            delete temp;
        }
    }

    void push(string type, double amount, string timestamp) {
        Transaction* newTransaction = new Transaction(type, amount, timestamp);
        newTransaction->next = top;
        top = newTransaction;
    }

    void displayHistory() {
        Transaction* temp = top;
        while (temp) {
            cout << temp->timestamp << " - " << temp->type << " " << fixed << setprecision(2) << temp->amount << endl;
            temp = temp->next;
        }
    }

    void saveToFile(ofstream& file) {
        Transaction* temp = top;
        while (temp) {
            file << temp->timestamp << "," << temp->type << "," << temp->amount << endl;
            temp = temp->next;
        }
    }
};

// Structure for storing account details
struct Account {
    int accountNumber;
    string accountHolderName;
    double balance;
    string pin;  // PIN for authentication
    TransactionStack history;
    Account* left;
    Account* right;
    int height;

    Account(int accNum, string name, double initialBalance, string pin)
        : accountNumber(accNum), accountHolderName(name), balance(initialBalance), pin(pin),
          left(nullptr), right(nullptr), height(1) {}
};

// AVL Tree class for managing accounts
class AccountAVLTree {
private:
    Account* root;

    // Utility functions for AVL Tree operations
    int getHeight(Account* node) {
        return node ? node->height : 0;
    }

    int getBalanceFactor(Account* node) {
        return node ? getHeight(node->left) - getHeight(node->right) : 0;
    }

    Account* rotateRight(Account* y) {
        Account* x = y->left;
        Account* T2 = x->right;

        x->right = y;
        y->left = T2;

        y->height = max(getHeight(y->left), getHeight(y->right)) + 1;
        x->height = max(getHeight(x->left), getHeight(x->right)) + 1;

        return x;
    }

    Account* rotateLeft(Account* x) {
        Account* y = x->right;
        Account* T2 = y->left;

        y->left = x;
        x->right = T2;

        x->height = max(getHeight(x->left), getHeight(x->right)) + 1;
        y->height = max(getHeight(y->left), getHeight(y->right)) + 1;

        return y;
    }

    Account* insert(Account* node, int accNum, string name, double initialBalance, string pin) {
        if (!node)
            return new Account(accNum, name, initialBalance, pin);

        if (accNum < node->accountNumber)
            node->left = insert(node->left, accNum, name, initialBalance, pin);
        else if (accNum > node->accountNumber)
            node->right = insert(node->right, accNum, name, initialBalance, pin);
        else
            return node;  // Duplicate accounts are not allowed

        node->height = max(getHeight(node->left), getHeight(node->right)) + 1;

        int balanceFactor = getBalanceFactor(node);

        // Balance the tree
        if (balanceFactor > 1 && accNum < node->left->accountNumber)
            return rotateRight(node);
        if (balanceFactor < -1 && accNum > node->right->accountNumber)
            return rotateLeft(node);
        if (balanceFactor > 1 && accNum > node->left->accountNumber) {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }
        if (balanceFactor < -1 && accNum < node->right->accountNumber) {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }

        return node;
    }

    Account* findMin(Account* node) {
        while (node->left)
            node = node->left;
        return node;
    }

    Account* deleteNode(Account* root, int accNum) {
        if (!root)
            return root;

        if (accNum < root->accountNumber)
            root->left = deleteNode(root->left, accNum);
        else if (accNum > root->accountNumber)
            root->right = deleteNode(root->right, accNum);
        else {
            if (!root->left || !root->right) {
                Account* temp = root->left ? root->left : root->right;
                if (!temp) {
                    temp = root;
                    root = nullptr;
                } else
                    *root = *temp;
                delete temp;
            } else {
                Account* temp = findMin(root->right);
                root->accountNumber = temp->accountNumber;
                root->accountHolderName = temp->accountHolderName;
                root->balance = temp->balance;
                root->history = temp->history;
                root->right = deleteNode(root->right, temp->accountNumber);
            }
        }

        if (!root)
            return root;

        root->height = max(getHeight(root->left), getHeight(root->right)) + 1;

        int balanceFactor = getBalanceFactor(root);

        // Balance the tree
        if (balanceFactor > 1 && getBalanceFactor(root->left) >= 0)
            return rotateRight(root);
        if (balanceFactor > 1 && getBalanceFactor(root->left) < 0) {
            root->left = rotateLeft(root->left);
            return rotateRight(root);
        }
        if (balanceFactor < -1 && getBalanceFactor(root->right) <= 0)
            return rotateLeft(root);
        if (balanceFactor < -1 && getBalanceFactor(root->right) > 0) {
            root->right = rotateRight(root->right);
            return rotateLeft(root);
        }

        return root;
    }

    void inOrderTraversal(Account* node) {
        if (node) {
            inOrderTraversal(node->left);
            cout << node->accountNumber << "\t" << node->accountHolderName << "\t" << fixed << setprecision(2) << node->balance << endl;
            inOrderTraversal(node->right);
        }
    }

    Account* search(Account* node, int accNum) {
        if (!node || node->accountNumber == accNum)
            return node;

        if (accNum < node->accountNumber)
            return search(node->left, accNum);
        else
            return search(node->right, accNum);
    }

    double calculateTotalFunds(Account* node) {
        if (!node)
            return 0.0;
        return node->balance + calculateTotalFunds(node->left) + calculateTotalFunds(node->right);
    }

    void saveToFileHelper(Account* node, ofstream& file) {
        if (node) {
            saveToFileHelper(node->left, file);
            file << node->accountNumber << "," << node->accountHolderName << "," << node->balance << "," << node->pin << endl;
            node->history.saveToFile(file);
            saveToFileHelper(node->right, file);
        }
    }

    void loadFromFileHelper(ifstream& file) {
        int accNum;
        string name, pin;
        double balance;
        while (file >> accNum >> name >> balance >> pin) {
            root = insert(root, accNum, name, balance, pin);
        }
    }

public:
    AccountAVLTree() : root(nullptr) {}

    void createAccount(int accNum, string name, double initialBalance, string pin) {
        root = insert(root, accNum, name, initialBalance, pin);
        cout << "Account created successfully!" << endl;
    }

    void deleteAccount(int accNum) {
        root = deleteNode(root, accNum);
        cout << "Account deleted successfully!" << endl;
    }

    void displayAccounts() {
        cout << "Account Number\tName\t\tBalance" << endl;
        cout << "----------------------------------------" << endl;
        inOrderTraversal(root);
    }

    Account* searchAccount(int accNum) {
        return search(root, accNum);
    }

    double getTotalFunds() {
        return calculateTotalFunds(root);
    }

    void saveToFile(string filename) {
        ofstream file(filename);
        if (file.is_open()) {
            saveToFileHelper(root, file);
            file.close();
            cout << "Accounts saved to file successfully!" << endl;
        } else {
            cout << "Unable to open file for saving!" << endl;
        }
    }

    void loadFromFile(string filename) {
        ifstream file(filename);
        if (file.is_open()) {
            loadFromFileHelper(file);
            file.close();
            cout << "Accounts loaded from file successfully!" << endl;
        } else {
            cout << "Unable to open file for loading!" << endl;
        }
    }

    void applyInterest(double rate) {
        // Apply interest to all accounts
        queue<Account*> q;
        q.push(root);
        while (!q.empty()) {
            Account* current = q.front();
            q.pop();
            if (current) {
                current->balance *= (1 + rate / 100);
                current->history.push("Interest", current->balance * rate / 100, getCurrentTimestamp());
                q.push(current->left);
                q.push(current->right);
            }
        }
        cout << "Interest applied successfully!" << endl;
    }

    string getCurrentTimestamp() {
        time_t now = time(0);
        char buf[80];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
        return string(buf);
    }
};

// Main function
int main() {
    AccountAVLTree accounts;
    int choice;

    // Load accounts from file (if any)
    accounts.loadFromFile("accounts.txt");

    do {
        cout << "\nBanking System Menu:" << endl;
        cout << "1. Create Account" << endl;
        cout << "2. Deposit" << endl;
        cout << "3. Withdraw" << endl;
        cout << "4. Transfer" << endl;
        cout << "5. View All Accounts" << endl;
        cout << "6. Manager Options" << endl;
        cout << "7. Exit" << endl;
        cout << "Enter your choice: ";

        // Input validation
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input! Please enter a number between 1 and 7." << endl;
            continue;
        }

        switch (choice) {
            case 1: {
                int accNum;
                string name, pin;
                double balance;
                cout << "Enter account number: ";
                cin >> accNum;
                cout << "Enter account holder name: ";
                cin.ignore();
                getline(cin, name);
                cout << "Enter initial balance: ";
                cin >> balance;
                cout << "Set a 4-digit PIN: ";
                cin >> pin;
                accounts.createAccount(accNum, name, balance, pin);
                break;
            }
            case 2: {
                int accNum;
                string pin;
                double amount;
                cout << "Enter account number: ";
                cin >> accNum;
                Account* account = accounts.searchAccount(accNum);
                if (account) {
                    cout << "Enter your PIN: ";
                    cin >> pin;
                    if (pin == account->pin) {
                        cout << "Enter amount to deposit: ";
                        cin >> amount;
                        account->balance += amount;
                        account->history.push("Deposit", amount, accounts.getCurrentTimestamp());
                        cout << "Deposited successfully!" << endl;
                    } else {
                        cout << "Invalid PIN!" << endl;
                    }
                } else {
                    cout << "Account not found!" << endl;
                }
                break;
            }
            case 3: {
                int accNum;
                string pin;
                double amount;
                cout << "Enter account number: ";
                cin >> accNum;
                Account* account = accounts.searchAccount(accNum);
                if (account) {
                    cout << "Enter your PIN: ";
                    cin >> pin;
                    if (pin == account->pin) {
                        cout << "Enter amount to withdraw: ";
                        cin >> amount;
                        if (amount <= account->balance) {
                            account->balance -= amount;
                            account->history.push("Withdraw", amount, accounts.getCurrentTimestamp());
                            cout << "Withdrawn successfully!" << endl;
                        } else {
                            cout << "Insufficient balance!" << endl;
                        }
                    } else {
                        cout << "Invalid PIN!" << endl;
                    }
                } else {
                    cout << "Account not found!" << endl;
                }
                break;
            }
            case 4: {
                int fromAccNum, toAccNum;
                string pin;
                double amount;
                cout << "Enter sender's account number: ";
                cin >> fromAccNum;
                cout << "Enter receiver's account number: ";
                cin >> toAccNum;
                cout << "Enter amount to transfer: ";
                cin >> amount;

                Account* fromAccount = accounts.searchAccount(fromAccNum);
                Account* toAccount = accounts.searchAccount(toAccNum);

                if (fromAccount && toAccount) {
                    cout << "Enter your PIN: ";
                    cin >> pin;
                    if (pin == fromAccount->pin) {
                        if (amount <= fromAccount->balance) {
                            fromAccount->balance -= amount;
                            fromAccount->history.push("Transfer Out", amount, accounts.getCurrentTimestamp());

                            toAccount->balance += amount;
                            toAccount->history.push("Transfer In", amount, accounts.getCurrentTimestamp());

                            cout << "Transfer successful!" << endl;
                        } else {
                            cout << "Insufficient balance in sender's account!" << endl;
                        }
                    } else {
                        cout << "Invalid PIN!" << endl;
                    }
                } else {
                    cout << "One or both accounts not found!" << endl;
                }
                break;
            }
            case 5: {
                accounts.displayAccounts();
                break;
            }
            case 6: {
                int managerChoice;
                cout << "\nManager Options:" << endl;
                cout << "1. View Total Funds" << endl;
                cout << "2. View All Accounts" << endl;
                cout << "3. Apply Interest" << endl;
                cout << "4. Save Accounts to File" << endl;
                cout << "Enter your choice: ";
                cin >> managerChoice;

                switch (managerChoice) {
                    case 1: {
                        cout << "Total funds in the bank: Rs. " << fixed << setprecision(2) << accounts.getTotalFunds() << endl;
                        break;
                    }
                    case 2: {
                        accounts.displayAccounts();
                        break;
                    }
                    case 3: {
                        double rate;
                        cout << "Enter interest rate (%): ";
                        cin >> rate;
                        accounts.applyInterest(rate);
                        break;
                    }
                    case 4: {
                        accounts.saveToFile("accounts.txt");
                        break;
                    }
                    default: {
                        cout << "Invalid choice!" << endl;
                        break;
                    }
                }
                break;
            }
            case 7: {
                accounts.saveToFile("accounts.txt");
                cout << "Exiting... Thank you!" << endl;
                break;
            }
            default: {
                cout << "Invalid choice! Please try again." << endl;
                break;
            }
        }
    } while (choice != 7);

    return 0;
}