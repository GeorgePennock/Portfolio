#pragma once

#include "IGameModeDetails.h"
#include "Map.h"
#include "Player.h"
#include "Fruit.h"

#define ID_HIGH_SCORE 100

#define GM_WALLCOL 101
#define GM_SELFCOL 102
#define GM_SNAKESPEED 103
#define GM_LARGEMAP 104
#define GM_COMBO 105


#define GMC_FRUITCOUNT 106

#define GMD_DIFFICULTY 109
#define GMD_NEGATIVES 110

#define GMLM_SCORELIVES 107

#define GMM_STARTHARD 108

class GameMode : public IGameModeDetails
{
protected:
	GameModeSettings* settings;

	int highScore;

	bool achievedHighScore = false;

public:
	GameMode();

	// GENERAL SETTINGS - Cache values when getting

	bool GetTorus();

	bool GetSelfCollision();

	bool GetIsLargeMap();

	float GetSnakeSpeedMultiplier();

	bool GetIsComboMode();

	// END OF SETTINGS

	virtual void LoadSettings();

	virtual int GetFruitCount();

	virtual float GetPlayerSpeed();

	virtual int GetPlayerBodySize();

	virtual float GetComboHoldTime();

	virtual Fruit* NewFruitElement(); // Never let the map decide what fruit should be made, game mode should handle that
	
	virtual void PlayerMovedTile(Player* player);

	virtual void PlayerHitSelf(Player* player);

	virtual void PlayerStarved(Player* player);
	
	virtual void PlayerHitEdgeOfMap(Player* player); // Doesn't count if the walls are on TORUS mode

	virtual void OverlapFruit(Player* player, Fruit* fruit);

	virtual void OnGameOver();

	virtual void RestartGame(std::vector<Element*> elements);

	virtual void MapReady(Map* map);

	virtual void NoValidFruitPositionsRemain();

	virtual void PreTick();

	virtual void PostTick();

	virtual std::string GetGameOverlayDisplay();

	virtual std::wstring GetGameScoreDisplay();

	virtual std::string GetGameOverDisplay();

	std::wstring HighScore();

	GameModeSettings* GetSettings() const override;

	virtual std::string GetName() const;

	// COMBO LOGIC

	static void DefaultSettings(GameModeSettings* settings);

	static void LettersAndMathsSettings(GameModeSettings* settings);

	// Doesn't technically construct the gamemode, but is REQUIRED for running initial gamemode logic
	static GameMode* Construct(GameMode* gameMode);

protected:

	void GivePlayerScoreWithCombo(Player* player);

};

// Start implementing game mode
// Game mode should manage the logic for things like what happens when the player eats fruit
// I do not want the player to decide what happens when they eat a fruit - gamemode should decide that
// I should also like reset fruit position on respawn
// Most resets SHOULD be doable by running the OnPlay() method again for any object :)

