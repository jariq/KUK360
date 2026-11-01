// Maps each screen pixel to a point on the sphere and samples the
// equirectangular image there.
// Port of fragEquiCubeBase + fragEquirectangular from libpannellum
// (c) 2011-2026 Matthew Petroff, MIT license - see THIRD_PARTY_NOTICES.txt
// The alignment grid (u_grid) is a KUK360 addition.

#ifdef GL_ES
precision highp float;
#endif

uniform float u_aspectRatio;
uniform float u_psi;    // yaw
uniform float u_theta;  // pitch
uniform float u_f;      // focal length
uniform float u_h;      // horizontal angle of view / 2PI
uniform float u_v;      // vertical angle of view / PI
uniform float u_vo;     // vertical offset / (PI / 2)
uniform float u_rot;    // roll

const float PI = 3.14159265358979323846264;

uniform sampler2D u_image0;
uniform sampler2D u_image1;
uniform bool u_splitImage;

uniform vec4 u_backgroundColor;

uniform bool u_grid;        // draw the latitude / longitude alignment grid
uniform float u_pixelSize;  // height of one device pixel in v_texCoord units (2 / height)

varying vec2 v_texCoord;

// Grid lines every 10 degrees in alternating colors (as in FlatView); the equator
// (horizon) is red, meridians end at +-80 degrees where they would merge.
const float GRID_STEP = PI / 18.0;
const float GRID_MERIDIAN_LIMIT = PI * 80.0 / 180.0 + 0.001;
const vec4 GRID_COLOR0 = vec4(128.0 / 255.0, 1.0, 0.0, 1.0);
const vec4 GRID_COLOR1 = vec4(83.0 / 255.0, 237.0 / 255.0, 243.0 / 255.0, 1.0);
const vec4 GRID_EQUATOR_COLOR = vec4(1.0, 0.0, 0.0, 1.0);

// Longitude (lambda, -PI..PI) and latitude (phi) of the sphere point seen at canvas
// position p
vec2 sphereCoord(vec2 p) {
    // Map canvas/camera to sphere
    float x = p.x * u_aspectRatio;
    float y = p.y;
    float sinrot = sin(u_rot);
    float cosrot = cos(u_rot);
    float rot_x = x * cosrot - y * sinrot;
    float rot_y = x * sinrot + y * cosrot;
    float sintheta = sin(u_theta);
    float costheta = cos(u_theta);
    float a = u_f * costheta - rot_y * sintheta;
    float root = sqrt(rot_x * rot_x + a * a);
    float lambda = atan(rot_x / root, a / root) + u_psi;
    float phi = atan((rot_y * costheta + u_f * sintheta) / root);

    // Wrap image
    lambda = mod(lambda + PI, PI * 2.0) - PI;
    return vec2(lambda, phi);
}

// Distance of value to the nearest grid line, in pixels; dx / dy are the changes of
// value per pixel. index receives the line number.
// The distance is signed and shifted by a quarter pixel: a line lying exactly between
// two pixel centers (e.g. the meridian in the middle of an even-width view at yaw 0) has
// both centers at 0.5 px and would otherwise be drawn in neither or both.
float gridDistance(float value, float dx, float dy, out float index) {
    float g = value / GRID_STEP;
    index = floor(g + 0.5);
    return abs((g - index) * GRID_STEP / max(length(vec2(dx, dy)), 1e-6) + 0.25);
}

vec4 gridColor(float index) {
    return mod(index, 2.0) < 0.5 ? GRID_COLOR0 : GRID_COLOR1;
}

void main() {
    vec2 sphere = sphereCoord(v_texCoord);
    float lambda = sphere.x;
    float phi = sphere.y;

    // Map texture to sphere
    vec2 coord = vec2(lambda / PI, phi / (PI / 2.0));

    // Look up color from texture
    // Map from [-1,1] to [0,1] and flip y-axis
    if (coord.x < -u_h || coord.x > u_h || coord.y < -u_v + u_vo || coord.y > u_v + u_vo) {
        gl_FragColor = u_backgroundColor;
    } else if (u_splitImage) {
        // Image was split into two textures to work around texture size limits
        if (coord.x < 0.0)
            gl_FragColor = texture2D(u_image0, vec2((coord.x + u_h) / u_h, (-coord.y + u_v + u_vo) / (u_v * 2.0)));
        else
            gl_FragColor = texture2D(u_image1, vec2((coord.x + u_h) / u_h - 1.0, (-coord.y + u_v + u_vo) / (u_v * 2.0)));
    } else {
        gl_FragColor = texture2D(u_image0, vec2((coord.x + u_h) / (u_h * 2.0), (-coord.y + u_v + u_vo) / (u_v * 2.0)));
    }

    if (u_grid) {
        // Per-pixel change of lambda / phi from the neighboring pixels, so that lines
        // stay 1 px wide at any zoom and direction
        vec2 dx = sphereCoord(v_texCoord + vec2(u_pixelSize / u_aspectRatio, 0.0)) - sphere;
        vec2 dy = sphereCoord(v_texCoord + vec2(0.0, u_pixelSize)) - sphere;
        dx.x = mod(dx.x + PI, PI * 2.0) - PI;  // across the +-180 degree seam
        dy.x = mod(dy.x + PI, PI * 2.0) - PI;

        float parallel;
        float meridian;
        float parallelDistance = gridDistance(phi, dx.y, dy.y, parallel);
        float meridianDistance = gridDistance(lambda, dx.x, dy.x, meridian);
        if (parallelDistance < 0.5)
            gl_FragColor = parallel == 0.0 ? GRID_EQUATOR_COLOR : gridColor(parallel);
        else if (meridianDistance < 0.5 && abs(phi) < GRID_MERIDIAN_LIMIT)
            gl_FragColor = gridColor(meridian);
    }
}
