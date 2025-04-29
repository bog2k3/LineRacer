import { Vector } from "./math/vector";

export class Color {
	constructor(public r: number, public g: number, public b: number, public a = 255) {}

	toVec(): Vector {
		return new Vector(this.r / 255.0, this.g / 255.0, this.b / 255.0, this.a / 255.0);
	}

	set(context: CanvasRenderingContext2D) {
		context.strokeStyle = `rgba(${this.r}, ${this.g}, ${this.b}, ${this.a / 255})`;
	}
}

export namespace Colors {
	export const RED = new Color(255, 0, 0);
	export const RED_TR = new Color(255, 0, 0, 64);
	export const GREEN = new Color(0, 255, 0);
	export const GREEN_TR = new Color(0, 255, 0, 64);
	export const BLUE = new Color(0, 0, 255);
	export const BLUE_TR = new Color(0, 0, 255, 64);

	export const BACKGROUND = new Color(0xff, 0xf0, 0xe6);
	export const GRID = new Color(190, 210, 255);
	export const MOUSE_POINT = new Color(128, 165, 255);
	export const TRACK = new Color(10, 35, 90);
	export const TRACK_TRANSP = new Color(10, 35, 90, 64);
	export const TRACK_SNAP = new Color(10, 145, 225);
	export const STARTLINE = new Color(0, 128, 0);

	export const OUT_TRACK_MOVE = new Color(210, 30, 0);
	export const VALID_MOVE = new Color(0, 210, 110);
	export const INVALID_MOVE = new Color(40, 40, 40, 150);

	export const UNCONFIRMED_ARROW = new Color(40, 40, 40, 95);
	export const PLAYER1 = new Color(0, 165, 235);
	export const PLAYER2 = new Color(235, 28, 36);
	export const PLAYER3 = new Color(34, 177, 76);
	export const PLAYER4 = new Color(255, 201, 14);
	export const PLAYER5 = new Color(163, 73, 164);

	export const BUTTON_BORDER = new Color(5, 100, 110, 128);
	export const BUTTON_FILL = new Color(80, 215, 235);
	export const BUTTON_FILL_HOVER = new Color(135, 225, 240);
	export const BUTTON_FILL_PRESSED = new Color(25, 145, 215);
	export const BUTTON_TEXT = new Color(0, 50, 50);
}
