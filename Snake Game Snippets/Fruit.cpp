#include "Fruit.h"

#include "Game.h"

#define CASCADE_FAIL_LIMITER 50

#define CYCLE_QUEUE seededPositions.push(seededPositions.front()); seededPositions.pop()

// This should have a scope around it since its messing w. my ifs
// Works without the scope but not inside the if context
#define SKIP_QUEUE { seededPositions.push(seededPositions.front()); seededPositions.pop(); continue; }

std::queue<Position> Fruit::seededPositions{};
int Fruit::queueShuffle = 0;

Fruit::Fruit()
{

	Debugging::Log((std::string)"Creating fruit");
	Texture(Game::Current()->Resources()->FruitSheet());

	if (Game::Current()->Settings()->Value(SETTING_HIGHCONTRAST))
	{
		SetUV(Resource::UVFruit(0));
	}
	else
	{
		SetUV(Resource::UVFruit(Game::Current()->Settings()->Value(SETTING_FRUITSKIN)));
	}

	animations = Game::Current()->Settings()->Value(SETTING_ANIMATIONS);
}

void Fruit::ResetElement()
{
	Debugging::Log((std::string)"Resetting Fruit");
	SetActive(true);
	Reposition(Game::Current()->GetMap()->GetPlayer(), Game::Current()->GetMap()->GetFruits());
}

void Fruit::DefineQueue()
{
	while (!seededPositions.empty()) seededPositions.pop(); // Needed to clear queue because if the map size changes this breaks
	for (int x = 0; x < Map::MapWidth; x++)
	{
		for (int y = 0; y < Map::MapHeight; y++)
		{
			seededPositions.push({ x,y });
		}
	}
	ShuffleQueue();
}

// O(n * 2) i think - which could be worse. There might be a better way of doing this but ide prefer to use a queue.
void Fruit::ShuffleQueue()
{
	std::vector<Position> originalQueue;

	while (!seededPositions.empty())
	{
		originalQueue.push_back(seededPositions.front());
		seededPositions.pop(); // is a void method so you have to dequeue after using
	}

	// This might hurt my O(n * 2)
	while (!originalQueue.empty())
	{
		int index = rand() % originalQueue.size();
		seededPositions.push(originalQueue[index]);
		originalQueue[index] = originalQueue.back();
		originalQueue.erase(originalQueue.end() - 1); // https://www.geeksforgeeks.org/cpp/remove-last-element-from-vector-in-cpp/
	}

	queueShuffle = (rand() % (seededPositions.size() / 2)) + (seededPositions.size() / 2);
}

bool Fruit::Reposition(Player* player, std::vector<Fruit*> otherFruit)
{
	return RepositionWithMultiplePlayers({ player }, otherFruit);
}

bool Fruit::RepositionWithMultiplePlayers(std::vector<Player*> players, std::vector<Fruit*> otherFruit)
{
	if (--queueShuffle == 0)
	{
		Debugging::Log("Shuffling queue order");
		ShuffleQueue();
	}

	Position start = seededPositions.front();
	Position current = start;

	CYCLE_QUEUE;

	while (start != seededPositions.front())
	{
		current = seededPositions.front();

		// Cannot go back to same spot because it will be taken by the snake
		if (current == position) SKIP_QUEUE

			bool invalid = false;
		for (Fruit* f : otherFruit)
		{
			if (f->position == current)
			{
				invalid = true;
				break;
			}
		}
		if (invalid) SKIP_QUEUE

			for (Player* player : players) if (player != nullptr && player->TakesSpot(current))
			{
				invalid = true;
				break;
			}
		if (invalid) SKIP_QUEUE

			position = current;

		return true;
	}

	Debugging::Error("No valid spots left for this fruit!");

	return false;
}

void Fruit::SetPosition(Position pos)
{
	position = pos;
}

void Fruit::OnTick()
{
	if (animations)
	{
		angle = std::sinf(Game::Current()->GetElapsedSeconds()) * 15;
	}
}

//0xf23535
