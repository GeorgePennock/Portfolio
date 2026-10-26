#pragma once

#include "SFML/Graphics.hpp"
#include "UIElement.h"
#include "AssetPack.h"

// Include all UI types so the only reference needed for child UIs is UI.h
#include "UIImage.h"
#include "UIText.h"
#include "UIButton.h"

#include <vector>
#include "Callback.h"

class UI
{
private:
	
	std::vector<UIElement*> uiElements;

	std::vector<UIButton*> pressedButtons;

	UI* overridingUI;

	// Leaves every child UI
	void RemoveChild();

public:
	bool RecieveInputs;

	void Draw(sf::RenderWindow& renderWindow);

	virtual void OnReEntry();

	virtual void PreDraw(); // Ran before rendering the UI

	virtual void PreDeath(); // The destructor is evil

	void RecieveInputEvent(Position position, bool isRelease);

	UIElement* RegisterUIElement(UIElement* reference);

	// Lets you enter a child UI, such as pages
	void EnterUI(UI* child);

	// Leaves the child UI
	void BackoutUI();

	void ReccursiveTextureReload();

	UI* Deepest();


	bool HasChildUI();

	UI();
	~UI();
};

