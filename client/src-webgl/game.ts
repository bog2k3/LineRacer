import { PlayerController } from "./entities/player.controller";
import { GlobalState } from "./global-state";
import { logprefix } from "./joglfw/log";
import { Vector } from "../src/math/vector";
import { Event } from "../src/utils/event";
import { Entity } from "./joglfw/world/entity";
import { World } from "./joglfw/world/world";
import { PlayerInputHandler } from "./player-input-handler";

const console = logprefix("Game");

export enum GameState {
	CONFIGURE_TERRAIN,
	SPECTATE,
	PLAY,
}

export class Game {
	state = GameState.CONFIGURE_TERRAIN;
	playerController = new PlayerController();
	playerInputHandler = new PlayerInputHandler();

	/** these are entities that are never destroyed */
	readonly godEntities: Entity[] = [];

	onStateChanged = new Event<(state: GameState, prevState: GameState) => void>();

	constructor() {
		this.setupNetworkManagerFactories();
	}

	async initialize(): Promise<void> {
		console.log("Initializing");

		console.log("Ready");
	}

	update(dt: number): void {
		this.playerInputHandler.update(dt);
		World.getInstance().update(dt);
	}

	async setState(state: GameState): Promise<void> {
		if (state === this.state) {
			return;
		}
		const oldState = this.state;
		this.state = state;
		if (oldState !== GameState.CONFIGURE_TERRAIN && state === GameState.CONFIGURE_TERRAIN) {
			await this.stop();
		} else if (oldState === GameState.CONFIGURE_TERRAIN && state !== GameState.CONFIGURE_TERRAIN) {
			await this.start();
		}
		if (this.state === GameState.PLAY) {
		}
		if (this.state === GameState.SPECTATE) {
		}
		this.onStateChanged.trigger(state, oldState);
	}

	// -------------------------- PRIVATE AREA ------------------------------- //

	private start(): Promise<void> {
		console.log("Starting game...");

		console.log("Game started.");
		return Promise.resolve();
	}

	private stop(): Promise<void> {
		console.log("Stopping game...");

		console.log("Game stopped.");
		return Promise.resolve();
	}
}
