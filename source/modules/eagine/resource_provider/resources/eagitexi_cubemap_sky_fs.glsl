#version 140

in vec2 vertCoord;
out vec4 fragColor;
uniform int faceIdx;
uniform float planetRadius;
uniform float atmThickness;
uniform float cloudAltitude;
uniform float cloudThickness;
uniform float cloudiness;
uniform float aboveGround;
uniform vec3 sunDirection;
uniform float sunDot = 0.003;
uniform float tilingSide = 512;
uniform isampler2D tilingTex;

vec3 clearDirection = normalize(vec3(0.1, 0.2, 1.0));
float atmRadius = planetRadius+atmThickness;
//------------------------------------------------------------------------------
struct Ray {
	vec3 origin;
	vec3 direction;
};
//------------------------------------------------------------------------------
struct Sphere {
	vec3 center;
	float radius;
};
//------------------------------------------------------------------------------
float to01(float x) {
	return 0.5 + 0.5 * x;
}

float sin01(float x) {
	return to01(sin(x));
}

float dot01(vec3 l, vec3 r) {
	return to01(dot(l, r));
}
//------------------------------------------------------------------------------
Ray raySample(Ray ray, float rayDist) {
	return Ray(ray.origin + ray.direction * rayDist, ray.direction);
}
//------------------------------------------------------------------------------
Ray raySample(Ray ray, float rayDist, vec3 direction) {
	return Ray(ray.origin + ray.direction * rayDist, normalize(direction));
}
//------------------------------------------------------------------------------
bool isValidHit(float param) {
	return param >= 0.0;
}
//------------------------------------------------------------------------------
float sphereHit(Ray ray, Sphere sphere) {
    float rad2 = pow(sphere.radius, 2.0);
    vec3 dir = sphere.center - ray.origin;
    float tca = dot(dir, ray.direction);

    float d2 = dot(dir, dir) - pow(tca, 2.0);
    if(rad2 < d2) {
		return -1.0;
	}
    float thc = sqrt(rad2 - d2);
    float t0 = tca - thc;
    float t1 = tca + thc;

    if (t0 >= 0.0) {
        return t0;
    } 

    if (t1 >= 0.0) {
        return t1;
    }

    return -1.0;
}
//------------------------------------------------------------------------------
Ray getViewRay() {
	mat3 cubeFace = mat3[6](
		mat3( 0.0, 0.0,-1.0, 0.0,-1.0, 0.0, 1.0, 0.0, 0.0),
		mat3( 0.0, 0.0, 1.0, 0.0,-1.0, 0.0,-1.0, 0.0, 0.0),
		mat3( 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0),
		mat3( 1.0, 0.0, 0.0, 0.0, 0.0,-1.0, 0.0,-1.0, 0.0),
		mat3( 1.0, 0.0, 0.0, 0.0,-1.0, 0.0, 0.0, 0.0, 1.0),
		mat3(-1.0, 0.0, 0.0, 0.0,-1.0, 0.0, 0.0, 0.0,-1.0))[faceIdx];
	return Ray(
		vec3(0.0),
		normalize(
			cubeFace[0]*vertCoord.x +
			cubeFace[1]*vertCoord.y +
			cubeFace[2]));
}
//------------------------------------------------------------------------------
vec3 cloudCoord(
	vec3 sample,
	Sphere planet,
	vec2 offset,
	float scale,
	float altMin,
	float altMax) {
	sample = sample - planet.center;
	float altitude = distance(planet.center, sample) - planet.radius;
	scale = 10.0 / scale;
	sample = normalize(sample);
	offset = (scale * offset / tilingSide);
	return vec3(
		offset.x+scale * (atan(sample.y, sample.x)+3.14157),
		offset.y+scale * (asin(sample.z)),
		2.0*(altitude - altMin)/(0.001 + altMax - altMin)-1.0);
}
//------------------------------------------------------------------------------
vec3 cloudCoord(vec3 sample, Sphere planet, vec2 offset, float scale) {
	return cloudCoord(
		sample,
		planet,
		offset,
		scale,
		cloudAltitude - cloudThickness * 0.5,
		cloudAltitude + cloudThickness * 0.5);
}
//------------------------------------------------------------------------------
float tilingSample(vec2 coord) {
	coord = coord * (512.0 / tilingSide);
	return float(texture(tilingTex, coord).r) / 15.0;
}
//------------------------------------------------------------------------------
float tilingSample(
	vec3 sample,
	Sphere planet,
	vec2 offset,
	float scale,
	float altMin,
	float altMax) {
	return tilingSample(cloudCoord(
		sample,
		planet,
		offset,
		scale,
		altMin,
		altMax).xy);
}
//------------------------------------------------------------------------------
float cloudSample(vec3 sample, Sphere planet, vec2 offset, float scale) {
	// TODO
	return tilingSample(cloudCoord(sample, planet, offset, scale).xy);
}
//------------------------------------------------------------------------------
float groundVaporSample(Ray sample, Sphere planet, vec2 offset, float scale) {
	vec3 coord = cloudCoord(
		sample.origin,
		planet,
		offset,
		scale,
		planetRadius-atmThickness*0.05,
		planetRadius+atmThickness*0.05);

	return tilingSample(coord.xy+offset*float(int(coord.z*20.0))*0.05)*
		max(1.0-sample.direction.y, 0.0)*
		exp(-coord.z * 2.0);
}
//------------------------------------------------------------------------------
float groundVaporDensity(Ray sample, Sphere planet, Sphere atmosphere) {
	return 0.00025*
		mix(
			groundVaporSample(sample, planet, vec2(11.1, 63.1), 0.33),
			groundVaporSample(sample, planet, vec2(23.7, 31.5), 0.19),
			0.5)*
		mix(
			groundVaporSample(sample, planet, vec2(93.1, 99.3), 0.07),
			groundVaporSample(sample, planet, vec2(41.7, 31.5), 0.03),
			0.5)*
		mix(
			groundVaporSample(sample, planet, vec2(29.1, 59.1), 0.02),
			groundVaporSample(sample, planet, vec2(71.3, 97.1), 0.01),
			0.5);
}
//------------------------------------------------------------------------------
float vaporDensity(Ray sample, Sphere planet, Sphere atmosphere) {
	return groundVaporDensity(sample, planet, atmosphere);
}
//------------------------------------------------------------------------------
float airDensity(vec3 sample, Sphere planet, Sphere atmosphere) {
	float ds = distance(sample, atmosphere.center);
	float fact = (ds - planet.radius) / (atmosphere.radius - planet.radius);
	return pow(clamp(1.0 - fact, 0.0, 1.0), 1.25) * 0.000001;
}
//------------------------------------------------------------------------------
float sunHit(Ray ray) {
	float d = dot(ray.direction, sunDirection);
	return max(sign(d + sunDot - 1.0), 0.0);
}

vec4 sunLight() {
	return vec4(1.0);
}
//------------------------------------------------------------------------------
float thinCloudDensity(
	Ray viewRay,
	Sphere planet,
	Sphere atmosphere,
	float atmHit) {
	float vapor = 0.0;
	int sampleCount = 1000;
	float sampleLen = atmHit / float(sampleCount);
	for(int s=1; s<=sampleCount; ++s) {
		vapor += sampleLen * vaporDensity(
			raySample(viewRay, s*sampleLen),
			planet,
			atmosphere);
	}
	return pow(vapor, 0.5);
}
//------------------------------------------------------------------------------
vec4 clearAirColor(Ray viewRay, Sphere planet, Sphere atmosphere) {

	float atmHit = max(sphereHit(viewRay, atmosphere), 0.0);
	float vapor = thinCloudDensity(viewRay, planet, atmosphere, atmHit);

	float directLight = dot01(viewRay.direction, sunDirection);
	float scatterLight = pow(directLight, mix(128.0, 0.5, min(vapor, 1.0)));

	float atmShadow = max(sphereHit(
		raySample(
			viewRay,
			atmHit * mix(0.99, 1.0, directLight),
			sunDirection),
		atmosphere), 0.0);
	atmShadow = min(sqrt(atmShadow / atmThickness), 1.0);
	atmShadow = 1.0 - atmShadow;

	float planetShadow = clamp(sphereHit(
		raySample(
			viewRay,
			atmHit * 1.1,
			sunDirection),
		planet), 0.0, 1.0);
	planetShadow = 1.0 - planetShadow;

	vec4 sun = mix(sunLight(), vec4(0.7), vapor);
	vec4 c1 = mix(
		mix(vec4(0.15, 0.25, 0.40, 0.5), vec4(0.7), vapor),
		mix(vec4(1.00, 0.95, 0.80, 1.0), vec4(0.7), vapor),
		scatterLight);
	return mix(c1, sun, sunHit(viewRay));
}
//------------------------------------------------------------------------------
vec3 planetCenter() {
	return vec3(0.0, -planetRadius-aboveGround, 0.0);
}
//------------------------------------------------------------------------------
Sphere getCloudsTop() {
	return Sphere(
		planetCenter(),
		planetRadius+cloudAltitude+cloudThickness*0.5);
}

Sphere getCloudsBottom() {
	return Sphere(
		planetCenter(),
		planetRadius+cloudAltitude-cloudThickness*0.5);
}
//------------------------------------------------------------------------------
vec4 skyColor(Ray viewRay, Sphere planet, Sphere atmosphere) {
	return clearAirColor(viewRay, planet, atmosphere);
}
//------------------------------------------------------------------------------
vec4 surfaceColor(Ray viewRay, float rayDist, Sphere planet) {
	float f = exp(-rayDist/500.0);
	return mix(vec4(0.23, 0.20, 0.17, 0.0), vec4(0.4, 0.3, 0.2, 0.0), f);
}
//------------------------------------------------------------------------------
Sphere getPlanet() {
	return Sphere(planetCenter(), planetRadius);
}
//------------------------------------------------------------------------------
Sphere getAtmosphere() {
	return Sphere(planetCenter(), planetRadius+atmThickness);
}
//------------------------------------------------------------------------------
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

