#include "UI.h"
#include "Debugging.h"

void UI::Draw(sf::RenderWindow& renderWindow)
{
	if (overridingUI != nullptr) { overridingUI->Draw(renderWindow); return; }

	PreDraw();
	for (UIElement* element : uiElements)
	{
		element->Draw(renderWindow);
	}
}

void UI::PreDraw()
{
}

void UI::PreDeath()
{
}

void UI::OnReEntry()
{

}

void UI::RecieveInputEvent(Position position, bool isRelease)
{
	if (overridingUI != nullptr) { overridingUI->RecieveInputEvent(position,isRelease); return; }
	if (RecieveInputs == false) return;

	if (!isRelease)
	{
		// Go in inverse order (render order)
		pressedButtons.clear();
		for (int i = std::size(uiElements) - 1; i >= 0; i--)
		{
			UIButton* button = dynamic_cast<UIButton*>(uiElements[i]);
			if (button != nullptr && button->RecieveInputs)
			{
				if (button->PointInBounds(position))
				{
					button->OnMouseDown();
					pressedButtons.push_back(button);
					break;
				}
			}
		}
	}
	else
	{
		if (pressedButtons.empty() == false)
		{
			for (UIButton* b : pressedButtons)
			{
				b->OnMouseRelease();
			}
		}
		// Go in inverse order (render order)
		for (int i = std::size(uiElements) - 1; i >= 0; i--)
		{
			UIElement* element = uiElements[i];
			if (element->RecieveInputs)
			{
				if (element->PointInBounds(position))
				{
					element->OnInteract();
					break;
				}
			}
		}
	}
	
}

UIElement* UI::RegisterUIElement(UIElement* reference)
{
	uiElements.push_back(reference);
	return reference;
}

void UI::EnterUI(UI* child)
{
	// DisQUSTing reccusion - if it enters an infnite loop ur cooked
	if (overridingUI != nullptr)
	{
		overridingUI->EnterUI(child);
	}
	else
	{
		overridingUI = child;
	}
}

void UI::BackoutUI()
{
	if (overridingUI == nullptr) return;

	// DisQUSTing reccusion - if it enters an infnite loop ur cooked
	if (overridingUI->HasChildUI())
	{
		overridingUI->BackoutUI();
	}
	else
	{
		OnReEntry();
		RemoveChild();
	}
}

void UI::ReccursiveTextureReload()
{
	if (overridingUI != nullptr)
	{
		overridingUI->ReccursiveTextureReload();
	}

	for (UIElement* element : uiElements)
	{
		element->TextureUpdate();
	}
}


UI* UI::Deepest()
{
	if (overridingUI == nullptr) return this;
	
	if (overridingUI->HasChildUI()) return overridingUI->Deepest();
	else return overridingUI;
}


void UI::RemoveChild()
{
	// TODO
	if (overridingUI == nullptr) return;
	overridingUI->PreDeath();
	delete overridingUI;
	overridingUI = nullptr;
}

bool UI::HasChildUI()
{
	return overridingUI != nullptr;
}

UI::UI()
{
	RecieveInputs = true;
}

UI::~UI()
{
	Debugging::Log("UI Dying!");
	PreDeath();
	if (overridingUI != nullptr) delete overridingUI;
	// Do i need to deconstruct here?
	// Yes its a pointer but its a pointer to somewhere within a stack
	// I  think i need to destruct elements
}

