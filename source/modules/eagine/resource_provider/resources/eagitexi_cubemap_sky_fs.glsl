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
uniform float sunApparentAngle;
uniform float tilingSide = 512;
uniform sampler2D tilingTex;

vec3 clearDirection = normalize(vec3(0.1, 0.2, 1.0));
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
	scale = 10.0 / scale;
	location = normalize(location);
	offset = (scale * offset / tilingSide);
	return vec2(
		offset.x+scale * (atan(location.y, location.x)+3.14157),
		offset.y+scale * (asin(location.z)));
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
		1.0 / 6.0);
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
		offset*(1.0 + fsteps(coordAlt.x, 31)/scale),
		scale));
}
//------------------------------------------------------------------------------
float thickCloudDensity(vec3 location, Sphere planet) {
	vec2 ca = cloudCoordAlt(
		location,
		planet,
		cloudAltitude-cloudThickness*0.5,
		cloudAltitude+cloudThickness*0.5);

	float s640000 = thickCloudSampleB(location, planet, fib2( 0, 0), 64.000);
	float s160000 = thickCloudSampleB(location, planet, fib2( 2, 3), 16.000);
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
	v = v * max(2.00*sqrt(s640000)-1.00, 0.0);
	v = v * max(2.00*sqrt(s160000)-1.00, 0.0);
	v = v * max(2.00*sqrt(s080000)-1.00, 0.0);
	v = v * max(1.50*sqrt(s040000)-0.50, 0.0);
	v = v * max(1.50*sqrt(s020000)-0.50, 0.0);
	v = v * max(1.25*sqrt(s020000)-0.25, 0.0);
	v = v * max(1.25*sqrt(s010000)-0.25, 0.0);

	v = pow(v, 1.0 / 7.0);

	v *= mix(mix(s005000, s002500, 0.5), mix(s001250, s000625, 0.5), 0.5);

	return pow(ca.y, 1.0 / 4.0)*
		max(sign(sqrt(v) - abs(ca.x)), 0.0)*
		mix(1.0, snoise, 0.55);
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
	float sunDown = max(-lightDirection.y, 0.0);
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
			sign(max(hitNear(sphereHit(lightRay, planet)), 0.0))*sqrt(sunDown),
			1.0,
			sunDown));
}
//------------------------------------------------------------------------------
vec2 cloudSample(AtmosphereSample a, Sphere planet) {
	float occlusion = 1.0;
	if(a.cloudsIntersection.is_valid) {
		const int sampleCount = 100;
		const float isc = 1.0 / float(sampleCount);
		vec3 direction = a.cloudsIntersection.far - a.lightRay.origin;
		float l = length(direction);
		direction = direction / l;
		for(int s = 1; s <= sampleCount; ++s) {
			float z = float(s) * isc;
			float density = min(0.5 * l * thickCloudDensity(
				a.lightRay.origin + direction * l * z,
				planet), 1.0);
			occlusion = occlusion * mix(1.0, 0.95, density);
		}
	}
	return vec2(
		occlusion,
		min(2.0 * thickCloudDensity(a.lightRay.origin, planet), 1.0));
}
//------------------------------------------------------------------------------
vec4 sunlightColor(AtmosphereSample a) {
	return mix(
		mixColor012n(
			vec4(1.5), vec4(1.4),
			vec4(1.3, 0.9, 0.6, 1.0),
			vec4(1.2, 0.6, 0.4, 0.9),
			a.atmDistRatio, 0.5),
		vec4(0.0),
		a.planetShadow);
}
//------------------------------------------------------------------------------
vec4 clearAirColor(AtmosphereSample a, float sampleLen) {
	float vapor = min(
		pow(abs(a.directLight), 16.0) +
		a.vaporDensity *
		a.atmDensity *
		sampleLen,
		1.0);

	return mix(
		mix(
			mix(
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
				sqrt(to01(a.viewRay.direction.y))),
			mixColor012n(
				vec4(1.10, 1.10, 1.10, 0.4),
				vec4(0.96, 0.95, 0.94, 0.4),
				vec4(0.82, 0.80, 0.79, 0.3),
				vec4(0.58, 0.43, 0.41, 0.2),
				a.atmDistRatio, 0.5),
			min(vapor, 1.0)),
		mix(
			vec4(0.0, 0.0, 0.0, 1.0),
			vec4(0.0, 0.0, 0.0, 0.3),
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

	const float airDensityMult = 20.0;
	float sampleLen = 25.0;
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
		vec4 airLight = clearAirColor(a, sampleLen);
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
		vec4 airLight = clearAirColor(a, sampleLen);
		color = mix(
			mix(
				mix(color, airLight, airDensityMult * sampleRatio),
				sunLight,
				a.hitLight*airLight.a*cloud.y),
			sunLight * cloud.x,
			cloud.y);
	}

	sampleLen = 10.0;
	sampleRatio = sampleLen / atmThickness;

	for(float dist = cloudHitNear; dist > sampleLen; dist -= sampleLen) {
		AtmosphereSample a = atmSample(
			sunDirection,
			sunApparentAngle,
			viewRay,
			dist,
			planet,
			atmosphere);
		vec4 airLight = clearAirColor(a, sampleLen);
		color = mix(color, airLight, airDensityMult * sampleRatio * airLight.a);
	}

	return color;
}
//------------------------------------------------------------------------------
vec4 surfaceColor(Ray viewRay, float rayDist, Sphere planet) {
	float f = exp(-rayDist/500.0);
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

