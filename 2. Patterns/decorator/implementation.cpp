#include <bits/stdc++.h>
using namespace std;
/*
Decorator Pattern (Structural) allows you to add new behavior to an object dynamically without changing its existing code.
    Uses composition over inheritance
    Wraps an object inside another object that has the same interface
    Behaviors can be stacked at runtime.

Use it when:
✔ You want to add features at runtime
✔ You want to avoid too many subclasses


Core idea:
    Component → Interface
    ConcreteComponent → Base behavior
    Decorator → Wraps Component
    ConcreteDecorators → Add features
*/

// component interface
class Coffee
{
public:
    virtual string getDescription() const = 0; // const: This function will not modify any data inside the object. used only for class functions.
    virtual double cost() const = 0;
    virtual ~Coffee() {};
};

// concrete component
class SimpleCoffee : public Coffee
{
public:
    string getDescription() const override
    {
        return "Simple coffee";
    }
    double cost() const override
    {
        return 50.0;
    }
};

// In C++, if a class inherits from an interface but doesn't implement all the pure virtual functions (= 0), that class also becomes an Abstract Class.
class CoffeeDecorator : public Coffee
{
protected:
    Coffee *coffee; // composition over inheritance

public:
    CoffeeDecorator(Coffee *c) : coffee(c) {}
};

class MilkDecorator : public CoffeeDecorator
{
public:
    // base constructor not inherited but called, we use the ':' to call the Base constructor,
    MilkDecorator(Coffee *c) : CoffeeDecorator(c) {}
    string getDescription() const override
    {
        return coffee->getDescription() + ", Milk";
    }

    double cost() const override
    {
        return coffee->cost() + 10.0;
    }
};

class SugarDecorator : public CoffeeDecorator
{
public:
    SugarDecorator(Coffee *c) : CoffeeDecorator(c) {}

    string getDescription() const override
    {
        return coffee->getDescription() + ", Sugar";
    }

    double cost() const override
    {
        return coffee->cost() + 5.0;
    }
};

int main()
{
    // with inheritance we have to create correct derived object (Class Explosion), but with decorator we can add any feature to base object dynamically

    /*
    SimpleCoffee
    CoffeeWithMilk
    CoffeeWithSugar
    CoffeeWithMilkAndSugar
    CoffeeWithMilkAndCaramel
    ...and so on.

    If you have 5 toppings, you might end up needing 31 different classes to cover every combination. This is impossible to maintain.
    */
    Coffee *coffee = new SimpleCoffee();
    coffee = new MilkDecorator(coffee);
    coffee = new SugarDecorator(coffee);

    cout << coffee->getDescription() << endl;
    cout << "Total cost: " << coffee->cost() << endl;
    return 0;
}