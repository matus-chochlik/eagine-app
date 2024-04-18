/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
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
uniform vec2 cloudOffset;
uniform vec3 sunDirection;
uniform float sunApparentAngle;
uniform float tilingSide = 512;
uniform sampler2D tilingTex;

//------------------------------------------------------------------------------
float to01(float x) {
	return 0.5 + 0.5 * x;
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

vec2 fib2(int nx, int ny) {
	return vec2(fib(nx), fib(ny));
}
//------------------------------------------------------------------------------
vec4 mixColor101(vec4 c_1, vec4 c0, vec4 c1, float f) {
	return mix(mix(c0, c_1, -f), mix(c0, c1, f), to01(f));
}
//------------------------------------------------------------------------------
vec4 mixColor01n(vec4 c0, vec4 c1, vec4 cn, float f, float s) {
	return mix(cn, mix(c0, c1, sqrt(clamp(f, 0.0, 1.0))), min(exp(-(f-1)*s), 1.0));
}
//------------------------------------------------------------------------------
vec4 mixColor012n(vec4 c0, vec4 c1, vec4 c2, vec4 cn, float f, float s) {
	return mix(c0, mixColor01n(c1, c2, cn, max(f - 1.0, 0.0), s), min(f, 1.0));
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
	vec3 v = ray.direction;
	vec3 d = ray.origin - sphere.center;
	float e = dot(v, d);
	float r2 = pow(sphere.radius, 2.0);

	const vec2 z = vec2(0.0);

	vec2 b = vec2(pow(e, 2.0) - dot(d, d) + r2);
	vec2 m = max(sign(-b), z);

	b = sqrt(max(b, 0.0)) * vec2(-1.0,1.0);

	vec2 t = mix(b - vec2(e), vec2(-1.0), m);
	m = max(sign(-t), z);

	return vec2(mix(t.x, t.y, m.x), t.y);
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
vec2 cloudCoordAlt(vec3 location, Sphere planet, float altMin, float altMax) {
	float altitude = distance(location, planet.center) - planet.radius;
	altitude = (altitude - altMin) / (altMax - altMin);
	vec2 result = vec2(2.0 * clamp(altitude, 0.0, 1.0) - 1.0);

	return vec2(result.x, 1.0 - abs(result.x));
}
//------------------------------------------------------------------------------
vec2 cloudCoord(
	vec3 location,
	Sphere planet,
	vec2 offset,
	float scale) {
	location = location - planet.center;
	location = normalize(location);
	vec2 sph = vec2(atan(location.y, location.x)+3.14157, asin(location.z));
	vec2 sca = vec2(10.0 / scale);
	offset = (sca * offset / tilingSide);
	return cloudOffset / scale + offset + sca * sph;
}
//------------------------------------------------------------------------------
float tilingSample(vec2 coord) {
	coord = coord * (512.0 / tilingSide);
	return texture(tilingTex, coord).r;
}
//------------------------------------------------------------------------------
float tilingSample(vec3 location, Sphere planet, vec2 offset, float scale) {
	return tilingSample(cloudCoord(location, planet, offset, scale));
}
//------------------------------------------------------------------------------
float thinCloudSample(vec3 location, Sphere planet, vec2 offset, float scale) {
	vec2 coordAlt = cloudCoordAlt(
		location,
		planet,
		-atmThickness*vaporThickness,
		+atmThickness*vaporThickness);

	vec2 coord = cloudCoord(location, planet, offset, scale);

	return fsteps(tilingSample(coord+offset*fsteps(coordAlt.x, 37)), 3) *
		pow(coordAlt.y, 3.0);
}
//------------------------------------------------------------------------------
float thinCloudDensity(vec3 location, Sphere planet) {
	const float om = 0.618;
	return pow(
		thinCloudSample(location, planet, om*fib2( 5,  6), 6.31)*
		thinCloudSample(location, planet, om*fib2( 7,  8), 3.11)*
		thinCloudSample(location, planet, om*fib2( 9, 10), 1.11)*
		thinCloudSample(location, planet, om*fib2(11, 12), 0.31)*
		thinCloudSample(location, planet, om*fib2(12, 13), 0.13)*
		thinCloudSample(location, planet, om*fib2(12, 13), 0.07),
		1.0 / 3.0);
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
float thickCloudSampleB(vec3 location, Sphere planet, vec2 offset, float scale) {
	return tilingSample(location, planet, offset, scale*0.5);
}
//------------------------------------------------------------------------------
float thickCloudSampleN(
	vec3 location,
	Sphere planet,
	vec2 offset,
	float scale,
	vec2 coordAlt) {

	return tilingSample(cloudCoord(
		location,
		planet,
		offset*(1.0 + offset*fsteps(coordAlt.x, 31)/scale),
		scale));
}
//------------------------------------------------------------------------------
float thickCloudDensity(vec3 location, Sphere planet) {
	vec2 ca = cloudCoordAlt(
		location,
		planet,
		cloudAltitude-cloudThickness*0.5,
		cloudAltitude+cloudThickness*0.5);

	float s640000 = thickCloudSampleB(location, planet, fib2( 1, 2), 64.000);
	float s320000 = thickCloudSampleB(location, planet, fib2( 2, 3), 32.000);
	float s160000 = thickCloudSampleB(location, planet, fib2( 3, 4), 16.000);
	float s080000 = thickCloudSampleB(location, planet, fib2( 4, 5),  8.000);
	float s040000 = thickCloudSampleB(location, planet, fib2( 5, 6),  4.000);
	float s020000 = thickCloudSampleB(location, planet, fib2( 6, 7),  2.000);
	float s010000 = thickCloudSampleB(location, planet, fib2( 7, 8),  1.000);
	float s005000 = thickCloudSampleB(location, planet, fib2( 8, 9), 0.5000);
	float s002500 = thickCloudSampleB(location, planet, fib2( 9,10), 0.2500);
	float s001250 = thickCloudSampleB(location, planet, fib2(10,11), 0.1250);
	float s000625 = thickCloudSampleB(location, planet, fib2(11,12), 0.0625);
	float snoise  = thickCloudSampleN(location, planet, fib2(12,13), 0.0314, ca);

	float v = 1.0;
	v = max(v - max(sqrt(s640000)-mix(0.17, 0.97, cloudiness), 0.0), 0.0);
	v = max(v - max(sqrt(s320000)-mix(0.27, 0.87, cloudiness), 0.0), 0.0);
	v = max(v - max(sqrt(s160000)-mix(0.31, 0.81, cloudiness), 0.0), 0.0);
	v = max(v - max(sqrt(s080000)-mix(0.37, 0.79, cloudiness), 0.0), 0.0);
	v = max(v - max(sqrt(s040000)-mix(0.41, 0.77, cloudiness), 0.0), 0.0);
	v = max(v - max(sqrt(s020000)-mix(0.47, 0.71, cloudiness), 0.0), 0.0);
	v = max(v - max(sqrt(s010000)-0.71, 0.0), 0.0);
	v = max(v - max(sqrt(s005000)-0.73, 0.0), 0.0);
	v = max(v - max(sqrt(s002500)-0.74, 0.0), 0.0);
	v = max(v - max(sqrt(s001250)-0.79, 0.0), 0.0);

	float w = mix(
		mix(s020000, s010000, 0.6),
		mix(mix(s005000, s002500, 0.5), mix(s001250, s000625, 0.5), 0.5),
		0.6);

	float b = mix(
		snoise,
		max(
			mix(pow(s040000, 2.0), pow(s020000, 2.0), 0.4),
			mix(pow(s010000, 2.0), pow(s005000, 2.0), 0.4)),
		0.8);

	return pow(snoise, 1.4) * sqrt(ca.y) * max(sign(v*w - abs(ca.x-b)), 0.0);
}
//------------------------------------------------------------------------------
struct AtmosphereSample {
	Ray viewRay;
	Ray lightRay;
	Segment cloudsIntersection;
	float hitLight;
	float directLight;
	float atmDensity;
	float atmDistance;
	float atmDistRatio;
	float vaporDensity;
	float planetShadow;
};
//------------------------------------------------------------------------------
AtmosphereSample atmSample(
	vec3 lightDirection,
	float lightAngle,
	Ray viewRay,
	float rayDist,
	Sphere planet,
	Sphere atmosphere) {
	Ray lightRay = raySample(viewRay, rayDist, lightDirection);
	float directLight = dot(viewRay.direction, lightRay.direction);
	float atmDistance = max(hitNear(sphereHit(lightRay, atmosphere)), 0.0);
	float sunDown = pow(max(-lightDirection.y, 0.0), 0.25);
	return AtmosphereSample(
		viewRay,
		lightRay,
		cloudsIntersection(lightRay, planet),
		max(sign(lightAngle-acos(directLight)), 0.0),
		directLight,
		pow(clamp(
			distance(lightRay.origin, planet.center) - planet.radius /
			atmosphere.radius - planet.radius,
			0.0, 1.0), 1.21),
		atmDistance,
		atmDistance / atmThickness,
		thinCloudDensity(
			lightRay.origin,
			planet),
		mix(
			sign(max(hitNear(sphereHit(lightRay, planet)), 0.0)) * sunDown,
			1.0,
			sunDown));
}
//------------------------------------------------------------------------------
vec2 cloudSample(AtmosphereSample a, Sphere planet) {
	float shadow = 1.0;
	if(a.cloudsIntersection.is_valid) {
		const int sampleCount = 150;
		const float isc = 1.0 / float(sampleCount);
		vec3 direction = a.cloudsIntersection.far - a.lightRay.origin;
		float l = length(direction);
		direction = direction / l;
		l = min(l, 128.0 * cloudThickness * isc);
		for(int s = 1; s <= sampleCount; ++s) {
			vec3 location = a.lightRay.origin + direction * l * float(s) * isc;
			float density = min(
				2.5 * cloudiness * l * thickCloudDensity(location, planet),
				1.0);
			shadow = shadow * mix(1.0, 0.995, density);
		}
	}
	return vec2(
		shadow,
		min(thickCloudDensity(a.lightRay.origin, planet), 1.0));
}
//------------------------------------------------------------------------------
float cloudShadow(AtmosphereSample a, Sphere planet) {
	float shadow = 1.0;
	if(a.cloudsIntersection.is_valid) {
		const int sampleCount = 50;
		const float isc = 1.0 / float(sampleCount);
		float l = distance(a.cloudsIntersection.far, a.cloudsIntersection.near);
		float density = 0.0;
		for(int s = 0; s <= sampleCount; ++s) {
			vec3 location = mix(
				a.cloudsIntersection.far,
				a.cloudsIntersection.near,
				float(s) * isc);
			density += min(l * thickCloudDensity(location, planet), 1.0);
		}
		shadow = 1.0 - density * isc;
	}
	return shadow;
}
//------------------------------------------------------------------------------
vec4 sunlightColor(AtmosphereSample a) {
	return mix(
		mixColor012n(
			vec4(1.5), vec4(1.4),
			vec4(1.3, 1.1, 0.8, 1.0),
			vec4(1.2, 0.6, 0.4, 0.9),
			a.atmDistRatio, 0.5),
		vec4(0.0),
		a.planetShadow);
}
//------------------------------------------------------------------------------
vec4 clearAirColor(AtmosphereSample a, float sampleLen, float shadow) {
	float directLight = pow(abs(a.directLight), 16.0);
	float sunDown = sqrt(to01(a.viewRay.direction.y));

	float vapor = min(
		directLight +
		a.vaporDensity *
		a.atmDensity *
		sampleLen,
		1.0);

	vec4 airColor = mix(
		mixColor012n(
			vec4(0.30, 0.45, 0.75, 1.0),
			vec4(0.25, 0.40, 0.70, 1.0),
			vec4(0.50, 0.40, 0.25, 0.9),
			vec4(0.40, 0.10, 0.25, 0.8),
			a.atmDistRatio, 1.0),
		mixColor012n(
			vec4(0.30, 0.45, 0.75, 1.0),
			vec4(0.25, 0.40, 0.70, 1.0),
			vec4(0.25, 0.40, 0.70, 0.9),
			vec4(0.50, 0.40, 0.35, 0.8),
			a.atmDistRatio * 0.7, 0.5),
		sunDown);

	airColor = vec4(airColor.rgb, mix(airColor.a, 1.00, a.hitLight));

	vec4 vaporColor = mix(
		mixColor012n(
			mix(
				vec4(0.95, 0.95, 0.96, 0.4),
				vec4(1.05, 1.05, 1.06, 0.4),
				directLight),
			mix(
				vec4(0.94, 0.94, 1.94, 0.4),
				vec4(1.02, 1.02, 1.02, 0.4),
				directLight),
			vec4(0.90, 0.90, 0.90, 0.3),
			vec4(0.55, 0.55, 0.54, 0.2),
			a.atmDistRatio, 0.5),
		mixColor012n(
			mix(
				vec4(0.95, 0.95, 0.96, 0.4),
				vec4(1.05, 1.05, 1.04, 0.3),
				directLight),
			vec4(0.95, 0.95, 0.96, 0.3),
			mix(
				vec4(0.85, 0.85, 0.85, 0.2),
				vec4(0.83, 0.83, 0.85, 0.2),
				shadow),
			mix(
				vec4(0.58, 0.43, 0.41, 0.1),
				vec4(0.42, 0.45, 0.49, 0.1),
				shadow),
			a.atmDistRatio, 0.5),
		sunDown);

	vaporColor = vec4(vaporColor.rgb, mix(vaporColor.a, 1.00, a.hitLight));

	return mix(
		mix(airColor, vaporColor, min(vapor, 1.0)),
		mix(
			vec4(0.0, 0.0, 0.01, 1.0),
			vec4(0.0, 0.0, 0.05, 0.3),
			min(vapor, 1.0)),
		a.planetShadow);
}
//------------------------------------------------------------------------------
vec4 skyColor(Ray viewRay, Sphere planet, Sphere atmosphere) {
	float cloudHitNear = hitNear(sphereHit(viewRay, Sphere(
		planet.center,
		planet.radius+cloudAltitude-cloudThickness*0.5)));
	float cloudHitFar = hitFar(sphereHit(viewRay, Sphere(
		planet.center,
		planet.radius+cloudAltitude+cloudThickness*0.5)));

	float atmosphereHit = hitNear(sphereHit(viewRay, atmosphere));

	vec4 color = vec4(0.0);

	const float airDensityMult = 5.0;
	float sampleLen = 50.0;
	float sampleRatio = sampleLen / atmThickness;

	for(float dist = atmosphereHit; dist > cloudHitFar; dist -= sampleLen) {
		AtmosphereSample a = atmSample(
			sunDirection,
			sunApparentAngle,
			viewRay,
			dist,
			planet,
			atmosphere);
		vec4 sunLight = sunlightColor(a);
		vec4 airLight = clearAirColor(a, sampleLen, 0.0);
		color = mix(
			mix(color, airLight, airDensityMult * sampleRatio),
			sunLight,
			a.hitLight);
	}

	int cloudSamples = 200;
	float isc = 1.0 / float(cloudSamples);
	sampleLen = (cloudHitFar - cloudHitNear) * isc;
	sampleRatio = sampleLen / atmThickness;

	for(int s = 0; s <= cloudSamples; ++s) {
		float dist = mix(cloudHitFar, cloudHitNear, float(s) * isc);
		AtmosphereSample a = atmSample(
			sunDirection,
			sunApparentAngle,
			viewRay,
			dist,
			planet,
			atmosphere);

		vec2 cloud = cloudSample(a, planet);
		vec4 sunLight = sunlightColor(a);
		vec4 airLight = clearAirColor(a, sampleLen, cloud.y);
		color = mix(
			mix(
				mix(color, airLight, airDensityMult * sampleRatio),
				sunLight,
				a.hitLight * airLight.a * cloud.y),
			sunLight * cloud.x,
			cloud.y);
	}

	sampleLen = 25.0;
	sampleRatio = sampleLen / atmThickness;

	for(float dist = cloudHitNear; dist > sampleLen; dist -= sampleLen) {
		AtmosphereSample a = atmSample(
			sunDirection,
			sunApparentAngle,
			viewRay,
			dist,
			planet,
			atmosphere);
		float shadow = cloudShadow(a, planet);
		vec4 airLight = clearAirColor(a, sampleLen, shadow);
		color = mix(color, airLight, airDensityMult * sampleRatio);
	}

	return color;
}
//------------------------------------------------------------------------------
vec4 surfaceColor(Ray viewRay, float rayDist, Sphere planet) {
	float f = exp(-rayDist / 500.0);
	return mix(vec4(0.23, 0.20, 0.17, 0.0), vec4(0.4, 0.3, 0.2, 0.0), f);
}
//------------------------------------------------------------------------------
vec3 planetCenter() {
	return vec3(0.0, -planetRadius-aboveGround, 0.0);
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

