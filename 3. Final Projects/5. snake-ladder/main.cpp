#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <string>
#include <memory>
#include <utility>
#include <cstdlib>
using namespace std;

class Jump
{
    int start;
    int end;

public:
    Jump(int start, int end) : start(start), end(end) {}
    int getStart() const
    {
        return start;
    }
    int getEnd() const
    {
        return end;
    }
};

class Board
{
    int size;
    unordered_map<int, Jump> jumps;

public:
    Board(int size) : size(size) {}
    int getSize() const
    {
        return size;
    }
    void addJump(const Jump &jump)
    {
        jumps.insert(make_pair(jump.getStart(), jump));
    }

    bool hasJump(int position) const
    {
        return jumps.find(position) != jumps.end();
    }

    int getJumpDestination(int position) const
    {
        return jumps.at(position).getEnd();
    }
};

class Player
{
    string name;
    int position;

public:
    Player(const string &name, int position) : name(name), position(position) {}
    string getName() const
    {
        return name;
    }
    int getPosition() const
    {
        return position;
    }
    void setPosition(int pos)
    {
        position = pos;
    }
};

// ---------------------- Dice interface  -------------------------------------//
class Dice
{
public:
    virtual int roll() = 0;
    virtual ~Dice() = default;
};

class NormalDice : public Dice
{
public:
    int roll() override
    {
        return 1 + rand() % 6; // min + rand()%(max-min+1);
    }
};

class Dice20 : public Dice
{
public:
    int roll() override
    {
        return 1 + rand() % 20; // min + rand()%(max-min+1);
    }
};

class Game
{
private:
    Board board;
    unique_ptr<Dice> dice;
    vector<Player> players;
    queue<Player *> turnQueue;

public:
    Game(Board &board, unique_ptr<Dice> dice, vector<Player> playerList) : board(board), dice(move(dice)), players(move(playerList))
    {
        for (auto &player : this->players)
        {
            turnQueue.push(&player);
        }
    }

    void movePlayer(Player *player)
    {
        int diceValue = dice->roll();
        int nextPos = player->getPosition() + diceValue;
        if (nextPos > board.getSize())
        {
            return;
        }
        if (board.hasJump(nextPos))
        {
            nextPos = board.getJumpDestination(nextPos);
        }
        cout << player->getName() << " moving to: " << nextPos << "\n";
        player->setPosition(nextPos);
    }

    bool hasWon(Player *player)
    {
        return player->getPosition() == board.getSize();
    }
    void start()
    {
        while (true)
        {
            Player *current = turnQueue.front();
            turnQueue.pop();
            movePlayer(current);
            if (hasWon(current))
            {
                cout << current->getName() << " wins.\n";
                break;
            }

            turnQueue.push(current);
        }
    }
};

int main()
{
    Board board(100);
    // Ladders
    board.addJump(Jump(4, 25));
    board.addJump(Jump(13, 46));
    board.addJump(Jump(33, 49));
    board.addJump(Jump(42, 63));
    board.addJump(Jump(50, 69));

    // Snakes
    board.addJump(Jump(40, 3));
    board.addJump(Jump(56, 18));
    board.addJump(Jump(83, 54));
    board.addJump(Jump(94, 71));
    board.addJump(Jump(99, 12));

    vector<Player> players;

    players.emplace_back("Inderjit", 0);
    players.emplace_back("Rahul", 0);

    unique_ptr<Dice> dice = make_unique<NormalDice>();
    unique_ptr<Dice> dice20 = make_unique<Dice20>();

    Game game(board, move(dice20), move(players));
    cout << "Game started\n";
    game.start();
    cout << "Game ended\n";
    return 0;
    return 0;
}