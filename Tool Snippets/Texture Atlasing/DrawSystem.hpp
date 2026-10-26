#pragma once

#include <string>
#include <unordered_map>
#include <set>
#include "Vector.h"
#include "Event.h"

class DrawShelf;
class StaticDrawShelf;

class DrawSystem;

// If i could private this i would be very happy, but i cannie :(
namespace sf { class RectangleShape; class Texture; class VertexArray; class Image; }

struct TexId
{
private:
	int identifier;

public:
	TexId()
	{
		identifier = -1;
	}

	//TexId(TexId& other)
	//{
	//	this->identifier = other.identifier;
	//}

	bool operator==(const TexId& other) const;
	friend DrawSystem;
};

// The animation direction that the sprite sheet follows when being looped through.
enum AnimDirection
{
	// Does not animate, returns the entire sprite rect
	kStatic,

	// Starting on the left sprite and continues linearly right. Once it reaches the end it goes through the next vertical row (if it exists), sets back to the start afterwards.
	kHorizontal,

	// Starting on the left sprite and continues linearly right. Once it reaches the end it goes through the next vertical row (if it exists), bounces back to the start, going in reverse once reaching the end.
	kHorizontalPing,

	// Starting on the top sprite and continues linearly down. Once it reaches the end it goes through the next horizontal column (if it exists), sets back to the start afterwards.
	kVertical,

	// Starting on the top sprite and continues linearly down. Once it reaches the end it goes through the next horizontal column (if it exists), bounces back to the start, going in reverse once reaching the end.
	kVerticalPing,

	// Picks a random sprite from the entire sheet, no realistic applied pattern.
	kRandom
};

// Has to be a .hpp to be able to use "unsigned int" annoyingly.

// The idea of the draw class is to entirely seperate the ImGui and the SFML includes, so its basically a wrapper.
// It also brings all of the controls into one place instead of splitting it across both libraries.

// I dont want to include SFML anywhere, I want it to be hidden.

// I want to try setup an ECS system for draw calls, where it sends a structure of what to draw and stores them to speed things up.
class DrawSystem
{
public:
	enum StaticOrder
	{
		Pre,
		Post
	};
private:

	bool isInitialised;

	static int activeDrawCalls;

	static float targetFPS;

	struct DrawInst
	{
		sf::RectangleShape* shape;
		DrawShelf* draw;

		DrawSystem* sys;

		sf::Texture* activeTexture;

		void UpdPos(Vectorf);
		void UpdRot(float);
		void UpdSiz(Vectorf);
		void UpdPiv(bool);
		void UpdTex(TexId);
	};

	struct Tex
	{
		sf::Texture* texture;
		Recti subrect = { 0,0,0,0 };
	};

	struct TexAnim
	{
		sf::Texture* texture;
		Recti subrect = { 0,0,0,0 };
		int xTiles = 1;
		int yTiles = 1;
		AnimDirection direction = kStatic;
	};

	struct Atlas
	{
		std::set<sf::Texture*> existingTextures;
		sf::Texture* texture;
		sf::Image* image;

		std::unordered_map<int, Recti> rects;

		DrawSystem* sys;

		void GenerateStaticAtlas(std::set<int> ids);

		~Atlas();
	};

	int texIdIncrPtr = 0;

	// All texture references for normal sprite drawing

	std::unordered_map<int, Tex> allImages;
	std::unordered_map<int, TexAnim> allAnimations;
	std::unordered_map<std::string, sf::Texture*> allTextures;

	std::unordered_map<int, bool> isAnimatedTextures;

	// All draw shelf references for what to draw and when

	std::unordered_map<DrawShelf*, DrawInst> drawableInstances;
	std::set<DrawShelf*> drawables;
	std::set<StaticDrawShelf*> preDrawStatics;
	std::set<StaticDrawShelf*> postDrawStatics;

	// All atlas generation for static drawn assets
	Atlas preDrawAtlas;
	Atlas postDrawAtlas;
	std::unordered_map<int, std::string> texFileNames;


	// Event for pre drawing things that like fires literally right b4 window.draw() dawg
	Event preDrawEvent;

	// After preDrawEvent, meant for UI
	Event uiPreDrawEvent;

	bool wantsToBakePreStatics;
	bool wantsToBakePostStatics;

	bool shouldDrawPreStatics = false;
	bool shouldDrawPostStatics = false;

	Vectorf tempMouseDelta;
	Vectorf oldMouse;

	void ActuallyBakePreStatics();
	void ActuallyBakePostStatics();

	void LoadTexture(std::string texture);

	void BakeStaticsOf(std::set<StaticDrawShelf*>& shelves, sf::VertexArray& target, bool isPreDraw);

	static DrawSystem* ptr;

public:
	DrawSystem(std::string title, unsigned int windowWidth, unsigned int windowHeight);

	~DrawSystem();

	bool IsOpen();

	void ForceClose();

	bool IsInitialised();

	// Registers a draw shelf to the rendering system, letting it be easily drawn
	void RegisterShelf(DrawShelf* shelf);

	// Remove the registration of a draw shelf
	void DeregisterShelf(DrawShelf* shelf);

	// Registers a static draw shelf to the rendering system, once registered changes applied do not take affect unless BakeStatics() is ran again. The Automatically runs BakeStatics() after registering
	void RegisterStaticShelf(StaticDrawShelf* shelf, StaticOrder order);

	// Remove the registration of a static draw shelf and automatically runs BakeStatics()
	void DeregisterStaticShelf(StaticDrawShelf* shelf, StaticOrder order);

	// Draws a line from the starting point to the goal point, should mostly be used for debugging.
	void DrawLine(Vectorf from, Vectorf to, int thickness = 1, int hue = 0xffffffff);

	// Draws a box from the starting point to the goal point, should mostly be used for debugging.
	void DrawBox(Vectorf from, Vectorf to, int thickness = 1, int hue = 0xffffffff);

	// Draws a circle around the center point with a radius, should mostly be used for debugging.
	void DrawCircle(Vectorf from, float radius, int density = 16, int thickness = 1, int hue = 0xffffffff);

	// Draws a connected set of lines from the start to the end, and has an option to complete the loop, should mostly be used for debugging.
	void DrawLineStrip(std::vector<Vectorf> points, bool connectEndToStart = false, int thickness = 1, int hue = 0xffffffff);

	// TODO: Could i thread this?
	// Refreshes the static shelf bake and textures, can be an intensive operation depending on how many statics are int the world. Does not run instantly, once ran it will wait until pre-render before baking.
	void BakeStatics(StaticOrder staticOrder);
	
	// An event that is fired right before drawing, and after every other possible draw call. Its better to use the DrawShelves still, but for situations that can't, this is another good bet.
	Event* OnPreDrawEvent();

	// An event that is fired after all the map should have been drawn, should only be used for UI based draw calls.
	Event* OnUiPreDrawEvent();

	// Creates a button on the screen that returns whether it has been clicked, should mostly be used for debugging.
	bool DrawButton(Vectorf origin, Vectorf size, int hue = 0xffffffff);


	Vectorf TEMP_MouseDelta();
	bool TEMP_MouseStatus();
	

public:
	// Runs Pre-Rendering Methods
	void PreRender();
	// Runs Post-Rendering Methods
	void Render();
	// Polls Game Events
	void PollEvents();
	// Draws all active draw shelves & statics that are registered
	void DrawShelves();

public:

	// Set the target FPS of the draw system, ImGui & Debugging
	void SetTargetFPS(int fps);
	// Returns the target FPS
	static float GetTargetFPS();

	// The number of times a draw call method is ran
	static int GetDrawCallCount();
	// Returns how long its been since this method was last ran (deltaTime lol)
	static float TimeSinceLastSample();
	// Returss the actual game time
	static float Time();

	static DrawSystem* I();

public:

	TexId LoadImage(std::string file, Recti subRect = { 0,0,0,0 });
	TexId LoadAnimation(std::string file, Recti subRect = { 0,0,0,0 }, int xTiles = 1, int yTiles = 1, AnimDirection animation = kStatic);

};
