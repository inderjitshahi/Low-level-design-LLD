#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <stdexcept>

using namespace std;

enum class SplitType
{
    EQUAL,
    EXACT,
    PERCENTAGE
};

class User
{
private:
    string userId;
    string name;

public:
    User(const string &userId, const string &name)
        : userId(userId), name(name) {}

    const string &getId() const
    {
        return userId;
    }

    const string &getName() const
    {
        return name;
    }
};

class Split
{
private:
    string userId;
    double amount;

public:
    Split(const string &userId, double amount = 0.0)
        : userId(userId), amount(amount) {}

    const string &getUserId() const
    {
        return userId;
    }

    double getAmount() const
    {
        return amount;
    }

    void setAmount(double newAmount)
    {
        amount = newAmount;
    }
};

class SplitStrategy
{
public:
    virtual ~SplitStrategy() = default;

    virtual vector<Split> calculateSplits(
        double totalAmount,
        const vector<string> &userIds,
        const vector<double> &values = {}) = 0;
};

class EqualSplitStrategy : public SplitStrategy
{
public:
    vector<Split> calculateSplits(
        double totalAmount,
        const vector<string> &userIds,
        const vector<double> &values = {}) override
    {
        if (userIds.empty())
        {
            throw runtime_error("No users provided");
        }

        double amountPerUser =
            totalAmount / userIds.size();

        vector<Split> splits;

        for (const string &userId : userIds)
        {
            splits.emplace_back(
                userId,
                amountPerUser);
        }

        return splits;
    }
};

class ExactSplitStrategy : public SplitStrategy
{
public:
    vector<Split> calculateSplits(
        double totalAmount,
        const vector<string> &userIds,
        const vector<double> &values) override
    {
        if (userIds.size() != values.size())
        {
            throw runtime_error(
                "Exact amounts count must match users count");
        }

        double sum = 0;

        for (double value : values)
        {
            sum += value;
        }

        if (fabs(sum - totalAmount) > 0.01)
        {
            throw runtime_error(
                "Exact split amounts do not match total amount");
        }

        vector<Split> splits;

        for (int i = 0; i < userIds.size(); i++)
        {
            splits.emplace_back(
                userIds[i],
                values[i]);
        }

        return splits;
    }
};

class PercentageSplitStrategy : public SplitStrategy
{
public:
    vector<Split> calculateSplits(
        double totalAmount,
        const vector<string> &userIds,
        const vector<double> &percentages) override
    {
        if (userIds.size() != percentages.size())
        {
            throw runtime_error(
                "Percentage count must match users count");
        }

        double totalPercentage = 0;

        for (double percentage : percentages)
        {
            totalPercentage += percentage;
        }

        if (fabs(totalPercentage - 100.0) > 0.01)
        {
            throw runtime_error(
                "Percentages must add up to 100");
        }

        vector<Split> splits;

        for (int i = 0; i < userIds.size(); i++)
        {
            double amount =
                totalAmount * percentages[i] / 100.0;

            splits.emplace_back(
                userIds[i],
                amount);
        }

        return splits;
    }
};

class Expense
{
private:
    string expenseId;
    string description;

    double amount;

    string paidBy;

    vector<Split> splits;

    SplitType splitType;

public:
    Expense(
        string expenseId,
        string description,
        double amount,
        string paidBy,
        vector<Split> splits,
        SplitType splitType)
        : expenseId(move(expenseId)),
          description(move(description)),
          amount(amount),
          paidBy(move(paidBy)),
          splits(move(splits)),
          splitType(splitType)
    {
    }

    const string &getExpenseId() const
    {
        return expenseId;
    }

    const string &getDescription() const
    {
        return description;
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
        return splitType;
    }
};

class Group
{
private:
    string groupId;
    string name;

    vector<string> userIds;
    vector<string> expenseIds;

public:
    Group(
        const string &groupId,
        const string &name)
        : groupId(groupId),
          name(name)
    {
    }

    const string &getId() const
    {
        return groupId;
    }

    const string &getName() const
    {
        return name;
    }

    void addUser(const string &userId)
    {
        userIds.push_back(userId);
    }

    void addExpense(const string &expenseId)
    {
        expenseIds.push_back(expenseId);
    }

    const vector<string> &getUsers() const
    {
        return userIds;
    }
};

class SplitwiseSystem
{
private:
    unordered_map<string, User> users;
    unordered_map<string, Group> groups;
    unordered_map<string, Expense> expenses;

    /*
        balances[A][B] = amount A owes B

        Example:

        balances["U2"]["U1"] = 500

        means U2 owes U1 Rs.500
    */
    unordered_map<
        string,
        unordered_map<string, double>>
        balances;

    int expenseCounter = 1;

    unique_ptr<SplitStrategy> createStrategy(
        SplitType splitType)
    {
        if (splitType == SplitType::EQUAL)
        {
            return make_unique<EqualSplitStrategy>();
        }

        if (splitType == SplitType::EXACT)
        {
            return make_unique<ExactSplitStrategy>();
        }

        if (splitType == SplitType::PERCENTAGE)
        {
            return make_unique<PercentageSplitStrategy>();
        }

        throw runtime_error(
            "Unsupported split type");
    }

    void addBalance(
        const string &debtor,
        const string &creditor,
        double amount)
    {
        if (debtor == creditor)
        {
            return;
        }

        /*
            Suppose:

            U2 owes U1 = 500

            Later U1 owes U2 = 200

            Instead of storing both:

            U2 -> U1 = 500
            U1 -> U2 = 200

            we simplify it to:

            U2 -> U1 = 300
        */

        double oppositeBalance =
            balances[creditor][debtor];

        if (oppositeBalance >= amount)
        {
            balances[creditor][debtor] -= amount;
        }
        else
        {
            balances[creditor][debtor] = 0;

            balances[debtor][creditor] +=
                amount - oppositeBalance;
        }
    }

public:
    void addUser(
        const string &userId,
        const string &name)
    {
        users.emplace(
            userId,
            User(userId, name));
    }

    void createGroup(
        const string &groupId,
        const string &groupName)
    {
        groups.emplace(
            groupId,
            Group(groupId, groupName));
    }

    void addUserToGroup(
        const string &groupId,
        const string &userId)
    {
        auto groupIt =
            groups.find(groupId);

        if (groupIt == groups.end())
        {
            throw runtime_error(
                "Group not found");
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
        const string &description,
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

        unique_ptr<SplitStrategy> strategy =
            createStrategy(splitType);

        vector<Split> splits =
            strategy->calculateSplits(
                amount,
                participants,
                values);

        string expenseId =
            "EXP_" +
            to_string(expenseCounter++);

        Expense expense(
            expenseId,
            description,
            amount,
            paidBy,
            splits,
            splitType);

        expenses.emplace(
            expenseId,
            move(expense));

        groups.at(groupId)
            .addExpense(expenseId);

        /*
            If U1 paid Rs.1000

            Equal split between:
            U1, U2, U3, U4

            Each share = Rs.250

            U1 paid his own Rs.250,
            so nothing for U1.

            U2 owes U1 Rs.250
            U3 owes U1 Rs.250
            U4 owes U1 Rs.250
        */

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

        for (const auto &entry : balances)
        {
            const string &debtor =
                entry.first;

            for (const auto &credit : entry.second)
            {
                const string &creditor =
                    credit.first;

                double amount =
                    credit.second;

                if (amount <= 0.01)
                {
                    continue;
                }

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

    void showBalance(
        const string &userId) const
    {
        if (users.find(userId) == users.end())
        {
            throw runtime_error(
                "User not found");
        }

        cout << "\nBalance for "
             << users.at(userId).getName()
             << "\n";

        bool found = false;

        /*
            Money this user owes someone
        */

        auto debtorIt =
            balances.find(userId);

        if (debtorIt != balances.end())
        {
            for (const auto &entry : debtorIt->second)
            {
                if (entry.second <= 0.01)
                {
                    continue;
                }

                cout
                    << users.at(userId).getName()
                    << " owes "
                    << users.at(entry.first).getName()
                    << " : Rs."
                    << entry.second
                    << '\n';

                found = true;
            }
        }

        /*
            Money someone owes this user
        */

        for (const auto &entry : balances)
        {
            auto it =
                entry.second.find(userId);

            if (it == entry.second.end() ||
                it->second <= 0.01)
            {
                continue;
            }

            cout
                << users.at(entry.first).getName()
                << " owes "
                << users.at(userId).getName()
                << " : Rs."
                << it->second
                << '\n';

            found = true;
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

    // =====================================
    // Expense 1
    //
    // Aman paid 1000.
    // Split equally between all 4.
    //
    // Each share = 250
    //
    // Rahul owes Aman 250
    // Karan owes Aman 250
    // Rohit owes Aman 250
    // =====================================

    splitwise.addExpense(
        "G1",
        "U1",
        1000,
        "Dinner",
        {"U1", "U2", "U3", "U4"},
        SplitType::EQUAL);

    splitwise.showBalances();

    // =====================================
    // Expense 2
    //
    // Rahul paid 600.
    //
    // Exact split:
    //
    // Aman 100
    // Rahul 100
    // Karan 200
    // Rohit 200
    // =====================================

    splitwise.addExpense(
        "G1",
        "U2",
        600,
        "Taxi",
        {"U1", "U2", "U3", "U4"},
        SplitType::EXACT,
        {100, 100, 200, 200});

    splitwise.showBalances();

    // =====================================
    // Expense 3
    //
    // Karan paid 1000.
    //
    // Percentage:
    //
    // Aman 40% = 400
    // Rahul 30% = 300
    // Karan 20% = 200
    // Rohit 10% = 100
    // =====================================

    splitwise.addExpense(
        "G1",
        "U3",
        1000,
        "Hotel",
        {"U1", "U2", "U3", "U4"},
        SplitType::PERCENTAGE,
        {40, 30, 20, 10});

    splitwise.showBalances();

    // Show one user's balances
    splitwise.showBalance("U1");

    return 0;
}