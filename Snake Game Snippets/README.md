# Snake Game Snippets

## Code Structure

This is a generalised code structure, it does not contain a list of every existing class in the project.

    Game*
        Resource* 
        Audio*
        GameMode*
        GameModeSettings*
        Combo*
        UI*
        Map*
            Element*[]
                Player*
                Fruit*
                Background*

**Game:** Base class for the entire game.\
**Resource:** Textures & Sound Files.\
**Audio:** Plays audio in channels.\
**GameMode:** Rules for the game and what to do on specific events.\
**GameModeSettings:** Modifications for each game mode, such as how many fruit to spawn.\
**Combo:** Controls scoring for when you pick up multiple fruit.\
**UI:** Draws all text & menus to the screen.\
**Map:** Container for all the parts of the game map.\
**Element:** Base class for any parts of the game.\
**Player:** Controller for the player.\
**Fruit:** A collectable for the player.\
**Background:** A drawable grid background with a dynamic size.\

The Game runs a game loop which runs update and draw calls seperate for each object.

## Gamemode Class
All game logic and events are passed through the gamemode class, this lets me easily manipulate what to do depending on the specific rules of each gamemode, without having the project littered with IF statements. I can control what happens when fruit is picked up, every time the player moves, and lots of other events.

## Element Class
The element class is used as a base class for all the game's components to inherit from. Things such as the fruit, player and the background all inherit from element and can ovveride any virtual events to connect to different parts of the game loop. You can also override its drawing method and change how it draws itself, instead of just using the default shape drawing. 

## Fruit Class
The fruit class is mainly just a stored position to check if the player has eaten it. It also has its own repostioning algorithm to avoid overlapping the player or other fruits.

## UI Class
The UI Class can hold arrays of UI elements, aswell as subpages to draw inside of itself. The subpage system lets you easily create layers of UI to back in and out of menus.

## Split Image Class
The UI split image class its a custom drawn shape, which pushes the borders of an image to the sides and streches the interior using a custom vertex shape. Its mainly used in this project to make the pushable buttons any size.