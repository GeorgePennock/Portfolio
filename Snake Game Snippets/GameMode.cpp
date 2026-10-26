#include "GameMode.h"

#include "Game.h"

GameMode::GameMode()
{

}

void GameMode::LoadSettings()
{
	settings = new GameModeSettings();

	DefaultSettings(settings);

	settings->RegisterSetting(GMC_FRUITCOUNT, GameModeSettings::Setting("Fruit Count", 1, 1, 10));

	settings->Read(GetName());


	highScore = settings->Value(ID_HIGH_SCORE);
	Debugging::Log("Getting high score from save file " + std::to_string(highScore));
}

int GameMode::GetFruitCount()
{
	return settings->Value(GMC_FRUITCOUNT);
}

float GameMode::GetPlayerSpeed()
{
	return 4 * GetSnakeSpeedMultiplier();
}

int GameMode::GetPlayerBodySize()
{
	return 5; // 5
}

float GameMode::GetComboHoldTime()
{
	return 4.0f;
}

bool GameMode::GetTorus()
{
	return !settings->Value(GM_WALLCOL);
}

bool GameMode::GetSelfCollision()
{
	return settings->Value(GM_SELFCOL);
}

bool GameMode::GetIsLargeMap()
{
	return settings->Value(GM_LARGEMAP);
}

float GameMode::GetSnakeSpeedMultiplier()
{
	return (settings->Value(GM_SNAKESPEED) * .5f);
}

bool GameMode::GetIsComboMode()
{
	return settings->Value(GM_COMBO);
}

Fruit* GameMode::NewFruitElement()
{
	return new Fruit();
}

void GameMode::PlayerMovedTile(Player* player)
{
}

void GameMode::PlayerHitSelf(Player* player)
{
	if (!GetSelfCollision()) return;
	OnGameOver();
}

void GameMode::PlayerStarved(Player* player)
{
	Debugging::Log("Player hit self");
	OnGameOver();
}

void GameMode::PlayerHitEdgeOfMap(Player* player)
{
	Debugging::Log("Player hit edge of map");
	// Torus map checks are ran in player - no need to implemented the GetTorus() here
	OnGameOver();
}

void GameMode::OverlapFruit(Player* player, Fruit* fruit)
{
	Game::Current()->GetAudio()->Play(Audio::ESound::eEat);
	if (!fruit->Reposition(player, Game::Current()->GetMap()->GetFruits()))
	{
		fruit->SetActive(false);

		bool gameOver = true;
		for (Fruit* f : Game::Current()->GetMap()->GetFruits())
		{
			if (f->IsActive())
			{
				gameOver = false;
				break;
			}
		}
		if (gameOver)
		{
			Debugging::Log("No more fruit positions, you win!");
			GivePlayerScoreWithCombo(player);
			NoValidFruitPositionsRemain();
			return;
		}
	}

	GivePlayerScoreWithCombo(player);
}

void GameMode::OnGameOver()
{
	if (Game::Current()->GetMap()->GetPlayer()->IsDead()) return; // Attempt to prevent duplicate game overs

	int score = Game::Current()->GetMap()->GetPlayer()->GetScore();
	
	achievedHighScore = false;
	if (score > highScore)
	{
		Game::Current()->GetAudio()->Play(Audio::ESound::eHighScore);
		highScore = score;
		
		// Could be optimised to only write on destruct
		settings->Set(ID_HIGH_SCORE, highScore);
		settings->Write(GetName());

		achievedHighScore = true;
	}

	Debugging::Log("Got high score? " + std::to_string(achievedHighScore));
	Game::Current()->GetMap()->GetPlayer()->SetAlive(false);
	Game::Current()->UITargetDeath();
	Game::Current()->GetCombo()->CancelCombo();

	Game::Current()->GetAudio()->Play(Audio::ESound::eDeath);
}

void GameMode::RestartGame(std::vector<Element*> elements)
{
	for (Element* element : elements) element->ResetElement();
}

void GameMode::MapReady(Map* map)
{

}

void GameMode::NoValidFruitPositionsRemain() // Depending on the amount of active fruit this means something different
{
	if (Game::Current()->GetMap()->GetPlayer()->IsDead()) return; // Attempt to prevent duplicate game overs

	int score = Game::Current()->GetMap()->GetPlayer()->GetScore();

	achievedHighScore = false;
	if (score > highScore)
	{
		Game::Current()->GetAudio()->Play(Audio::ESound::eHighScore);
		highScore = score;

		// Could be optimised to only write on destruct
		settings->Set(ID_HIGH_SCORE, highScore);
		settings->Write(GetName());

		achievedHighScore = true;
	}

	Debugging::Log("Got high score? " + std::to_string(achievedHighScore));
	Game::Current()->GetMap()->GetPlayer()->SetAlive(false, true);
	Game::Current()->UITargetWin();
	Game::Current()->GetCombo()->CancelCombo();

	Game::Current()->GetAudio()->Play(Audio::ESound::eVictory);
}

void GameMode::PreTick()
{
}

void GameMode::PostTick()
{
}

std::string GameMode::GetGameOverlayDisplay()
{
	return "Collect fruit!";
}

std::wstring GameMode::GetGameScoreDisplay()
{
	return std::to_wstring(Game::Current()->GetMap()->GetPlayer()->GetScore());
}

std::string GameMode::GetGameOverDisplay()
{
	return achievedHighScore ? "High Score!" : "Game Over!";
}

std::wstring GameMode::HighScore()
{
	return L"\u2605 " + std::to_wstring(highScore);
}

IGameModeDetails::GameModeSettings* GameMode::GetSettings() const
{
	return settings;
}

std::string GameMode::GetName() const
{
	return "Classic";
}

// Adds in the settings that every game mode has
void GameMode::DefaultSettings(GameModeSettings* settings)
{
	// Storing the high score in the settings file - faster as I need to load less files and system alr designed
	settings->RegisterSetting(ID_HIGH_SCORE, GameModeSettings::Setting("High Score", 0, true));

	settings->RegisterSetting(GM_WALLCOL, GameModeSettings::Setting("Wall Collisions", true));
	settings->RegisterSetting(GM_SELFCOL, GameModeSettings::Setting("Self Collisions", true));
	settings->RegisterSetting(GM_SNAKESPEED, GameModeSettings::Setting("Snake Speed", 2, 1, 3));
	settings->RegisterSetting(GM_LARGEMAP, GameModeSettings::Setting("Large Map", false));
	settings->RegisterSetting(GM_COMBO, GameModeSettings::Setting("Combo Mode", true));
}

void GameMode::LettersAndMathsSettings(GameModeSettings* settings)
{
	settings->RegisterSetting(GMLM_SCORELIVES, GameModeSettings::Setting("Length as Lives", false));
}

GameMode* GameMode::Construct(GameMode* gameMode)
{
	gameMode->LoadSettings();
	return gameMode;
}

void GameMode::GivePlayerScoreWithCombo(Player* player)
{
	Combo* combo = Game::Current()->GetCombo();
	
	combo->OnComboTrigger();

	player->GainScore(combo->CurrentCombo() >= 3 ? combo->CurrentCombo() - 2 : 1);

}
