#include <iostream>
#include <vector>
#include <cmath>
#include <unordered_map>
#include <memory>
#include <utility>
#include <algorithm>
using namespace std;

enum class SplitType
{
    EXACT = 0,
    EQUAL,
    PERCENTAGE
};

class User
{
    string id, name;

public:
    User(const string &id, const string &name) : name(name), id(id) {}

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

    void setAmount(double a)
    {
        amount = a;
    }
};

class SplitStrategy
{
public:
    virtual vector<Split> calculateSplits(const double totalAmount, const vector<string> &usersIds, const vector<double> &values = {}) = 0;
    virtual ~SplitStrategy() = default;
};

class EqualSplitStrategy : public SplitStrategy
{
public:
    vector<Split> calculateSplits(const double totalAmount, const vector<string> &userIds, const vector<double> &values = {}) override
    {
        int users = userIds.size();
        if (users == 0)
        {
            throw runtime_error("No users");
        }

        double amountPerUser = totalAmount / users;
        vector<Split> splits;
        for (auto &userId : userIds)
        {
            splits.push_back(Split(userId, amountPerUser));
        }
        return splits;
    }
};

class ExactSplitStrategy : public SplitStrategy
{
public:
    vector<Split> calculateSplits(const double totalAmount, const vector<string> &userIds, const vector<double> &values) override
    {
        int users = userIds.size();
        if (users != values.size())
        {
            throw runtime_error("exact amount for each users needed");
        }

        double sum = 0.0;
        for (auto val : values)
        {
            sum += val;
        }
        if (fabs(sum - totalAmount) > 0.01)
        {
            throw runtime_error("sum of values must be equal to total amount.\n");
        }

        vector<Split> splits;
        for (int i = 0; i < users; i++)
        {
            splits.push_back(Split(userIds[i], values[i]));
        }
        return splits;
    }
};

class PercentageSplitStrategy : public SplitStrategy
{
public:
    vector<Split> calculateSplits(const double totalAmount, const vector<string> &userIds, const vector<double> &values) override
    {
        int users = userIds.size();
        if (users != values.size())
        {
            throw runtime_error("exact amount for each users needed");
        }

        double sum = 0.0;
        for (auto val : values)
        {
            sum += val;
        }
        if (fabs(100.0 - sum) > 0.01)
        {
            throw runtime_error("sum of total percentage must be equal to 100.\n");
        }

        vector<Split> splits;
        for (int i = 0; i < users; i++)
        {
            double amount = (values[i] * totalAmount) / 100.0;
            splits.push_back(Split(userIds[i], amount));
        }
        return splits;
    }
};

class Expense
{
    string id;
    double amount;
    string paidBy;
    vector<Split> splits;
    SplitType type;

public:
    Expense(const string &id, const double amount, const string &paidBy, const vector<Split> &splits, SplitType type)
        : id(id), amount(amount), paidBy(paidBy), splits(splits), type(type) {}

    const string &getId() const
    {
        return id;
    }

    double getAmount() const
    {
        return amount;
    }

    const string &getPaidBy() const
    {
        return paidBy;
    }

    const vector<Split> &getSplits() const
    {
        return splits;
    }

    SplitType getSplitType() const
    {
        return type;
    }
};

class Group
{

    string id;
    string name;
    vector<string> userIds;
    vector<string> expenseIds;

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

    void addUser(const string &id)
    {
        auto it = find(userIds.begin(), userIds.end(), id);
        if (it != userIds.end())
        {
            return;
        }
        userIds.push_back(id);
    }
    void addExpense(const string &id)
    {
        expenseIds.push_back(id);
    }

    const vector<string> &getUsers() const
    {
        return userIds;
    }

    const vector<string> &getExpenses() const
    {
        return expenseIds;
    }
};

class SplitwiseSystem
{
private:
    unordered_map<string, User> users;
    unordered_map<string, Group> groups;
    unordered_map<string, Expense> expenses;

    unordered_map<string, unordered_map<string, double>> balances; // balances[a][b] = 10, a owes b 10Rs.

    int expenseCounter = 1;

    unique_ptr<SplitStrategy> createStrategy(SplitType type)
    {
        switch (type)
        {
        case SplitType::EXACT:
            return make_unique<ExactSplitStrategy>();
        case SplitType::EQUAL:
            return make_unique<EqualSplitStrategy>();
        case SplitType::PERCENTAGE:
            return make_unique<PercentageSplitStrategy>();
        default:
            throw runtime_error("Unknown split strategy");
        }
    }

    void addBalance(const string &debtor, const string &creditor, double amount)
    {
        if (debtor == creditor)
            return;

        double oppositeBalance = balances[creditor][debtor];
        if (oppositeBalance >= amount)
        {
            balances[creditor][debtor] = oppositeBalance - amount;
        }
        else
        {
            balances[creditor][debtor] = 0;
            balances[debtor][creditor] += (amount - oppositeBalance);
        }
    }

public:
    void addUser(const string &id, const string &name)
    {
        users.emplace(id, User(id, name));
    }

    void createGroup(const string &groupId, const string &groupName)
    {
        groups.emplace(groupId, Group(groupId, groupName));
    }

    void addUserToGroup(const string &groupId, const string &userId)
    {
        auto groupIt = groups.find(groupId);
        if (groups.find(groupId) == groups.end())
        {
            throw runtime_error("Group not found\n.");
        }

        if (users.find(userId) == users.end())
        {
            throw runtime_error(
                "User not found");
        }
        groupIt->second.addUser(userId);
    }

    string addExpense(
        const string &groupId,
        const string &paidBy,
        double amount,
        const vector<string> &participants,
        SplitType splitType,
        const vector<double> &values = {})
    {
        if (groups.find(groupId) == groups.end())
        {
            throw runtime_error(
                "Group not found");
        }

        if (users.find(paidBy) == users.end())
        {
            throw runtime_error(
                "Payer not found");
        }

        for (const string &userId : participants)
        {
            if (users.find(userId) == users.end())
            {
                throw runtime_error(
                    "Participant not found");
            }
        }

        unique_ptr<SplitStrategy> splitStrategy = createStrategy(splitType);

        auto splits = splitStrategy->calculateSplits(amount, participants, values);
        string expenseId = "Expense_id_" + to_string(expenseCounter++);
        Expense expense = Expense(expenseId, amount, paidBy, splits, splitType);
        expenses.emplace(expenseId, move(expense));
        groups.at(groupId).addExpense(expenseId);

        for (const Split &split : splits)
        {
            if (split.getUserId() == paidBy)
            {
                continue;
            }

            addBalance(
                split.getUserId(),
                paidBy,
                split.getAmount());
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
    SplitwiseSystem splitwise;

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
        "G1",
        "U1",
        1000.0,
        {"U1", "U2", "U3", "U4"},
        SplitType::EQUAL);

    splitwise.showBalances();

    splitwise.addExpense(
        "G1",
        "U2",
        600,
        {"U1", "U2", "U3", "U4"},
        SplitType::EXACT,
        {100, 100, 200, 200});

    splitwise.showBalances();
    return 0;
}