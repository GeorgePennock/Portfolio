#pragma once

#include <queue>

#include "Element.h"
#include "Player.h"

class Fruit : public Element
{
private:
	bool animations;
public:
	Fruit();

	bool Reposition(Player* player, std::vector<Fruit*> otherFruit);
	bool RepositionWithMultiplePlayers(std::vector<Player*> players, std::vector<Fruit*> otherFruit);

	void SetPosition(Position);

	void OnTick() override;
	void ResetElement() override;

	static void DefineQueue();
private:

	static std::queue<Position> seededPositions;
	static void ShuffleQueue();
	static int queueShuffle;
};

