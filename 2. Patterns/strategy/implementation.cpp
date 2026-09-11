#include <bits/stdc++.h>
using namespace std;
/**
1️⃣ What is the Strategy Pattern( Behavioral )?

Define a family of algorithms, encapsulate each one, and make them interchangeable at runtime.

Instead of hardcoding behavior using if-else or switch,
we delegate behavior to strategy objects.

| Component                | Responsibility            |
| ------------------------ | ------------------------- |
| **Strategy (Interface)** | Declares common algorithm |
| **ConcreteStrategy**     | Implements algorithm      |
| **Context**              | Uses a Strategy           |


Use Strategy when: You have multiple ways to perform an action

Strategy Pattern lets you select an algorithm at runtime by encapsulating each algorithm inside a separate class and making them interchangeable.

follows: Open–Closed + Dependency Inversion.
 */

class PaymentProcessorWithoutStrategy
{
public:
    void pay(string method)
    {
        if (method == "UPI")
        {
            cout << "Paying via UPI\n";
        }
        else if (method == "Card")
        {
            cout << "Paying via Card\n";
        }
        else if (method == "NetBanking")
        {
            cout << "Paying via NetBanking\n";
        }
    }
};

class PaymentStrategy
{
public:
    virtual void pay(int amount) = 0;
    virtual ~PaymentStrategy() = default;
};

class UpiPayment : public PaymentStrategy
{
public:
    void pay(int amount) override
    {
        cout << "Paying " << amount << " via UPI\n";
    }
};

class CardPayment : public PaymentStrategy
{
public:
    void pay(int amount) override
    {
        cout << "Paying " << amount << " via Card\n";
    }
};

class PaymentContext
{
private:
    unique_ptr<PaymentStrategy> strategy;

public:
    PaymentContext(PaymentStrategy *s) : strategy(s) {};

    void setStrategy(PaymentStrategy *s)
    {
        strategy.reset(s); // safely reset a unique pointer to new  value
    }
    void make_payment(int amount)
    {
        strategy->pay(amount);
    }
};

int main()
{
    // strategies or ways of payment
    PaymentStrategy *upi = new UpiPayment();
    PaymentStrategy *card = new CardPayment();

    PaymentContext context(upi);
    context.make_payment(100);

    // changing strategy
    context.setStrategy(card);
    context.make_payment(200);

    delete upi;
    delete card;
    return 0;
}