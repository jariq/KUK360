// Full-screen quad; the fragment shader casts a ray per pixel.
// Port of the equirectangular vertex shader from libpannellum
// (c) 2011-2026 Matthew Petroff, MIT license - see THIRD_PARTY_NOTICES.txt

attribute vec2 a_texCoord;
varying vec2 v_texCoord;

void main() {
    gl_Position = vec4(a_texCoord, 0.0, 1.0);
    v_texCoord = a_texCoord;
}
