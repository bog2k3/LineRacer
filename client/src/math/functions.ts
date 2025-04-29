import { assert } from "../utils/assert";

/** Returns the first power-of-two number greater than or equal to x */
export function nextPowerOfTwo(x: number): number {
	assert(x << 1 > x); // if x already uses the highest bit then we can't compute
	let r = 1;
	while (r < x) r = r << 1;
	return r;
}

export function clamp(x: number, a: number, b: number): number {
	if (x < a) return a;
	if (x > b) return b;
	return x;
}

export function lerp(a: number, b: number, f: number): number {
	f = clamp(f, 0, 1);
	return a * (1 - f) + b * f;
}

export function sqr(x: number): number {
	return x * x;
}
