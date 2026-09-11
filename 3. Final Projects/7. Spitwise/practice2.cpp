#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include <utility>
#include <cmath>
#include <string>
using namespace std;

enum class SplitType
{
    EXACT = 0,
    EQUAL,
    PERCENTAGE
};

class User
{

    string id;
    string name;

public:
    User(const string &id, const string &name) : id(id), name(name) {}
    const string &getId() const
    {
        return id;
    }
    const string &getName() const
    {
        return name;
    }
};

class Split
{
    string userId;
    double amount;

public:
    Split(const string &userId, double amount) : userId(userId), amount(amount) {}
    const string &getUserId() const
    {
        return userId;
    }
    const double getAmount() const
    {
        return amount;
    }
};

class SplitStrategy
{
public:
    virtual ~SplitStrategy() = default;
    virtual vector<Split> calculateSplits(double amount, const vector<string> &participants, const vector<double> &values = {}) = 0;
};

class EqualSplitStrategy : public SplitStrategy
{
public:
    vector<Split> calculateSplits(double amount, const vector<string> &participants, const vector<double> &values = {}) override
    {
        if (participants.size() == 0)
        {
            throw runtime_error("there must be participants in the expense.\n");
        }
        double perPersonAmount = amount / participants.size();
        vector<Split> splits;
        for (int i = 0; i < participants.size(); i++)
        {
            splits.push_back(Split(participants[i], perPersonAmount));
        }

        return splits;
    }
};

class ExactSplitStrategy : public SplitStrategy
{
public:
    vector<Split> calculateSplits(double amount, const vector<string> &participants, const vector<double> &values = {}) override
    {
        if (participants.size() != values.size() or values.size() == 0)
        {
            throw runtime_error("there must be participants in the expense.\n");
        }
        double sum = 0;
        for (auto &x : values)
        {
            sum += x;
        }
        if (fabs(sum - amount) >= 0.01)
        {
            throw runtime_error("sum of values must be equal to total expense amount.\n");
        }
        vector<Split> splits;
        for (int i = 0; i < participants.size(); i++)
        {
            splits.push_back(Split(participants[i], values[i]));
        }

        return splits;
    }
};

class PercentageSplitStrategy : public SplitStrategy
{
public:
    vector<Split> calculateSplits(double amount, const vector<string> &participants, const vector<double> &values = {}) override
    {
        if (participants.size() != values.size() or values.size() == 0)
        {
            throw runtime_error("there must be participants in the expense.\n");
        }
        double sum = 0;
        for (auto &x : values)
        {
            sum += x;
        }
        if (fabs(sum - 100.0) >= 0.01)
        {
            throw runtime_error("sum of values must be equal to total expense amount.\n");
        }
        vector<Split> splits;
        for (int i = 0; i < participants.size(); i++)
        {
            double amount = amount * values[i] / 100.0;
            splits.push_back(Split(participants[i], amount));
        }

        return splits;
    }
};

class Expense
{
    string id;
    string description;

    string paidBy;
    double amount;

    vector<string> participants;
    vector<Split> splits;

    SplitType type;

public:
    Expense(
        const string &id,
        const string &description,
        const string &paidBy,
        double amount,
        SplitType type,
        const vector<string> &participants,
        const vector<Split> &splits) : id(id), description(description), paidBy(paidBy), amount(amount), type(type), participants(participants), splits(splits)
    {
    }

    const string &getId() const
    {
        return id;
    }

    const string &getPaidBy() const
    {
        return paidBy;
    }
    double getAmount() const
    {
        return amount;
    }
    const vector<string> &getParticipants() const
    {
        return participants;
    }
};

class Group
{
    string id;
    string name;
    vector<string> users;
    vector<string> expenses;

public:
    Group(const string &id, const string &name) : id(id), name(name) {}
    const string &getId() const
    {
        return id;
    }
    const string &getName() const
    {
        return name;
    }

    void addUser(const string &userId)
    {
        auto it = find(users.begin(), users.end(), userId);
        if (it == users.end())
        {
            users.push_back(id);
        }
    }
    void addExpense(const string &expenseId)
    {
        auto it = find(expenses.begin(), expenses.end(), expenseId);
        if (it == expenses.end())
        {
            expenses.push_back(id);
        }
    }

    const vector<string> &getUsers() const
    {
        return users;
    }

    const vector<string> &getExpenses() const
    {
        return expenses;
    }
};

class SplitWise
{
    unordered_map<string, User> users;
    unordered_map<string, Expense> expenses;
    unordered_map<string, Group> groups;

    unordered_map<string, unordered_map<string, double>> balances;
    int expenseCounter = 1;

    void addBalance(const string &debtor, const string &creditor, double amount)
    {
        if (creditor == debtor)
            return;
        double oppBal = balances[creditor][debtor];
        if (oppBal >= amount)
        {
            balances[creditor][debtor] -= amount;
        }
        else
        {
            balances[debtor][creditor] = amount - oppBal;
        }
    }
    unique_ptr<SplitStrategy> createStrategy(SplitType type)
    {
        switch (type)
        {
        case SplitType::EQUAL:
            return make_unique<EqualSplitStrategy>();
        case SplitType::EXACT:
            return make_unique<ExactSplitStrategy>();
        case SplitType::PERCENTAGE:
            return make_unique<PercentageSplitStrategy>();
        default:
            throw runtime_error("Unknown split type.\n");
        }
    }

public:
    void addUser(const string &id, const string &name)
    {
        users.emplace(id, User(id, name));
    }

    void createGroup(const string &id, const string &name)
    {
        groups.emplace(id, Group(id, name));
    }

    void addUserToGroup(const string &userId, const string &groupId)
    {
        // can be implemented easily
    }

    string addExpense(
        const string &description,
        double amount,
        const string &paidBy,
        SplitType type,
        const vector<string> &participants,
        const vector<double> &values = {})
    {
        // should apply checks if valid participants and amount >0

        unique_ptr<SplitStrategy> strategy = createStrategy(type);
        auto splits = strategy->calculateSplits(amount, participants, values);

        const string expenseId = "expense_id_" + to_string(expenseCounter++);
        Expense expense = Expense(expenseId, description, paidBy, amount, type, participants, splits);

        expenses.emplace(expenseId, move(expense));

        for (auto &split : splits)
        {
            if (split.getAmount() > 0.01)
            {
                if (split.getUserId() == paidBy)
                    continue;
                addBalance(split.getUserId(), paidBy, split.getAmount());
            }
        }

        return expenseId;
    }

    void showBalances() const
    {
        cout << "\n===== BALANCES =====\n";
        bool found = false;

        for (auto &entry : balances)
        {
            const string &debtor = entry.first;
            for (auto &credit : entry.second)
            {
                const string &creditor = credit.first;
                double amount = credit.second;
                if (amount < 0.01)
                    continue;

                cout
                    << users.at(debtor).getName()
                    << " owes "
                    << users.at(creditor).getName()
                    << " : Rs."
                    << amount
                    << '\n';
                found = true;
            }
        }

        if (!found)
        {
            cout << "No balances\n";
        }
    }
};

int main()
{
    SplitWise splitwise;

    // ---------------- USERS ----------------

    splitwise.addUser(
        "U1",
        "Aman");

    splitwise.addUser(
        "U2",
        "Rahul");

    splitwise.addUser(
        "U3",
        "Karan");

    splitwise.addUser(
        "U4",
        "Rohit");

    // ---------------- GROUP ----------------

    splitwise.createGroup(
        "G1",
        "Goa Trip");

    splitwise.addUserToGroup(
        "G1",
        "U1");

    splitwise.addUserToGroup(
        "G1",
        "U2");

    splitwise.addUserToGroup(
        "G1",
        "U3");

    splitwise.addUserToGroup(
        "G1",
        "U4");

    splitwise.addExpense(
        "Dinner",
        1000.0,
        "U1",
        SplitType::EQUAL,
        {"U1", "U2", "U3", "U4"});

    splitwise.showBalances();

    return 0;
}