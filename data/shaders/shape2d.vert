attribute vec3 vPos;
attribute vec4 vColor;

varying vec4 fColor;

uniform mat4 mViewportInverse;

void main() {

    //gl_Position = mViewportInverse * vec4(vPos, 1);
	gl_Position = vec4(vPos.x/100.0, vPos.y/100.0, 0, 1);
    fColor = vColor;
}
