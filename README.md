An ollama AI is playing a text adventure along with you.

I only have DeepSeek-r1 on my PC right now because it is small.

The code uses the all-in-one hpp ollama API.
https://github.com/jmont-dev/ollama-hpp

You must have ollama installed and a model downloaded.

You can enter commands or allow the AI to choose what to do.

I want to keep developing this and adding other commands and activities.
I think it would be awesome to have two AIs meet, etc.

The adventure game itself is pretty self-explanatory.
There are areas connected to other areas with directional commands.
There are a few hidden commands like:

ai = Have the AI tell you what move to make for it.
fullai = Let the AI take the wheel and run intil it quits or finds the goal.
map = draw a simple map from the current room.
cheat = Use the A* algorithm to tell you the rooms from here to the goal.
help = Display a few lines of help.

So far, the AI hasn't discovered any of those commands. :)
Contributions are welcome!

Sean Lane Fuller
