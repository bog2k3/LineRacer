import { GlobalState } from "../global-state";

export let renderContext: CanvasRenderingContext2D;
let nextTextY = 15;

export type RenderConfig = {
	drawDebugKeys: boolean;
	drawDebugPlayerList: boolean;
	drawSpectateText: boolean;
};

export function setRenderContext(context: CanvasRenderingContext2D): void {
	renderContext = context;
}

export function render(config: RenderConfig): void {
	renderContext.clearRect(0, 0, renderContext.canvas.width, renderContext.canvas.height);

	GlobalState.grid.draw();

	nextTextY = 15; // reset
	if (config.drawDebugKeys) {
		drawDebugKeys();
	}
	if (config.drawDebugPlayerList) {
		drawDebugPlayerList();
	}
	if (config.drawSpectateText) {
		drawSpectateText();
	}
}

function drawDebugKeys(): void {
	renderContext.font = "14px sans-serif";
	renderContext.fillStyle = "#fff";
	renderContext.textAlign = "left";
	printText("Click on canvas to capture input");
	printText("TAB toggles between free-camera and player camera");
	printText("W,A,S,D to move around");
	printText("R to reset car");
	printText("Click to fire a projectile");
	printText("I to pause/unpause game");
	printText("Right-click to move up (in free-camera mode)");
	printText("CTRL to move down (in free-camera mode)");
}

function drawDebugPlayerList(): void {
	renderContext.font = "15px sans-serif";
	renderContext.fillStyle = "#fff";
	renderContext.textAlign = "left";
	for (let player of GlobalState.playerList.getPlayers()) {
		printText(`${player.name} (${player.state})`);
	}
}

function printText(text: string): void {
	renderContext.fillText(text, 10, nextTextY);
	nextTextY += 15;
}

function drawTextShadow(text: string, x: number, y: number, color = "#fff", shadowColor = "#000"): void {
	const oldFill = renderContext.fillStyle;
	renderContext.fillStyle = shadowColor;
	renderContext.fillText(text, x + 1, y + 1);
	renderContext.fillStyle = color;
	renderContext.fillText(text, x, y);
	renderContext.fillStyle = oldFill;
}

function drawSpectateText(): void {
	renderContext.font = "32px sans-serif";
	renderContext.textAlign = "center";
	drawTextShadow("Ești spectator, apasă SPACE ca să intri in joc", renderContext.canvas.width * 0.5, 100);
}
