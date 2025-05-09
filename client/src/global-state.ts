import { Game } from "./game";
import { Grid } from "./grid";
import { HtmlInputHandler } from "./input";
import { PlayerList } from "./player-list";

export const GlobalState = {
	playerName: "",
	// world: World;
	grid: null as Grid,
	game: null as Game,
	inputHandler: null as HtmlInputHandler,
	isPaused: false,
	playerList: new PlayerList(),
};
