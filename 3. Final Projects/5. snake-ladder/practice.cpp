#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <memory>
#include <utility>
#include <cstdlib> // for random
using namespace std;

// board, user/player, game(orchestrator), Dice, snake/ladder => jump

enum class GameStatus
{
    NOT_STARTED = 0,
    ONGOING,
    COMPLETED
};

class Player
{
    string id;
    string name;
    int position;

public:
    Player(const string &id, const string &name) : id(id), name(name), position(0) {}

    const string &getId() const
    {
        return id;
    }

    const string &getName() const
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

class Jump
{
private:
    int start;
    int end;

public:
    Jump(int start, int end) : start(start), end(end) {};
    int getStart() const
    {
        return start;
    }

    int getEnd() const
    {
        return end;
    }
};

class Dice
{
private:
    int faces;

public:
    Dice(const int faces = 6) : faces(faces) {}
    int getFaces() const
    {
        return faces;
    }

    void setFaceSize(int s)
    {
        faces = s;
    }

    // dice can roll
    int roll() const
    {
        return (rand() % (faces) + 1);
    }
};

class Board
{
    int size;
    unordered_map<int, Jump> jumps; // start, Jump

public:
    Board(int size = 100) : size(size) {}

    int getSize() const
    {
        return size;
    }

    void setSize(int s)
    {
        size = s;
    }

    void addJump(int s, int e)
    {
        jumps.emplace(s, Jump(s, e));
    }

    bool hasJump(int pos) const
    {
        return jumps.count(pos) != 0;
    }

    int getJump(int pos) const
    {
        auto it = jumps.find(pos);
        if (it != jumps.end())
        {
            return it->second.getEnd();
        }
        return 0;
    }
};

// orchestrator
class Game
{

    // using composition, lifecycle of dice, board and players are handled by Game. Has a relation
    unique_ptr<Dice> dice; // this is just blue print, no constructor etc called here
    unique_ptr<Board> board;
    vector<Player> players;

    GameStatus status;

    int turn; // on start its player at index 0 turn

public:
    Game(unique_ptr<Dice> dice, unique_ptr<Board> board)
        : dice(move(dice)),
          board(move(board)),
          turn(0),
          status(GameStatus::NOT_STARTED) {} // dice, board object is created here,only in initializer list is mandatory

    void addUser(const string &id, const string &name)
    {
        if (status != GameStatus::NOT_STARTED)
        {
            throw runtime_error("Can't add player while game going on.\n");
        }
        players.emplace_back(Player(id, name));
    }

    void movePlayer(Player &player)
    {
        int points = dice->roll();
        int newPos = points + player.getPosition();

        if (newPos > board->getSize()) // player can't move if going outside the board
            return;

        if (board->hasJump(newPos))
        {
            int jump = board->getJump(newPos);
            newPos = jump;
        }

        player.setPosition(newPos);
    }

    bool hasWon(const Player &player) const
    {
        if (player.getPosition() == board->getSize())
            return true;
        return false;
    }

    void startGame()
    {
        if (players.size() < 2)
        {
            throw runtime_error("Please add atleast 2 players to start the game.\n");
        }
        status = GameStatus::ONGOING;
        while (true)
        {
            int totalPlayers = players.size();
            Player &player = players[turn];
            movePlayer(players[turn]);
            cout << "Moved player " << player.getName() << " to position " << player.getPosition() << "\n";
            if (hasWon(players[turn]))
            {
                cout << "Player " << players[turn].getName() << " have won the game.\n";
                break;
            }

            turn = (turn + 1) % totalPlayers;
        }

        status = GameStatus::COMPLETED;
    }
};

int main()
{
    try
    {
        // create dice
        unique_ptr<Dice> dice = make_unique<Dice>(6);

        // create board
        unique_ptr<Board> board = make_unique<Board>(100);

        // add snakes and ladders
        board->addJump(5, 10);
        board->addJump(90, 4);
        board->addJump(11, 14);
        board->addJump(31, 99);
        board->addJump(98, 20);
        board->addJump(45, 81);

        Game game(move(dice), move(board));

        game.addUser("1", "Ram");
        game.addUser("2", "Shyam");
        game.startGame();
    }
    catch (const exception &e) // const and & not required, but as general practice added
    {
        cerr << "Game over: " << e.what() << ".\n";
        return 1;
    }
    catch (...)
    {
        cerr << "Unknown error occurred\n";
    }
    return 0;
}