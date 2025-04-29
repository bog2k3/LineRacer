import { Game } from "./game";
import { HtmlInputHandler } from "./input";
import { World } from "./joglfw/world/world";
import { PlayerList } from "./player-list";

export namespace GlobalState {
	export let playerName: string;
	export let world: World;
	export let game: Game;
	export let inputHandler: HtmlInputHandler;
	export let isPaused = false;
	export let playerList = new PlayerList();
}
