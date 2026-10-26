#include "Player.h"
#include "Map.h"
#include "Game.h"

#define TURNING_TOLERANCE .1f

#define FLAG_HEAD 0b01
#define OPT_FLAG_HEAD(condition) condition ? 0b01 : 0b00
#define FLAG_COLOUR 0b10
#define OPT_FLAG_COLOUR(condition) condition ? 0b10 : 0b00
#define FLAG_BOTH 0b11

inline float interpolate(float a, float b, float t);

#define TRECT_HEAD textureRects[0]
#define TRECT_EYES textureRects[1]
#define TRECT_BODY_PRIMARY textureRects[2]
#define TRECT_BODY_SECONDARY textureRects[3]
#define TRECT_NEGATIVE textureRects[4]
#define TRECT_POSITIVE textureRects[5]

void Player::GainScore()
{
	Game::Current()->GetAudio()->Play(Audio::eGainScore);
	score++;
	bodyLength++;	

	Flash(true);
}

void Player::GainScore(int amount)
{
	Game::Current()->GetAudio()->Play(Audio::eGainScore);
	score += amount;
	bodyLength++;

	Flash(true);
}

void Player::LoseScore()
{
	Game::Current()->GetAudio()->Play(Audio::eLoseScore);
	score--;
	bodyLength--;

	if (score < 0) score = 0;

	Flash(false);
}


int Player::GetScore() const
{
	return score;
}

void Player::OnPlay() // This is ran to reset the snake after you die
{
	Debugging::Log((std::string)"Player Activated!");
	speed = Game::Current()->GetGameMode()->GetPlayerSpeed();
	bodyLength = Game::Current()->GetGameMode()->GetPlayerBodySize();
	score = 0;
	cacheGameElapsedSeconds = 0;
	cacheFlashTicks = 0;
	flashPosition.clear();
	flashState = false;
	
	int skin = Game::Current()->Settings()->Value(SETTING_SNAKESKIN);
	if (Game::Current()->Settings()->Value(SETTING_HIGHCONTRAST)) skin = 0;
	
	if (ai) skin = 8;

	textureRects = {
		/* 0 */ Resource::UVSnakeHead(skin),
		/* 1 */ Resource::UVSnakeEyes(skin),
		/* 2 */ Resource::UVSnakeBodyPrimary(skin),
		/* 3 */ Resource::UVSnakeBodySecondary(skin),
		/* 4 */ Resource::UVSnakeNegative(skin),
		/* 5 */ Resource::UVSnakePositive(skin),
	};

	Texture(Game::Current()->Resources()->SnakeSheet());
	SetUV(TRECT_BODY_PRIMARY);
	isCurrentTextureHead = true;
	alive = true;
	isPlayerEdge = false;
	body.clear();
	direction = Position(0, 0);
	directionalKey = Inputs::EKey::eNone;
	lockedDirectionalKey = Inputs::EKey::eNone;

	if (!ai) position = Position(0, 0);

	torus = Game::Current()->GetGameMode()->GetTorus();

}

void Player::OnTick()
{
	if (!IsActive()) return;

	if (!alive)
	{
		return;
	}

	if (!ai)
	{
		Position oldDirection = direction;

		bool keyUp = Inputs::IsKeyPressed(Inputs::EKey::eUp);
		bool keyDown = Inputs::IsKeyPressed(Inputs::EKey::eDown);
		bool keyLeft = Inputs::IsKeyPressed(Inputs::EKey::eLeft);
		bool keyRight = Inputs::IsKeyPressed(Inputs::EKey::eRight);

		if (keyUp && lockedDirectionalKey != Inputs::EKey::eDown)
		{
			directionalKey = Inputs::EKey::eUp;
			direction = Position(0, -1);
		}
		else if (keyDown && lockedDirectionalKey != Inputs::EKey::eUp)
		{
			directionalKey = Inputs::EKey::eDown;
			direction = Position(0, 1);
		}
		else if (keyLeft && lockedDirectionalKey != Inputs::EKey::eRight)
		{
			directionalKey = Inputs::EKey::eLeft;
			direction = Position(-1, 0);
		}
		else if (keyRight && lockedDirectionalKey != Inputs::EKey::eLeft)
		{
			directionalKey = Inputs::EKey::eRight;
			direction = Position(1, 0);
		}
	}

	// Speed cannot be changed midgame because of this system otherwise it can jump forwards
	float seconds = Game::GetElapsedSeconds() * speed;

	// I should let the player turn if their animation progress is less than .1f
	//if (seconds - floor(seconds) < TURNING_TOLERANCE && directionChanged)
	//{
	//position -= oldDirection;
	//	position += direction;
	//	Debugging::Log((std::string)"Player Redirected");
	//}

	// Try reduce indentation here pls
	if (floor(seconds) != cacheGameElapsedSeconds)
	{
		if (direction != Position(0, 0))
		{
			// Add new body segment but dont check max size limits until after I run fruit collection logic
			body.pushEnd(Segment(position,isPlayerEdge));

			// Move player
			position += direction;
			lockedDirectionalKey = directionalKey;

			// Map bounds
			bool hitWall = false;
			bool lPlayerEdge = false;
			if (position.X < 0)
			{
				hitWall = true;
				position.X = torus ? (Map::MapWidth - 1) : 0;
				lPlayerEdge = true;
			}
			else if (position.X >= Map::MapWidth)
			{
				hitWall = true;
				position.X = torus ? 0 : (Map::MapWidth - 1);
				lPlayerEdge = true;
			}

			if (position.Y < 0)
			{
				hitWall = true;
				position.Y = torus ? (Map::MapHeight - 1) : 0;
				lPlayerEdge = true;
			}
			else if (position.Y >= Map::MapHeight)
			{
				hitWall = true;
				position.Y = torus ? 0 : (Map::MapHeight - 1);
				lPlayerEdge = true;
			}

			isPlayerEdge = lPlayerEdge;

			FruitCheck();

			// Now i check the body length after doing fruit check logic but before collision logic 
			// Using a while loop here should allow the body of the snake to decrease when you lose score - as it needs to remove two elements
			while (body.size() > bodyLength)
			{
				body.popStart();
				//if (!body.empty()) body.erase(body.begin());
			}

			if (hitWall && !torus)
			{
				Game::Current()->GetGameMode()->PlayerHitEdgeOfMap(this);
			}

			Game::Current()->GetGameMode()->PlayerMovedTile(this);

			CollisionsCheck();
		}
	}
	cacheGameElapsedSeconds = seconds;

	if (!flashPosition.empty() )
	{
		int temp = static_cast<int>(floor(seconds * 2));
		if (temp != cacheFlashTicks)
		{
			DecrementFlash();
			cacheFlashTicks = temp;
		}
	}
}

void Player::ResetElement()
{
	// Should work
	OnPlay();
}

bool Player::TakesSpot(Position position)
{
	if (position == this->position) return true;

	return body.contains(Segment(position, false));
	/*for (Segment& p : body)
	{
		if (p.MyPosition == position) return true;
	}

	return false;*/
}

void Player::AI_Direction(Inputs::EKey directionalKey)
{
	if (!ai) return;
	if (directionalKey == Inputs::EKey::eUp && lockedDirectionalKey != Inputs::EKey::eDown)
	{
		this->directionalKey = Inputs::EKey::eUp;
		direction = Position(0, -1);
	}
	else if (directionalKey == Inputs::EKey::eDown && lockedDirectionalKey != Inputs::EKey::eUp)
	{
		this->directionalKey = Inputs::EKey::eDown;
		direction = Position(0, 1);
	}
	else if (directionalKey == Inputs::EKey::eLeft && lockedDirectionalKey != Inputs::EKey::eRight)
	{
		this->directionalKey = Inputs::EKey::eLeft;
		direction = Position(-1, 0);
	}
	else if (directionalKey == Inputs::EKey::eRight && lockedDirectionalKey != Inputs::EKey::eLeft)
	{
		this->directionalKey = Inputs::EKey::eRight;
		direction = Position(1, 0);
	}
}

void Player::AI_Position(Position position)
{
	if (!ai) return;
	Debugging::Log("Repositioning AI Snake to " + std::to_string(position.X) + ", " + std::to_string(position.Y));
	
	body.clear();

	this->position = position;
	body.pushEnd(Segment(position, false));
}

Inputs::EKey Player::Direction()
{
	return directionalKey;
}

bool Player::IsAI()
{
	return ai;
}

void Player::Draw(sf::RenderWindow& window)
{
	if (!IsActive()) return;
	// I should be interpolating between the previous and the current position
	// instead of the current and next!
	// It will prevent jumping and make more sense for things like collisions

	// Previous position would be body[len-1] when setup, however ive not made the body yet

	// I should render the body using a custom line renderer solution instead of drawing each piece seperatly
	// That or i could draw them as individual circle sprites kinda

	float animationProgress = 0;
	if (alive)
	{
		float t = Game::GetElapsedSeconds() * speed;
		animationProgress = t - cacheGameElapsedSeconds;
		if (animationProgress < 0) animationProgress = 0; else if (animationProgress > 1) animationProgress = 1;
	}
	

	int bodySize = body.size();
	//Debugging::Log("Body size " + std::to_string(bodySize));

	sf::Vector2f a;
	sf::Vector2f b;
	for (int i = 0; i < bodySize - 1; i++)
	{
		Segment seg1 = body.atIndex(i);
		Segment seg2 = body.atIndex(i + 1);
		a = Map::Translate(seg1.MyPosition);
		b = Map::Translate(seg2.MyPosition);

		if (seg2.IsEdge)
		{
			DrawDuoSegment(a, b, animationProgress, window, Cell(OPT_FLAG_COLOUR((bodySize - i) % 2 == 0), i), seg1.MyPosition - seg2.MyPosition);
		}
		else
		{
			DrawSegment(a, b, animationProgress, window, Cell(OPT_FLAG_COLOUR((bodySize - i) % 2 == 0),i));
		}
	}

	Segment last = body.peekEnd();

	a = Map::Translate(body.size() < 1 ? position : last.MyPosition);
	b = Map::Translate(position);

	if (isPlayerEdge)
	{
		Position direction = (body.size() < 1 ? position : last.MyPosition) - position;
		DrawDuoSegment(a, b, animationProgress, window, Cell(FLAG_HEAD, bodySize - 1), direction);
	}
	else DrawSegment(a, b, animationProgress,window, Cell(FLAG_HEAD, bodySize - 1));

}

void Player::SetAlive(bool state, bool victory)
{
	this->victorious = victory;
	alive = state;

	flashPosition.clear();
	flashState = false;
}

bool Player::IsDead()
{
	return !alive;
}

void Player::Flash(bool isPositive)
{
	// It skips the first time it tries to flash so dont subtract 1 - Order Of Operations issue
	if (isPositive)
	{
		flashPosition.push_back(body.size());
	}
	else
	{
		flashPosition.push_back(body.size() - 2);
	}
	flashState = isPositive;
}

Player* Player::CreateEvilPlayer()
{
	Player* player = new Player();
	player->ai = true;

	player->textureRects.clear();

	int skin = 8;

	return player;
}

bool Player::IsInFlashPosition(int value)
{
	// Since the older values of flash position cannot be greater than the newer values - I can do a greater than check to return false

	for (int& p : flashPosition)
	{
		if (p == value) return true;
		if (p > value) return false;
	}
	return false;
}

void Player::DecrementFlash()
{

	for (int& p : flashPosition)
	{
		p--;
		if (p < 0) flashPosition.erase(flashPosition.begin()); // Pop front solution from https://stackoverflow.com/questions/9445327/fast-way-to-implement-pop-front-to-a-stdvector
	}
}

void Player::DrawSegment(sf::Vector2f a, sf::Vector2f b, float animationProgress, sf::RenderWindow& window, Cell cell)
{
	sf::Vector2f target = sf::Vector2f(interpolate(a.x, b.x, animationProgress), interpolate(a.y, b.y, animationProgress));
	shape.setPosition(target);
	shape.setSize(Map::GetTileSizeAsVector2f());

	//shape.setFillColor(sf::Color(head ? 0x106bebff : (colour ? 0x3cf235ff : 0x18eb10ff)));

	// -- DECIDING TEXTURES --
	// TODO can this be optimised please
	bool head = (cell.flags & FLAG_HEAD) == FLAG_HEAD;
	bool colour = (cell.flags & FLAG_COLOUR) == FLAG_COLOUR;
	if (head != isCurrentTextureHead)
	{
		SetUV(head ? TRECT_HEAD : TRECT_BODY_PRIMARY);
		isCurrentTextureHead = head;
		
	}
	if (!head)
	{
		SetUV(colour ? TRECT_BODY_PRIMARY : TRECT_BODY_SECONDARY);
	}

	if (!alive)
	{
		SetUV(victorious ? TRECT_POSITIVE : TRECT_NEGATIVE);
	}
	bool inFlash = IsInFlashPosition(cell.position);
	if (inFlash)
	{
		// Currently does nothing if the snake gets a positive flash
		// TODO: Decide on positive vfx
		SetUV(flashState ? TRECT_POSITIVE : TRECT_NEGATIVE);
	}

	// -- DECIDING TEXTURES --

	window.draw(shape);

	// I Dont want to run this for every segment
	// TODO i should have seperate shapes and functions for drawing the body and the head
	if (head && alive && !inFlash)
	{
		DrawEyes(target, direction, window);
	}
}

// Draws differently
void Player::DrawDuoSegment(sf::Vector2f a, sf::Vector2f b, float animationProgress, sf::RenderWindow& window, Cell cell, Position direction)
{
	// Replace this once i fix other stuff (me when i dont ever replace this)
	if (direction.X > 1) direction.X = 1;
	else if (direction.X < -1) direction.X = -1;

	if (direction.Y > 1) direction.Y = 1;
	else if (direction.Y < -1) direction.Y = -1;

	sf::Vector2f aEnd = a + Map::TranslateExclude(direction);
	sf::Vector2f bStart = b - Map::TranslateExclude(direction);
	
	DrawSegment(a, aEnd, animationProgress, window, cell);
	DrawSegment(bStart, b, animationProgress, window, cell);
}

void Player::DrawEyes(sf::Vector2f position, Position direction, sf::RenderWindow& window)
{
	float radians = atan2((float)direction.Y, (float)direction.X);
	
	SetUV(TRECT_EYES);

	sf::Vector2f tileSize = Map::GetTileSizeAsVector2f();

	sf::Angle angle = sf::radians(radians) + sf::degrees(-90);
	shape.setOrigin(sf::Vector2f(tileSize.x * .5f, tileSize.y * .5f)); // Hacky but it works
	shape.setRotation(angle);
	shape.setPosition(position + sf::Vector2f(tileSize.x * .5f, tileSize.y * .5f));
	shape.setSize(tileSize);
	
	window.draw(shape);
	shape.setOrigin({0,0});

	shape.setRotation(sf::Angle::Zero);

	SetUV(isCurrentTextureHead ? TRECT_HEAD : TRECT_BODY_PRIMARY);
}

void Player::FruitCheck()
{
	for (Fruit* fruit : Game::Current()->GetMap()->GetFruits())
	{
		if (!fruit->IsActive()) continue;
		if (fruit->GetPosition() == position)
		{
			Game::Current()->GetGameMode()->OverlapFruit(this, fruit);
		}
	}
}

void Player::CollisionsCheck()
{
	//  This should be ran AFTER i remove the previous body segment
	// Or i just skip the first array value
	if (body.size() > 1)
	{
		for (int i = 1; i < body.size(); i++)
		{
			if (body.atIndex(i).MyPosition == this->position)
			{
				Game::Current()->GetGameMode()->PlayerHitSelf(this);
			}
		}		
	}
	else if (body.size() == 0)
	{
		Game::Current()->GetGameMode()->PlayerStarved(this);
	}
}

inline float interpolate(float a, float b, float t)
{
	return a + ((b - a) * t);
}

Player::Segment::Segment()
{
	MyPosition = { -1,-1 };
	IsEdge = false;
}

Player::Segment::Segment(Position segPos, bool isEdge)
{
	MyPosition = segPos;
	IsEdge = isEdge;
}

bool Player::Segment::operator==(const Segment& other) const
{
	return this->MyPosition == other.MyPosition;
}

//0x3cf235
//0x18eb10
//0x106beb

Player::Cell::Cell(char flags, short int position)
{
	this->flags = flags;
	this->position = position;
}
