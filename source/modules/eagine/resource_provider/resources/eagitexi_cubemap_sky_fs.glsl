#version 140

in vec2 vertCoord;
out vec4 fragColor;
uniform int faceIdx;
uniform float planetRadius;
uniform float atmThickness;
uniform float aboveGround;
uniform vec3 sunDirection;
uniform float sunDot = 0.003;

struct Ray {
	vec3 origin;
	vec3 direction;
};

struct Sphere {
	vec3 center;
	float radius;
};

Ray raySample(Ray ray, float rayDist) {
	return Ray(ray.origin + ray.direction * rayDist, ray.direction);
}

bool isValidHit(float param) {
	return param >= 0.0;
}

float sphereHit(Ray ray, Sphere sphere) {
    float rad2 = pow(sphere.radius, 2.0);
    vec3 l = sphere.center - ray.origin;
    float tca = dot(l, normalize(ray.direction));

    float d2 = dot(l, l) - pow(tca, 2.0);
    if(d2 > rad2) {
		return -1.0;
	}
    float thc = sqrt(rad2 - d2);
    float t0 = tca - thc;
    float t1 = tca + thc;

    if (t0 > 0.0) {
        return t0 / length(ray.direction);
    } 

    if (t1 > 0.0) {
        return t1 / length(ray.direction);
    }

    return -1.0;
}

Ray getViewRay() {
	mat3 cubeFace = mat3[6](
		mat3( 0.0, 0.0,-1.0, 0.0,-1.0, 0.0, 1.0, 0.0, 0.0),
		mat3( 0.0, 0.0, 1.0, 0.0,-1.0, 0.0,-1.0, 0.0, 0.0),
		mat3( 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0),
		mat3( 1.0, 0.0, 0.0, 0.0, 0.0,-1.0, 0.0,-1.0, 0.0),
		mat3( 1.0, 0.0, 0.0, 0.0,-1.0, 0.0, 0.0, 0.0, 1.0),
		mat3(-1.0, 0.0, 0.0, 0.0,-1.0, 0.0, 0.0, 0.0,-1.0))[faceIdx];
	return Ray(vec3(0.0), normalize(
		cubeFace[0]*vertCoord.x + cubeFace[1]*vertCoord.y + cubeFace[2]));
}

float vaporDensity(vec3 sample, Sphere planet, Sphere atmosphere) {
	Sphere clouds[2] = Sphere[2](
		Sphere(vec3(2100.0, 3100.0, 200.0), 1300.0),
		Sphere(vec3(1500.0, 3000.0, 200.0), 1200.0));
	float result = 0.0;
	for(int c=0; c<2; ++c) {
		float dist = distance(clouds[c].center, sample);
		float density = clamp(1.0 - dist / clouds[c].radius, 0.0, 1.0) * 0.001;
		result = max(result, density);
	}
	return result;
}

float airDensity(vec3 sample, Sphere planet, Sphere atmosphere) {
	float ds = distance(sample, atmosphere.center);
	float fact = (ds - planet.radius) / (atmosphere.radius - planet.radius);
	return pow(clamp(1.0 - fact, 0.0, 1.0), 1.25) * 0.000001;
}

float sunHit(Ray ray) {
	float d = dot(ray.direction, sunDirection);
	return max(sign(d + sunDot - 1.0), 0.0);
}

vec4 sunColor() {
	return vec4(1.0);
}

vec4 bgColor(Ray ray, float rayDist) {
	return mix(vec4(0.0), sunColor(), sunHit(ray));
}

vec4 skyColor(Ray viewRay, Sphere planet, Sphere atmosphere) {
	float atmHit = max(sphereHit(viewRay, atmosphere), 0.0);
	float dt = 0.5 + 0.5*dot(viewRay.direction, sunDirection);
	vec4 result = mix(mix(
		vec4(0.25, 0.35, 0.50, 0.5),
		vec4(1.00, 0.90, 0.75, 1.0),
		pow(dt, 16.0)), sunColor(), sunHit(viewRay));
	return result;
}

vec4 surfaceColor(Ray viewRay, float rayDist, Sphere planet) {
	float f = exp(-rayDist/500.0);
	return mix(vec4(0.23, 0.20, 0.17, 0.0), vec4(0.4, 0.3, 0.2, 0.0), f);
}

Sphere getPlanet() {
	return Sphere(
		vec3(0.0, -planetRadius-aboveGround, 0.0),
		planetRadius);
}

Sphere getAtmosphere() {
	return Sphere(
		vec3(0.0, -planetRadius-aboveGround, 0.0),
		planetRadius+atmThickness);
}

void main() {
	Ray viewRay = getViewRay();
	Sphere planet = getPlanet();
	float planetHit = sphereHit(viewRay, planet);
	if(isValidHit(planetHit)) {
		fragColor = surfaceColor(viewRay, planetHit, planet);
	} else {
		fragColor = skyColor(viewRay, planet, getAtmosphere());
	}
}

