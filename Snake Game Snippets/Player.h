#pragma once

#include "LinkedList.h"
#include "Element.h"
#include "Inputs.h"

// I can make a second player class which renders the snake body in a nicer way
// It can use smooth movement instead of the Positional movement, so that it animates
// Or i could do this logic within the draw part of the element class


class Player: public Element 
{
private:

	struct Cell
	{
		char flags;
		short int position;

		Cell(char flags, short int position);
	};

	struct Segment
	{
		Segment();
		Segment(Position segPos, bool isEdge);
		Position MyPosition;
		bool IsEdge;

		bool operator==(const Segment& other) const;
	};

	// Dictates whether the snake is controlled by the player or the GameMode - If enabled the snake does nothing on its own
	bool ai;

	bool isPlayerEdge; // If the player teleported

	bool isCurrentTextureHead = false;

	bool alive = true;

	bool flashState;

	bool torus;

	bool victorious = false;


	// Specification says that I need to store the snake body within a vector - however it was discussed w. tutors that it should be a LinkedList instead
	// In my case vector might be faster as I use indexing alot.

	// Body Parts
	LinkedList<Segment> body;
	
	int bodyLength; // Seperate from score
	int score;

	int cacheGameElapsedSeconds;
	int cacheFlashTicks;

	// How many tiles the snake can move per second (aprox)
	float speed;	

	// Used to check which direction you're currently going
	Inputs::EKey directionalKey;
	Inputs::EKey lockedDirectionalKey;

	// Flashing only info
	std::vector<int> flashPosition;

	std::vector<sf::IntRect> textureRects;

	bool IsInFlashPosition(int);
	void DecrementFlash();

	Position direction;


	void DrawSegment(sf::Vector2f a, sf::Vector2f b, float animationProgress, sf::RenderWindow& window, Cell cell);

	void DrawDuoSegment(sf::Vector2f a, sf::Vector2f b, float animationProgress, sf::RenderWindow& window, Cell cell, Position direction);

	void DrawEyes(sf::Vector2f position, Position direction, sf::RenderWindow& window);
	
	void FruitCheck();
	void CollisionsCheck();

public:

	void GainScore(); 
	void GainScore(int amount); 
	void LoseScore(); 
	int GetScore() const;

	void OnPlay() override;
	void OnTick() override;

	void ResetElement() override;

	bool TakesSpot(Position position);

	// Could also do a friend class instead of these methods

	// Only works if the snake is AI mode
	void AI_Direction(Inputs::EKey directionalKey);
	// Only works if the snake is AI mode
	void AI_Position(Position position);

	Inputs::EKey Direction();

	bool IsAI();

	// Evil
	// 16/02/2026: Why is this evil? i should stop leaving vague comments fr fr
	// 21/04/2026: Maybe its cause it was super unoptimised idk bro ur askin the wrong guy
	void Draw(sf::RenderWindow& window) override;

	void SetAlive(bool state, bool victory = false);

	bool IsDead();

	// Flashes the snake for half a second, isPositive decides whether it was a good or a bad thing
	void Flash(bool isPositive);

	static Player* CreateEvilPlayer();
};

