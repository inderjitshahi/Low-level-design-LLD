#include <iostream>
#include <vector>
#include <set>
#include <thread>
#include <chrono>
#include <mutex>
#include <climits>
using namespace std;

// To add a vip request, you would simply create a new VIPRequest class and a VIPSchedulingStrategy.
/*
in step():
    if (!vipStops.empty()) {
                int target = *vipStops.begin();
                if (target > currentFloor) currentFloor++;
                else if (target < currentFloor) currentFloor--;

                if (currentFloor == target) {
                    vipStops.erase(target);
                    openDoor();
                }
                return; // Skip normal stops logic
    }
*/

enum class Direction
{
    UP,
    DOWN,
    IDLE
};

enum class ElevatorState
{
    MOVING,
    STOPPED,
    DOOR_OPEN
};

// Requests structure
class ExternalRequest
{
public:
    int floor;
    Direction direction;
    ExternalRequest(int f, Direction d) : floor(f), direction(d) {}
};

class InternalRequest
{
public:
    int destinationFloor;
    InternalRequest(int f) : destinationFloor(f) {}
};

class Elevator
{
private:
    int id;
    int currentFloor;
    Direction direction;
    ElevatorState state;
    set<int> upStops;
    set<int, greater<int>> downStops;

    void stopAtFloor()
    {
        upStops.erase(currentFloor);
        downStops.erase(currentFloor);
        state = ElevatorState::STOPPED;
        // simulate door open
        state = ElevatorState::DOOR_OPEN;
    }

public:
    Elevator(int id)
        : id(id), currentFloor(0),
          direction(Direction::IDLE),
          state(ElevatorState::STOPPED) {}

    int getCurrentFloor() const
    {
        return currentFloor;
    }

    Direction getDirection() const
    {
        return direction;
    }

    int getId() const
    {
        return id;
    }

    void openDoor()
    {
        cout << "Opening elevator " << id << " door at " << currentFloor << "\n";
        state = ElevatorState::STOPPED;
        state = ElevatorState::DOOR_OPEN;
    }

    void addRequest(int floor)
    {
        // lock_guard<mutex> lock(mtx);
        if (floor == currentFloor)
        {
            openDoor();
            return;
        }
        if (floor > currentFloor)
        {
            upStops.insert(floor);
        }
        else
        {
            downStops.insert(floor);
        }
        updateDirection();
    }

    void step()
    {
        // lock_guard<mutex> lock(mtx);
        if (direction == Direction::UP and !upStops.empty())
        {
            currentFloor++;
            if (upStops.count(currentFloor))
                stopAtFloor();
        }
        else if (direction == Direction::DOWN and !downStops.empty())
        {
            currentFloor--;
            if (downStops.count(currentFloor))
                stopAtFloor();
        }
        updateDirection();
    }

    void updateDirection()
    {
        // Scan Algorithm
        if (direction == Direction::UP)
        {
            // keep going up if there are more steps above
            if (upStops.empty())
            {
                direction = !downStops.empty() ? Direction::DOWN : Direction::IDLE;
            }
        }
        else if (direction == Direction::DOWN)
        {
            // keep going down if there more steps down
            if (downStops.empty())
            {
                direction = !upStops.empty() ? Direction::UP : Direction::IDLE;
            }
        }
        else
        {
            if (!upStops.empty())
                direction = Direction::UP;
            else if (!downStops.empty())
                direction = Direction::DOWN;
        }
    }
};

//---------------------------------------- Scheduler/Controller --------------------------------------------------//
// Strategy Pattern for schedulers
class ISchedulingStrategy
{
public:
    virtual Elevator *selectElevator(
        vector<Elevator *> &elevators,
        ExternalRequest &request) = 0;

    virtual ~ISchedulingStrategy() {}
};

class LookSchedulingStrategy : public ISchedulingStrategy
{
public:
    Elevator *selectElevator(
        vector<Elevator *> &elevators,
        ExternalRequest &req) override
    {
        Elevator *best = nullptr;
        int minCost = INT_MAX;

        for (auto e : elevators)
        {
            int cost = computeCost(e, req);
            if (cost < minCost)
            {
                minCost = cost;
                best = e;
            }
        }
        cout << "Assigned Floor " << req.floor << " to Elevator " << best->getId() << endl;

        return best;
    }

private:
    int computeCost(Elevator *e, ExternalRequest &req)
    {
        if (e->getDirection() == Direction::IDLE)
            return abs(e->getCurrentFloor() - req.floor);

        if (e->getDirection() == req.direction)
        {
            if ((req.direction == Direction::UP &&
                 req.floor >= e->getCurrentFloor()) ||
                (req.direction == Direction::DOWN &&
                 req.floor <= e->getCurrentFloor()))
            {
                return abs(e->getCurrentFloor() - req.floor);
            }
        }

        // Penalize direction change
        return 1000 + abs(e->getCurrentFloor() - req.floor);
    }
};

class ElevatorController : public ISchedulingStrategy
{
public:
    // greedy scheduler, look/scan algorithms can also be used
    Elevator *selectElevator(vector<Elevator *> &elevators,
                             ExternalRequest &req) override
    {
        Elevator *best = nullptr;
        int minDistance = INT_MAX;
        for (auto e : elevators)
        {
            int dist = abs(e->getCurrentFloor() - req.floor);
            // prefer elevator in the same direction
            if (e->getDirection() == req.direction || e->getDirection() == Direction::IDLE)
            {
                if (dist < minDistance)
                {
                    minDistance = dist;
                    best = e;
                }
            }
        }
        if (best)
        {
            cout << "Assigned Floor " << req.floor << " to Elevator " << best->getId() << endl;
        }
        return best;
    }
};

//------ Facade Structural pattern: provides a simplified interface to a complex set of classes, a framework, or a subsystem.  ----------//

// making each thread running independently
// void runElevator(Elevator *e)
// {
//     while (true)
//     {
//         e->step();
//         this_thread::sleep_for(chrono::milliseconds(500));
//     }
// }

class ElevatorSystem
{
private:
    vector<Elevator *> elevators;
    ISchedulingStrategy *controller;
    // vector<thread> threads;

public:
    ElevatorSystem(int numElevators, ISchedulingStrategy *strategy) : controller(strategy)
    {
        for (int i = 0; i < numElevators; i++)
        {
            elevators.push_back(new Elevator(i));
        }
    }

    void setSchedulingStrategy(ISchedulingStrategy *strategy)
    {
        controller = strategy;
    }

    void HandleExternalrequest(int floor, Direction dir)
    {
        ExternalRequest req(floor, dir);
        Elevator *e = controller->selectElevator(elevators, req);
        if (e)
        {
            e->addRequest(floor);
        }
    }

    void HandleInternalRequest(int elevatorId, int destinationFloor)
    {
        if (elevatorId < 0 || elevatorId >= elevators.size())
        {
            cout << "Invalid Elevator ID" << endl;
            return;
        }

        // Direct communication with the specific elevator car
        elevators[elevatorId]->addRequest(destinationFloor);
    }

    // void start()
    // {
    //     for (auto e : elevators)
    //     {
    //         threads.emplace_back(runElevator, e);
    //     }
    // }

    void step()
    {
        for (auto e : elevators)
        {
            e->step();
        }
    }
};

int main()
{

    // Further Improvements:  Weight_limit, Maintenance state, emergency scenario for further upgrade, look algo with dynamic penalty
    int totalElevators = 5;
    // scheduling strategy
    ISchedulingStrategy *strategy = new LookSchedulingStrategy();
    // strategy = new ElevatorController();
    ElevatorSystem system(totalElevators, strategy);

    system.HandleExternalrequest(3, Direction::UP);
    system.HandleInternalRequest(1, 4);
    system.HandleInternalRequest(1, 0);
    system.HandleExternalrequest(0, Direction::DOWN);
    system.step();
    system.step();
    system.step();
    system.step();
    system.step();
    system.step();

    return 0;
}