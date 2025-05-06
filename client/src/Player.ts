import { Arrow, GridPoint } from "./math/math";

export enum PlayerType {
	TYPE_HUMAN,
	TYPE_AI,
	TYPE_NETWORK,
}

export enum TurnType {
	TURN_INACTIVE,
	TURN_SELECT_START,
	TURN_MOVE,
	TURN_FINISHED,
}

export type PlayerOfftrackData = {
	offTrack: boolean;
	contourIndex: number;
	position: number;
};

export class Player {
	color: number;

	constructor(public readonly type: PlayerType) {}

	/** @returns true if the player has finished performing an action */
	actionReady(): boolean {
		return this.hasAction_;
	}
	/** @returns the gridPoint where the last action happened */
	actionPoint(): GridPoint {
		return this.selectedPoint_;
	}
	/** clears the pending action and cancels waiting for the action, also takes control away from the player */
	endTurn(): void {
		this.turn_ = TurnType.TURN_INACTIVE;
	}
	/** called when the player's turn begins and he gets control to do his action */
	activateTurn(type: TurnType): void {
		this.turn_ = type;
		this.hasAction_ = false;
	}
	setAllowedVectors(vectors: Arrow[]): void {
		this.allowedVectors_ = vectors;
	}
	setOffTrackData(data: PlayerOfftrackData): void {
		this.offTrackData_ = data;
	}
	setLastArrow(arrow: Arrow): void {
		this.lastArrow_ = arrow;
	}
	selectNextPoint(point: GridPoint): void {
		this.selectedPoint_ = point;
	}
	confirmNextPoint(): void {
		if (!this.isTurnActive() || this.hasAction_) return;
		this.hasAction_ = true;
	}
	isTurnActive(): boolean {
		return this.turn_ != TurnType.TURN_INACTIVE && this.turn_ != TurnType.TURN_FINISHED;
	}
	isFinished(): boolean {
		return this.turn_ == TurnType.TURN_FINISHED;
	}
	validMoves(): Arrow[] {
		return this.allowedVectors_;
	}
	lastArrow(): Arrow {
		return this.lastArrow_;
	}
	offTrackData(): PlayerOfftrackData {
		return this.offTrackData_;
	}

	private color_ = -1;
	private turn_ = TurnType.TURN_INACTIVE;
	private hasAction_ = false;
	private selectedPoint_ = new GridPoint(0, 0);
	private allowedVectors_: Arrow[] = [];
	private lastArrow_ = new Arrow(new GridPoint(0, 0), new GridPoint(0, 0));
	private offTrackData_: PlayerOfftrackData = { offTrack: false, contourIndex: -1, position: -1 };
}
