import { Color, Colors } from "./color";
import { lineMath } from "./math/line-math";
import { Arrow, GridPoint, ScreenPoint, WorldPoint } from "./math/math";
import { Vector } from "./math/vector";
import { Player, TurnType } from "./player";
import { Track } from "./track";
import { Event } from "./utils/event";
import { randi } from "./utils/random";

export enum GameState {
	WAITING_PLAYERS,
	START_SELECTION,
	PLAYING,
	STOPPED,
	SPECTATE,
}

class PlayerInfo {
	/** arrows the player has drawn */
	arrows: Arrow[] = [];
	laps = 0;
	/** counts how many times the player crossed the start line (sign depends on direction) */
	startLineCrossCount = 0;

	constructor(public readonly player: Player) {}
}

export class Game {
	private playerInfo_: PlayerInfo[] = [];
	private startPosTaken_: boolean[] = [];
	private state_ = GameState.WAITING_PLAYERS;
	private currentPlayer_ = 0;
	private turnTimer_ = 0;

	readonly onStateChange = new Event<(GameState) => void>();
	readonly onTurnAdvance = new Event<() => void>();

	constructor(private track_: Track, private readonly turnTimeLimit_: number, private readonly targetLaps_: number) {}

	private currentPlayerInfo(): PlayerInfo {
		if (this.currentPlayer_ < 0 || this.currentPlayer_ >= this.playerInfo_.length)
			throw new Error("Invalid player index!");
		return this.playerInfo_[this.currentPlayer_];
	}

	currentPlayer(): Player {
		return this.currentPlayerInfo().player;
	}

	destroy() {
		this.stop();
		this.track_ = null;
	}

	track(): Track {
		return this.track_;
	}

	update(dt: number): void {
		switch (this.state_) {
			case GameState.WAITING_PLAYERS:
				if (this.playerInfo_.length == this.track_.getStartPositions().length)
					// all positions have been filled, start automatically
					this.start();
				return;
			case GameState.STOPPED:
				return;
			case GameState.START_SELECTION:
			case GameState.PLAYING:
				this.turnTimer_ += dt;
				if (this.turnTimer_ >= this.turnTimeLimit_ || this.currentPlayer().actionReady()) {
					// perform action for player, then move on to the next turn
					this.processPlayerSelection();
					this.nextTurn();
				}
		}
	}

	draw(): void {
		if (this.state_ != GameState.START_SELECTION && this.state_ != GameState.PLAYING) return;
		const colors: Color[] = [Colors.PLAYER1, Colors.PLAYER2, Colors.PLAYER3, Colors.PLAYER4, Colors.PLAYER5];
		for (
			let i = (this.currentPlayer_ + 1) % this.playerInfo_.length, n = 0;
			n < this.playerInfo_.length;
			n++, i = (i + 1) % this.playerInfo_.length
		) {
			for (let a of this.playerInfo_[i].arrows) {
				const p1: ScreenPoint = this.track_.grid().gridToScreen(a.from);
				const p2: ScreenPoint = this.track_.grid().gridToScreen(a.to);
				// FIXME draw
				// Painter.paintArrow(p1, p2, 10 * this.track_.grid().getTransform().scale, Math.PI/6, colors[this.players_[i].player.color()]);
			}
		}
	}

	/** resets the entire game, and removes all players */
	reset(): void {
		if (this.state_ != GameState.STOPPED) this.stop();
		this.playerInfo_.splice(0, this.playerInfo_.length);
		this.startPosTaken_.splice(0, this.startPosTaken_.length);
		this.currentPlayer_ = 0;
		this.turnTimer_ = 0;

		this.setState(GameState.WAITING_PLAYERS);
	}

	/** @returns true if player was added and false if it couldn't be added (session is full) */
	addPlayer(player: Player): boolean {
		if (this.playerInfo_.length < this.track_.getStartPositions().length) {
			player.color = this.playerInfo_.length;
			player.offTrackData = { offTrack: false, contourIndex: -1, position: -1 };
			this.playerInfo_.push(new PlayerInfo(player));
			return true;
		} else {
			return false;
		}
	}

	start(): void {
		if (this.state_ != GameState.STOPPED && this.state_ != GameState.WAITING_PLAYERS)
			throw new Error("Game already started!");
		this.startPosTaken_.fill(false, 0, this.track_.getStartPositions().length);
		this.setState(GameState.START_SELECTION);
		this.currentPlayer_ = -1;
		this.nextTurn();
	}

	stop(): void {
		// TODO stop the game and notify all players
		// ...
	}

	state(): GameState {
		return this.state_;
	}

	activePlayer(): Player {
		return this.currentPlayer_ >= 0 && this.currentPlayer_ < this.playerInfo_.length ? this.currentPlayer() : null;
	}

	/** @returns true if the arrow doesn't intersect any player's position */
	pathIsFree(a: Arrow): boolean {
		const p1: WorldPoint = this.track_.grid().gridToWorld(a.from);
		const p2: WorldPoint = this.track_.grid().gridToWorld(a.to);
		for (let i = 0; i < this.playerInfo_.length; i++) {
			if (!this.playerInfo_[i].arrows.length) continue;
			if (a.from == this.playerInfo_[i].arrows.slice(-1)[0].to)
				// we ignore the previous arrow because it would yield a false positive
				continue;
			if (a.to == this.playerInfo_[i].arrows.slice(-1)[0].to)
				// arrow would end up on top of an occupied position
				return false;
			const q: WorldPoint = this.track_.grid().gridToWorld(this.playerInfo_[i].arrows.slice(-1)[0].to);
			if (lineMath.onSegment(p1, q, p2))
				// arrow would cross another player's position
				return false;
		}
		if (this.track_.intersectionsCount(a.from, a.to) > 1)
			// if the arrow intersects the track in more than one place, disallow it
			return false;
		return true;
	}

	/** @returns true if the point is within the track limits */
	isGPointOnTrack(p: GridPoint): boolean {
		const wp: WorldPoint = this.track_.grid().gridToWorld(p);
		return this.isWPointOnTrack(wp);
	}

	/** @returns true if the point is within the track limits */
	isWPointOnTrack(wp: WorldPoint): boolean {
		return this.track_.pointInsidePolygon(wp, 0) && !this.track_.pointInsidePolygon(wp, 1);
	}

	setState(state: GameState): void {
		this.state_ = state;
		this.onStateChange.trigger(state);
	}

	private nextTurn(): void {
		if (this.playerInfo_.length == 0) this.setState(GameState.STOPPED);
		if (this.state_ == GameState.WAITING_PLAYERS || this.state_ == GameState.STOPPED) return;
		if (this.currentPlayer_ >= 0) {
			this.currentPlayer().endTurn();
			if (this.checkWin()) this.currentPlayer().activateTurn(TurnType.TURN_FINISHED);
		}
		while (++this.currentPlayer_ < this.playerInfo_.length && this.currentPlayer().isFinished()) {}
		if (this.currentPlayer_ == this.playerInfo_.length) {
			// all players took their turn for this round
			this.currentPlayer_ = 0;
			if (this.state_ == GameState.START_SELECTION) {
				this.setState(GameState.PLAYING);
			} else {
				while (this.currentPlayer_ < this.playerInfo_.length && this.currentPlayer().isFinished())
					this.currentPlayer_++;
				if (this.currentPlayer_ == this.playerInfo_.length) {
					// all players finished the game
					this.setState(GameState.STOPPED);
					return;
				}
			}
		}
		this.currentPlayer().activateTurn(
			this.state_ == GameState.START_SELECTION ? TurnType.TURN_SELECT_START : TurnType.TURN_MOVE,
		);
		this.currentPlayer().setAllowedVectors(this.getPlayerVectors());
		this.onTurnAdvance.trigger();
	}

	private checkWin(): boolean {
		/*
			count how many laps a player did
			laps is initially 0
			when player crosses the start-line SEGMENT, his laps is incremented/decremented depending on direction
			when player is OUTSIDE of track and crosses the start-line INFINITE LINE, his laps is also incremented/decremented based on direction
				this is to avoid player cheating, exiting the track, going behind start-line and instantly winning when returning back and crossing it
		*/
		if (this.currentPlayerInfo().arrows.length < 2) return false;
		const cross = this.track_.checkStartLineCross(
			this.currentPlayerInfo().arrows.slice(-1)[0].from,
			this.currentPlayerInfo().arrows.slice(-1)[0].to,
			false,
		);
		this.currentPlayerInfo().startLineCrossCount += cross;
		if (cross > 0 && this.currentPlayerInfo().startLineCrossCount > 0) {
			this.currentPlayerInfo().laps++;
			if (this.currentPlayerInfo().laps >= this.targetLaps_) {
				// player won
				return true;
			}
		}
		return false;
	}

	private processPlayerSelection(): void {
		let nextPoint: GridPoint = this.currentPlayer().actionPoint();
		if (!this.validatePlayerMove(nextPoint)) {
			nextPoint = this.autoSelectPlayerVector().to;
		}
		// push the next point
		switch (this.state_) {
			case GameState.START_SELECTION:
				for (let i = 0; i < this.track_.getStartPositions().length; i++) {
					const arrowTip = new GridPoint(
						this.track_.getStartPositions()[i].position.x + this.track_.getStartPositions()[i].direction.x,
						this.track_.getStartPositions()[i].position.y + this.track_.getStartPositions()[i].direction.y,
					);
					if (nextPoint == arrowTip) {
						// player chose position i
						const a = Arrow.fromPointAndDir(
							this.track_.getStartPositions()[i].position,
							this.track_.getStartPositions()[i].direction.x,
							this.track_.getStartPositions()[i].direction.y,
						);
						this.currentPlayerInfo().arrows.push(a);
						this.currentPlayer().lastArrow = a;
						this.startPosTaken_[i] = true;
						return;
					}
				}
				// if we got here, player's selection was none of the valid ones, kick him out
				this.currentPlayer().activateTurn(TurnType.TURN_FINISHED);
				break;
			case GameState.PLAYING: {
				const a = new Arrow(this.currentPlayerInfo().arrows.slice(-1)[0].to, nextPoint);
				this.currentPlayerInfo().arrows.push(a);
				this.currentPlayer().lastArrow = a;
				// check if player went off the track
				if (
					!this.track_.pointInsidePolygon(this.track_.grid().gridToWorld(nextPoint), 0) ||
					this.track_.pointInsidePolygon(this.track_.grid().gridToWorld(nextPoint), 1)
				) {
					if (!this.currentPlayer().offTrackData.offTrack) {
						// player is off track, set his speed to zero
						this.currentPlayer().offTrackData.offTrack = true;
						[this.currentPlayer().offTrackData.contourIndex, this.currentPlayer().offTrackData.position] =
							this.track_.computeCrossingIndex(a.from, a.to);
						this.currentPlayerInfo().arrows.push(new Arrow(nextPoint, nextPoint));
						this.currentPlayer().lastArrow = this.currentPlayerInfo().arrows.slice(-1)[0];
					}
				} else if (this.currentPlayer().offTrackData.offTrack) {
					// player went back on track
					this.currentPlayer().offTrackData.offTrack = false;
				}
				// check if arrow crosses the infinite start-line outside of the track
				const intersect = new WorldPoint(0, 0);
				const crossSign: number = this.track_.checkStartLineCross(a.from, a.to, true, intersect);
				if (crossSign != 0) {
					if (!this.isWPointOnTrack(intersect)) {
						this.currentPlayerInfo().startLineCrossCount += crossSign;
					}
				}
				break;
			}
			default:
				throw new Error("what are you trying to do?");
		}
	}

	private validatePlayerMove(move: GridPoint): boolean {
		const validMoves: Arrow[] = this.getPlayerVectors();
		for (let a of validMoves) {
			if (move == a.to) return true;
		}
		return false;
	}

	private getPlayerVectors(): Arrow[] {
		const ret: Arrow[] = [];
		if (this.state_ == GameState.START_SELECTION) {
			for (let i = 0; i < this.track_.getStartPositions().length; i++)
				if (!this.startPosTaken_[i])
					ret.push(
						Arrow.fromPointAndDir(
							this.track_.getStartPositions()[i].position,
							this.track_.getStartPositions()[i].direction.x,
							this.track_.getStartPositions()[i].direction.y,
						),
					);
		} else if (this.state_ == GameState.PLAYING) {
			const lastDir: Vector = this.currentPlayerInfo().arrows.slice(-1)[0].direction();
			const lastP: GridPoint = this.currentPlayerInfo().arrows.slice(-1)[0].to;
			const center: Arrow = Arrow.fromPointAndDir(lastP, lastDir.x, lastDir.y);
			for (let i = -1; i <= 1; i++)
				for (let j = -1; j <= 1; j++) {
					const a = new Arrow(center.from, new GridPoint(center.to.x + j, center.to.y + i));
					const maxLength = this.currentPlayer().offTrackData.offTrack ? 1 : 6;
					if (a.length() > maxLength) continue;
					if (!this.pathIsFree(a)) continue;
					if (this.currentPlayer().offTrackData.offTrack) {
						// player was off-track, must check if the current arrow would get him back on
						// and if it does, only allow it if it re-enters the track behind his exit position
						const arrowTipW: WorldPoint = this.track_.grid().gridToWorld(a.to);
						if (
							this.track_.pointInsidePolygon(arrowTipW, 0) &&
							!this.track_.pointInsidePolygon(arrowTipW, 1)
						) {
							const crossIndex: number[] = this.track_.computeCrossingIndex(a.from, a.to);
							if (
								(crossIndex[0] - this.currentPlayer().offTrackData.position) *
									this.track_.polyDirection(crossIndex[0]) >
								0
							)
								continue;
						}
					}
					ret.push(a);
				}
		} else throw new Error("Invalid state");
		return ret;
	}

	private autoSelectPlayerVector(): Arrow {
		if (!this.currentPlayerInfo().arrows.length) throw new Error("Invalid state!");
		const lastVector: Arrow = this.currentPlayerInfo().arrows.slice(-1)[0];
		const validMoves: Arrow[] = this.getPlayerVectors();
		// try to keep the same vector as before if it's valid
		for (const a of validMoves)
			if (
				a.direction().x == lastVector.direction().x &&
				a.direction().y == lastVector.direction().y &&
				a.length() == lastVector.length()
			)
				return a;
		// same vector could not be selected, try one with the same direction
		const sameDir: Arrow[] = [null, null];
		let nSame = 0;
		for (const a of validMoves)
			if (a.direction().x == lastVector.direction().x && a.direction().y == lastVector.direction().y)
				sameDir[nSame++] = a;
		if (nSame > 0) {
			if (nSame == 1 || sameDir[0].length() < sameDir[1].length())
				// choose the shorter (or the only) one
				return sameDir[0];
			else return sameDir[1];
		}
		// no same-direction vector, must steer left or right, choose randomly
		return validMoves[randi(validMoves.length)];
	}
}
