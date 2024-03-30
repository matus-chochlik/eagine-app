#version 140

in vec2 vertCoord;
out vec4 fragColor;
uniform int faceIdx;
uniform float planetRadius;
uniform float atmThickness;
uniform float vaporThickness;
uniform float cloudAltitude;
uniform float cloudThickness;
uniform float cloudiness;
uniform float aboveGround;
uniform vec3 sunDirection;
uniform float sunDot = 0.003;
uniform float tilingSide = 512;
uniform sampler2D tilingTex;

vec3 clearDirection = normalize(vec3(0.1, 0.2, 1.0));
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

float inf01(float x, float f) {
	return 1.0 - pow(exp(-f*x), 0.5 / f);
}

float fsteps(float x, int steps) {
	return float(int(x*float(steps)))/float(steps);
}

float fib(int n) {
	vec2 f = vec2(0.0, 1.0);
	for(int i=0; i<n; ++i) {
		f = vec2(f.y, f.x+f.y);
	}
	return f.y;
}

vec2 fibofs(int nx, int ny) {
	return vec2(fib(nx), fib(ny));
}
//------------------------------------------------------------------------------
struct Ray {
	vec3 origin;
	vec3 direction;
};

struct Segment {
	vec3 near;
	vec3 far;
	bool is_valid;
};

struct Sphere {
	vec3 center;
	float radius;
};

#define SphereHit vec2
//------------------------------------------------------------------------------
SphereHit sphereHit(Ray ray, Sphere sphere) {
    float rad2 = pow(sphere.radius, 2.0);
    vec3 dir = sphere.center - ray.origin;
    float tca = dot(dir, ray.direction);

    float d2 = dot(dir, dir) - pow(tca, 2.0);
	float radd2 = rad2 - d2;
	float sradd2 = sign(radd2);
    float thc = sqrt(sradd2*radd2) * max(sradd2, 0.0);
	vec2 t = vec2(tca) - vec2(thc,-thc);
	vec2 p = max(sign(t), vec2(0.0));

	return vec2(
		mix(tca, mix(t.y, t.x, p.x), p.y),
		mix(tca, t.y, p.y));
}
//------------------------------------------------------------------------------
float hitNear(SphereHit hit) {
	return hit.x;
}
//------------------------------------------------------------------------------
float hitFar(SphereHit hit) {
	return hit.y;
}
//------------------------------------------------------------------------------
bool isValidHit(SphereHit hit) {
	return hitNear(hit) >= 0.0;
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
vec2 cloudCoordAlt(
	vec3 sample,
	Sphere planet,
	float altMin,
	float altMax) {
	float altitude = distance(sample, planet.center) - planet.radius;
	altitude = (altitude - altMin) / (altMax - altMin);
	vec2 result = vec2(2.0 * clamp(altitude, 0.0, 1.0) - 1.0);

	return vec2(result.x, 1.0 - abs(result.x));
}
//------------------------------------------------------------------------------
vec4 cloudCoord(
	vec3 sample,
	Sphere planet,
	vec2 offset,
	float scale,
	vec2 coordAlt) {
	sample = sample - planet.center;
	scale = 10.0 / scale;
	sample = normalize(sample);
	offset = (scale * offset / tilingSide);
	return vec4(
		offset.x+scale * (atan(sample.y, sample.x)+3.14157),
		offset.y+scale * (asin(sample.z)),
		coordAlt.x,
		coordAlt.y);
}
//------------------------------------------------------------------------------
vec4 cloudCoord(
	vec3 sample,
	Sphere planet,
	vec2 offset,
	float scale,
	float altMin,
	float altMax) {
	return cloudCoord(
		sample,
		planet,
		offset,
		scale,
		cloudCoordAlt(sample, planet, altMin, altMax));
}
//------------------------------------------------------------------------------
float tilingSample(vec2 coord) {
	coord = coord * (512.0 / tilingSide);
	return texture(tilingTex, coord).r;
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
float thinCloudSample(Ray sample, Sphere planet, vec2 offset, float scale) {
	vec4 coord = cloudCoord(
		sample.origin,
		planet,
		offset,
		scale,
		-atmThickness*vaporThickness,
		+atmThickness*vaporThickness);

	float voffset = fsteps(coord.z, 37);
	return fsteps(tilingSample(coord.xy+offset*voffset), 3)*
		pow(coord.w, 1.21);
}
//------------------------------------------------------------------------------
float thinCloudDensity(Ray sample, Sphere planet) {
	const float om = 0.618;
	return pow(
		thinCloudSample(sample, planet, om*fibofs( 7, 15), 0.3)*
		thinCloudSample(sample, planet, om*fibofs(13, 11), 1.1)*
		thinCloudSample(sample, planet, om*fibofs( 6,  8), 3.1)*
		thinCloudSample(sample, planet, om*fibofs( 5,  6), 6.3)*
		thinCloudSample(sample, planet, om*fibofs( 4,  3), 16.3)*
		mix(
			thinCloudSample(sample, planet, om*fibofs( 2,  1), 31.7),
			thinCloudSample(sample, planet, om*fibofs( 0,  1), 63.1),
			0.4),
		1.0 / 3.0);
}
//------------------------------------------------------------------------------
float sunHit(Ray ray) {
	float d = dot(ray.direction, sunDirection);
	return max(sign(d + sunDot - 1.0), 0.0);
}
//------------------------------------------------------------------------------
float thinCloudDensity(
	Ray viewRay,
	Sphere planet,
	Sphere atmosphere,
	float atmHit) {
	float vapor = 0.0;
	const int sampleCount = 1000;
	const float isc = 1.0 / float(sampleCount);
	float sampleLen = vaporThickness * atmHit * isc ;
	for(int s=1; s<=sampleCount; ++s) {
		vapor += sampleLen * thinCloudDensity(
			raySample(viewRay, s*sampleLen),
			planet);
	}
	return vapor * isc;
}
//------------------------------------------------------------------------------
Segment cloudsIntersection(Ray ray, Sphere planet) {
	SphereHit topHit = sphereHit(ray, Sphere(
		planet.center,
		planet.radius+cloudAltitude+cloudThickness*0.5));
	SphereHit bottomHit = sphereHit(ray, Sphere(
		planet.center,
		planet.radius+cloudAltitude-cloudThickness*0.5));

	if(isValidHit(topHit) && isValidHit(bottomHit)) {
		vec2 hMinMax = vec2(
			min(hitNear(topHit), hitNear(bottomHit)),
			max(hitFar(topHit), hitFar(bottomHit)));

		return Segment(
			raySample(ray, hMinMax.x).origin,
			raySample(ray, hMinMax.y).origin,
			true);
	} else if(isValidHit(topHit)) {
		return Segment(
			raySample(ray, hitNear(topHit)).origin,
			raySample(ray, hitFar(topHit)).origin,
			true);
	} else if(isValidHit(bottomHit)) {
		return Segment(
			raySample(ray, hitNear(bottomHit)).origin,
			raySample(ray, hitFar(bottomHit)).origin,
			true);
	} else {
		return Segment(ray.origin, ray.origin, false);
	}
}
//------------------------------------------------------------------------------
float thickCloudSampleB(
	vec3 sample,
	Sphere planet,
	vec2 offset,
	float scale,
	vec2 coordAlt) {

	return tilingSample(cloudCoord(
		sample,
		planet,
		offset,
		scale*0.5,
		coordAlt).xy);
}
//------------------------------------------------------------------------------
float thickCloudSampleN(
	vec3 sample,
	Sphere planet,
	vec2 offset,
	float scale,
	vec2 coordAlt) {

	return tilingSample(cloudCoord(
		sample,
		planet,
		offset*(1.0 + fsteps(coordAlt.x, 31)/scale),
		scale,
		coordAlt).xy);
}
//------------------------------------------------------------------------------
float thickCloudDensity(vec3 sample, Sphere planet) {
	vec2 ca = cloudCoordAlt(
		sample,
		planet,
		cloudAltitude-cloudThickness*0.5,
		cloudAltitude+cloudThickness*0.5);

	float sc = 8.0;

	float s320000 = thickCloudSampleB(sample, planet, fibofs( 0, 0), 32.000, ca);
	float s160000 = thickCloudSampleB(sample, planet, fibofs( 2, 3), 16.000, ca);
	float s080000 = thickCloudSampleB(sample, planet, fibofs( 4, 5),  8.000, ca);
	float s040000 = thickCloudSampleB(sample, planet, fibofs( 5, 6),  4.000, ca);
	float s020000 = thickCloudSampleB(sample, planet, fibofs( 6, 7),  2.000, ca);
	float s010000 = thickCloudSampleB(sample, planet, fibofs( 7, 8),  1.000, ca);
	float s005000 = thickCloudSampleB(sample, planet, fibofs( 8, 9), 0.5000, ca);
	float s002500 = thickCloudSampleB(sample, planet, fibofs( 9,10), 0.2500, ca);
	float s001250 = thickCloudSampleB(sample, planet, fibofs(10,11), 0.1250, ca);
	float s000625 = thickCloudSampleB(sample, planet, fibofs(11,12), 0.0625, ca);
	float snoise  = thickCloudSampleN(sample, planet, fibofs(12,13), 0.0314, ca);

	float v = 1.0;
	v = v * max(1.05*pow(s320000, 2.00)-0.05, 0.0);
	v = v * max(1.15*pow(s160000, 2.00)-0.15, 0.0);
	v = v * max(1.20*pow(s080000, 2.00)-0.20, 0.0);
	v = v * max(1.25*pow(s040000, 2.00)-0.25, 0.0);
	v = v * max(1.15*pow(s020000, 3.00)-0.15, 0.0);
	v = v * max(1.10*pow(s020000, 3.00)-0.10, 0.0);
	v = v * max(1.10*pow(s010000, 3.00)-0.10, 0.0);

	v = sqrt(v);

	v *= mix(mix(s005000, s002500, 0.5), mix(s001250, s000625, 0.5), 0.5);

	return pow(ca.y, 1.0 / 8.0)*
		max(sign(2.0*pow(v, 1.0 / 2.0) - abs(ca.x)), 0.0)*
		mix(1.0, snoise, 0.5);
}
//------------------------------------------------------------------------------
vec4 cloudColor2(Ray ray, Sphere planet, vec4 sunColor) {
	Segment intersection = cloudsIntersection(ray, planet);
	float shadow = 1.0;
	if(intersection.is_valid) {
		const int sampleCount = 100;
		const float isc = 1.0 / float(sampleCount);
		vec3 direction = intersection.far - ray.origin;
		float l = length(direction);
		direction = direction / l;
		for(int s = 1; s <= sampleCount; ++s) {
			float z = float(s) * isc;
			float density = min(0.5 * l * thickCloudDensity(
				ray.origin + direction * l * z,
				planet), 1.0);
			shadow = shadow * mix(1.0, 0.99, density);
		}
	}
	return vec4(
		sunColor.rgb*shadow,
		thickCloudDensity(ray.origin, planet));
}
//------------------------------------------------------------------------------
vec4 cloudColor(Ray ray, Sphere planet, vec4 color) {
	const int sampleCount = 200;
	const float isc = 1.0 / float(sampleCount);
	Segment intersection = cloudsIntersection(ray, planet);
	float l = distance(intersection.near, intersection.far);
	float f = 2.7 * l * isc / cloudThickness;

	for(int s = 0; s <= sampleCount; ++s) {
		Ray sampleRay = Ray(
			mix(intersection.far, intersection.near, float(s)*isc),
			sunDirection);
		vec4 sample = cloudColor2(sampleRay, planet, vec4(1.0));
		color = vec4(
			mix(color.rgb, sample.rgb, sample.a*f),
			color.a);
	}
	return color;
}
//------------------------------------------------------------------------------
vec4 clearAirColor(Ray viewRay, Sphere planet, Sphere atmosphere) {

	float directLight = dot01(viewRay.direction, sunDirection);

	float atmHit = max(hitNear(sphereHit(viewRay, atmosphere)), 0.0);
	float vapor = min(thinCloudDensity(viewRay, planet, atmosphere, atmHit), 1.0);
	float atmDepth1 = 0.5 * sqrt(atmHit / atmThickness);
	float atmDepth2 = max(atmDepth1 - 1.0, 0.0);
	float sunAtHorizon = 1.0 - abs(sunDirection.y);

	float directScatter = pow(
		directLight,
		mix(64.0, 0.5, min(vapor * atmDepth2, 1.0)));

	float atmShadowHit = max(hitNear(sphereHit(
		raySample(
			viewRay,
			atmHit * mix(0.99, 1.0, directLight),
			sunDirection),
		atmosphere)), 0.0);
	float atmShadowDepth1 = sqrt(atmShadowHit / atmThickness);
	float atmShadowDepth2 = max(
		atmShadowDepth1 +
		sunAtHorizon*atmDepth1*2.0 -
		1.0,
		0.0);
	float atmScatter1 = inf01(atmShadowDepth1, 4.0);
	float atmScatter2 = inf01(atmShadowDepth2, 4.0);

	float planetShadow = max(hitNear(sphereHit(
		raySample(
			viewRay,
			atmHit * 1.1,
			sunDirection),
		planet)), 0.0);
	planetShadow = sign(planetShadow);
	planetShadow = min(pow(max(-sunDirection.y*2.0, 0.0), 2.0), 1.0);

	vec4 sun = mix(
		mix(vec4(1.1), vec4(0.90, 0.80, 0.55, 0.80), inf01(atmDepth2, 4.0)),
		mix(vec4(0.9), vec4(0.81, 0.79, 0.75, 0.80), inf01(atmDepth1, 4.0)),
		vapor);
	vec4 c1 = mix(
		mix(vec4(0.15, 0.25, 0.40, 0.5), vec4(0.7), vapor),
		mix(vec4(1.00, 0.95, 0.80, 1.0), vec4(0.9), vapor),
		directScatter);
	vec4 c2 = mix(
		mix(vec4(0.06, 0.10, 0.15, 1.0), vec4(0.50), vapor),
		mix(vec4(0.21, 0.13, 0.08, 1.0), vec4(0.75), vapor),
		directScatter);
	vec4 c3 = mix(
		mix(vec4(0.30, 0.15, 0.20, 0.2), vec4(0.40, 0.33, 0.30, 0.3), vapor),
		mix(vec4(0.80, 0.35, 0.40, 0.3), vec4(0.60, 0.45, 0.40, 0.4), vapor),
		directScatter);
	vec4 c4 = mix(vec4(0.01), vec4(0.11), vapor);
	return mix(
		mix(mix(mix(c1, c2, atmScatter1), c3, atmScatter2), c4, planetShadow),
		sun,
		sunHit(viewRay));
}
//------------------------------------------------------------------------------
vec3 planetCenter() {
	return vec3(0.0, -planetRadius-aboveGround, 0.0);
}
//------------------------------------------------------------------------------
vec4 skyColor(Ray viewRay, Sphere planet, Sphere atmosphere) {
	vec4 air = clearAirColor(viewRay, planet, atmosphere);
	return cloudColor(viewRay, planet, air);
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
	SphereHit planetHit = sphereHit(viewRay, planet);
	if(isValidHit(planetHit)) {
		fragColor = surfaceColor(viewRay, hitNear(planetHit), planet);
	} else {
		fragColor = skyColor(viewRay, planet, getAtmosphere());
	}
}

