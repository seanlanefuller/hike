#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <random> // Include for random number generation
#include <queue> // For std::queue
#include <limits> // For std::numeric_limits
#include <algorithm> // for std::reverse
#include <unordered_map>
#include <unistd.h>
#include <unistd.h> // For pipe, fork, dup2, close, write, read
#include <sys/wait.h> // For waitpid
#include <sstream>
#include "ollama.hpp"

using namespace std;

// Structure to hold the read and write handles
struct PipeHandles {
    int readPipe;
    int writePipe;
    int originalReadPipe; // Store original read fd
    int originalWritePipe; // Store original write fd
    pid_t pid;
};

ollama::response reply;
bool firstTime = true;

std::string ai(std::string query) {
	std::string model = "deepseek-r1";
	ollama::setReadTimeout(60 * 10);
	ollama::response context;
	std::string modQuery = "You are on an adventure to find the goal area. Please give a 1 word answer. ";
	modQuery += query;
	if (firstTime)
	{
		firstTime = false;
		cout << "query: " << modQuery << endl;
		reply = ollama::generate(model, modQuery);
	} else {
		cout << "query: " << modQuery << endl;
		context = reply;
		reply = ollama::generate(model, modQuery, context);
	}
	cout << "reply: " << reply.as_simple_string() << endl;
	return reply.as_simple_string();
}

struct Area {
	string description;
	map<string, string> exits; // Direction -> Area Name
	map<string, string> hints; // Direction -> Hint
};

bool isStringOnlySpaces(const std::string &str) {
	for (char c : str) {
		if (!std::isspace(c)) {
			return false; // Found a non-space character
		}
	}
	return true; // All characters are spaces
}

// Function to find the shortest path
vector<string> findShortestPath(const map<string, Area> &areas,
		const string &start, const string &goal) {
	map<string, string> cameFrom; // Keep track of how we got to each room
	map<string, int> costSoFar;   // Keep track of the cost to reach each room

	priority_queue<pair<int, string>, vector<pair<int, string>>,
			greater<pair<int, string>> > frontier; // Priority queue for A* (using cost as priority)

	frontier.push( { 0, start }); // Start with a cost of 0
	costSoFar[start] = 0;

	while (!frontier.empty()) {
		string current = frontier.top().second;
		frontier.pop();

		if (current == goal) {
			break; // Found the goal!
		}

		if (areas.count(current)) {
			for (const auto& [direction, next] : areas.at(current).exits) {
				int newCost = costSoFar[current] + 1; // Assuming each move has a cost of 1

				if (!costSoFar.count(next) || newCost < costSoFar[next]) {
					costSoFar[next] = newCost;
					cameFrom[next] = current;
					frontier.push( { newCost, next });
				}
			}
		}
	}

	// Reconstruct the path
	vector<string> path;
	string current = goal;
	while (cameFrom.count(current)) {
		path.push_back(current);
		current = cameFrom[current];
	}
	path.push_back(start); // Add the starting room
	reverse(path.begin(), path.end()); // Reverse to get the correct order

	if (path.size() == 1 && path[0] == start && start != goal) { //No path was found if the path only contains the start and the start is not the goal.
		return {}; // Return an empty path if no path was found.
	}

	cout << "path: ";
	for (auto &step : path) {
		cout << step << " ";
	}
	cout << endl;

	return path;
}

void drawMap(const map<string, Area> &areas, const string &currentArea) {
	// Basic map layout (you can customize this)
	vector<string> map(11, string(21, ' ')); // 11 rows, 21 columns (adjust as needed)

	// Helper function to place an area on the map
	auto placeArea = [&](int row, int col, const string &areaName) {
		if (areas.count(areaName)) {
			map[row][col] = '[';
			map[row][col + 1] = areaName[0];  //First letter of the area name
			map[row][col + 2] = ']';
		}
	};

	// Place areas (Adjust coordinates as needed)
	placeArea(5, 10, currentArea); // Center the current area

	//Example connections:
	if (areas.count(currentArea)) {
		for (const auto& [direction, connectedArea] : areas.at(currentArea).exits) {
			int currentRow = 5;
			int currentCol = 10;
			if (direction == "north" && areas.count(connectedArea)) {
				currentRow = 3;
			} else if (direction == "south" && areas.count(connectedArea)) {
				currentRow = 7;
			} else if (direction == "east" && areas.count(connectedArea)) {
				currentCol = 14;
			} else if (direction == "west" && areas.count(connectedArea)) {
				currentCol = 6;
			}
			placeArea(currentRow, currentCol, connectedArea);
		}
	}
	// Draw the map
	for (const string &row : map) {
		if (!isStringOnlySpaces(row))
			cout << row << endl;
	}
}

void printHelp() {
	cout << "You are on an adventure to find the goal area." << endl;
	cout << "The map is quite contorted an might involve wormholes (whahaha)"
			<< endl;
}

std::string processDirectionShortcut(const std::string &input) {
	// Use an unordered_map for efficient lookup
	static const std::unordered_map<std::string, std::string> shortcuts = { {
			"q", "quit" }, { "n", "north" }, { "s", "south" }, { "e", "east" },
			{ "w", "west" }, { "h", "help" }, { "m", "map" }, { "l", "look" } };

	// Find the input in the map.  If it is not there, return the original input.
	auto it = shortcuts.find(input);
	if (it != shortcuts.end()) {
		return it->second; // Return the full direction name
	} else {
		return input; // Return the original input if no shortcut matches
	}
}

int main() {
	map<string, Area> areas;

	// Define areas with hints
	areas["Start"] = {
			"You are at the starting point.  Paths lead north, east, and west.",
			{ { "north", "Forest" }, { "east", "Cave" }, { "west", "Plains" } },
			{ { "north", "You hear birds chirping to the north." }, { "east",
					"A cool breeze comes from the east." }, { "west",
					"The land stretches flat to the west." } } };
	areas["Forest"] = { "You are in a dense forest. Paths lead south and east.",
			{ { "south", "Start" }, { "east", "River" } }, { { "south",
					"You can see the starting point to the south." }, { "east",
					"You hear the sound of rushing water to the east." } } };
	areas["Cave"] = { "You are in a dark cave. Paths lead west and north.", { {
			"west", "Start" }, { "north", "Mountain" } }, { { "west",
			"You can see light to the west." }, { "north",
			"It's getting colder as you look north." } } };
	areas["Plains"] = {
			"You are on wide open plains. Paths lead east and south.", { {
					"east", "Start" }, { "south", "Village" } }, { { "east",
					"You can see the starting point to the east." }, { "south",
					"You see smoke rising in the distance to the south." } } };
	areas["River"] = { "You are at a flowing river. Paths lead west and north.",
			{ { "west", "Forest" }, { "north", "Bridge" } }, { { "west",
					"The forest is to the west." }, { "north",
					"You see a rickety bridge to the north." } } };
	areas["Mountain"] =
			{
					"You are at the base of a tall mountain. Paths lead south and east.",
					{ { "south", "Cave" }, { "east", "Summit" } }, { { "south",
							"The cave entrance is to the south." }, { "east",
							"The peak looks close to the east." } } };
	areas["Summit"] =
			{
					"You are at the top of a tall mountain. The only way to go down the mountain is west.",
					{ { "west", "Mountain" } }, };
	areas["Village"] = {
			"You are in a quiet village. Paths lead north and east.", { {
					"north", "Plains" }, { "east", "Ruins" } }, { { "north",
					"The plains are to the north." }, { "east",
					"You see crumbling walls to the east." } } };
	areas["Bridge"] = {
			"You are crossing an old bridge. Paths lead south and east.", { {
					"south", "River" }, { "east", "HiddenPath" } }, { { "south",
					"The river flows to the south." }, { "east",
					"A narrow path leads east." } } };
	areas["Ruins"] = { "You are in ancient ruins. Paths lead west and north.", {
			{ "west", "Village" }, { "north", "Goal" } }, { { "west",
			"The village is to the west." }, { "north",
			"A shining beacon is visible to the north." } } };
	areas["HiddenPath"] =
			{ "You are on a hidden path. Paths lead west.", {
					{ "west", "Bridge" } }, { { "west",
					"The bridge is to the west." } } };
	areas["Goal"] = { "You have reached the goal! Congratulations!", { }, { } // No exits or hints from the goal
	};

	string currentArea = "Start";
	random_device rd; // Obtain a random seed from the OS
	mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()

	while (true) {
		string fullQuery = "";
		cout << currentArea << ":" << endl;
		fullQuery += currentArea;
		fullQuery += ": ";
		cout << areas[currentArea].description << endl;
		fullQuery += areas[currentArea].description;

		if (currentArea == "Goal") {
			break;
		}

		// Randomly display ONE hint or no hint at all
		if (!areas[currentArea].hints.empty()) {
			uniform_int_distribution<> distrib(0,
					areas[currentArea].hints.size() - 1); // Define range
			int randomIndex = distrib(gen);

			// Get a random element from the hints map (since maps aren't directly indexable)
			auto it = areas[currentArea].hints.begin();
			advance(it, randomIndex); // Advance the iterator to the random position

			cout << it->second << endl; // Print the hint
			fullQuery += " " + it->second;
		}

		// Show available directions
		cout << "Available directions: ";
		fullQuery += " Available directions: ";
		for (const auto& [direction, nextArea] : areas[currentArea].exits) {
			cout << direction << " ";
			fullQuery += direction;
			fullQuery += ". ";
		}
		cout << endl;

		cout << "Enter direction (or quit): ";
		fullQuery += " Enter direction (or quit) ?";
		string direction;
		cin >> direction;

		// shortcuts
		if (direction == "q")
			direction = "quit";
		else if (direction == "n")
			direction = "north";
		else if (direction == "s")
			direction = "south";
		else if (direction == "e")
			direction = "east";
		else if (direction == "w")
			direction = "west";
		else if (direction == "h")
			direction = "help";
		else if (direction == "m")
			direction = "map";
		else if (direction == "l")
			direction = "look";

		if (direction == "quit") {
			cout << "Leaving so soon?" << endl;
			break;
		} else if (direction == "help") {
			printHelp();
		} else if (direction == "map") {
			drawMap(areas, currentArea);
		} else if (direction == "look") {
			cout << "You look around." << endl;
		} else if (direction == "cheat") {
			findShortestPath(areas, currentArea, "Goal");
		} else if (direction == "ai") {
			std::string direction = ai(fullQuery);
		} else {

			if (areas[currentArea].exits.count(direction)) {
				currentArea = areas[currentArea].exits[direction];
			} else {
				cout << "ERROR: Invalid command." << endl;
			}
		}
		cout << endl;
	}

	cout << "Game Over." << endl;

	return 0;
}
