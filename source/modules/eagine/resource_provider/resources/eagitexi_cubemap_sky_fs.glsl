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
uniform float glowStrength;
uniform float aboveGround;
uniform vec2 cloudOffset;
uniform vec3 sunDirection;
uniform float sunApparentAngle;
uniform float tilingSide = 512;
uniform sampler2D tilingTex;
const float pi = radians(180.0);

//------------------------------------------------------------------------------
float to01(float x) {
	return 0.5 + 0.5 * x;
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
vec3 cloudCoord(
	vec3 location,
	Sphere planet,
	vec2 offset,
	float scale,
	float steps,
	float cloudsBtm,
	float cloudsTop) {
	steps *= cloudThickness / 8000.0;
	float alt = distance(location, planet.center) - planet.radius;
	alt = (alt - cloudsBtm) / (cloudsTop - cloudsBtm);
	location = location - planet.center;
	location = normalize(location);
	vec2 sph = vec2(atan(location.y, location.x) + pi, asin(location.z));
	vec2 sca = vec2(8.0 / scale);
	offset = (sca * offset / tilingSide);
	return vec3(
		sph * sca +
		vec2(ceil(alt * steps / pow(scale, 1.0 / 3.0)) * pi * sca) +
		(cloudOffset * pow(offset.x, 0.0618)) / scale +
		offset,
		alt);
}
//------------------------------------------------------------------------------
vec3 cloudCoord(vec3 location, Sphere planet, vec2 offset, float scale) {
	return cloudCoord(
		location,
		planet,
		offset,
		scale,
		8.3,
		cloudAltitude - cloudThickness * 0.5,
		cloudAltitude + cloudThickness * 0.5);
}
//------------------------------------------------------------------------------
float tilingSample(vec2 coord) {
	coord = coord * (512.0 / tilingSide);
	return texture(tilingTex, coord).r;
}
//------------------------------------------------------------------------------
float tilingSample(vec3 location, Sphere planet, vec2 offset, float scale) {
	return tilingSample(cloudCoord(location, planet, offset, scale).xy);
}
//------------------------------------------------------------------------------
float thinCloudSample(vec3 location, Sphere planet, vec2 offset, float scale) {
	vec3 coord = cloudCoord(
		location,
		planet,
		offset,
		scale,
		93.1,
		-atmThickness*vaporThickness,
		+atmThickness*vaporThickness);
	return tilingSample(coord.xy) * (1.0 - max(abs(coord.z - 1.0), 0.0));
}
//------------------------------------------------------------------------------
float thinCloudDensity(vec3 location, Sphere planet) {
	return 0.0;
	/*
	const float om = 0.618;
	return 0.5*
		thinCloudSample(location, planet, om*fib2( 3,  4), 7.71)*
		thinCloudSample(location, planet, om*fib2( 5,  6), 4.31)*
		thinCloudSample(location, planet, om*fib2( 7,  8), 3.11)*
		thinCloudSample(location, planet, om*fib2( 9, 10), 1.11)*
		thinCloudSample(location, planet, om*fib2(11, 12), 0.31)*
		thinCloudSample(location, planet, om*fib2(12, 13), 0.13)*
		thinCloudSample(location, planet, om*fib2(12, 13), 0.07);
		*/
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
float thickCloudSample(vec3 location, Sphere planet, vec2 offset, float scale) {
	return tilingSample(location, planet, offset, scale * 0.5);
}
//------------------------------------------------------------------------------
float cloudCutout(float s, float low, float hi) {
	return (s - mix(low, hi, cloudiness)) *
		mix(1.0/(1.0 - low), 1.0/(1.0 - hi), cloudiness);
}
//------------------------------------------------------------------------------
float thickCloudDensity(vec3 location, Sphere planet) {
	float c = cloudiness;
	float s1280000 = thickCloudSample(location, planet, fib2( 1, 2),128.0017);
	float s0640000 = thickCloudSample(location, planet, fib2( 2, 3), 64.0013);
	float s0320000 = thickCloudSample(location, planet, fib2( 3, 4), 32.0011);
	float s0160000 = thickCloudSample(location, planet, fib2( 4, 5), 16.0023);
	float s0080000 = thickCloudSample(location, planet, fib2( 5, 6),  8.0019);
	float s0040000 = thickCloudSample(location, planet, fib2( 6, 7),  4.0003);
	float s0020000 = thickCloudSample(location, planet, fib2( 7, 8),  2.1817);
	float s0010000 = thickCloudSample(location, planet, fib2( 8, 8),  1.1313);
	float s0005000 = thickCloudSample(location, planet, fib2( 9,10),  0.5111);
	float s0002500 = thickCloudSample(location, planet, fib2(10,11),  0.2531);
	float s0001250 = thickCloudSample(location, planet, fib2(11,12),  0.1253);
	float s0000625 = thickCloudSample(location, planet, fib2(12,13),  0.0627);
	float snoise1  = thickCloudSample(location, planet, fib2(13,14), 0.01*pi);

	float e = pow(
		s0020000*
		s0010000*
		s0005000*
		s0002500*
		s0001250*
		s0000625*
		snoise1,
		1.0 / 7.0);

	float d = 1.5;
	d -= cloudCutout(sqrt(s1280000), 0.11, 0.63);
	d -= cloudCutout(sqrt(s0640000), 0.17, 0.61);
	d -= cloudCutout(sqrt(s0320000), 0.23, 0.57);
	d -= cloudCutout(sqrt(s0160000), 0.37, 0.53);
	d -= cloudCutout(sqrt(s0080000), 0.41, 0.51);
	d -= cloudCutout(sqrt(s0040000), 0.43, 0.49);
	d += e;

	return clamp(d, 0.0, 1.0);
}
//------------------------------------------------------------------------------
struct AtmosphereSample {
	Ray viewRay;
	Ray lightRay;
	Segment cloudsIntersection;
	float hitLight;
	float directLight;
	float sunUp;
	float altitude;
	float atmDensity;
	float atmDistance;
	float atmDistRatio;
	float vaporDensity;
	float planetShadow;
};
//------------------------------------------------------------------------------
struct AtmosphereShadow {
	float planetShadow;
	float cloudShadow;
};
//------------------------------------------------------------------------------
AtmosphereSample atmSample(
	vec3 lightDirection,
	float lightAngle,
	Ray viewRay,
	float rayDist,
	Sphere planet,
	Sphere atmosphere) {
	Ray lightRay0 = raySample(viewRay, rayDist, lightDirection);
	Ray lightRayS = raySample(viewRay, rayDist * 3.0, lightDirection);
	float directLight = dot(viewRay.direction, lightRay0.direction);
	float atmDistance = max(hitNear(sphereHit(lightRay0, atmosphere)), 0.0);

	float planetHit = sign(max(hitNear(sphereHit(lightRayS, planet)), 0.0));
	float planetShadow = 1.0 - planetHit;
	float altitude = distance(lightRay0.origin, planet.center) - planet.radius;

	return AtmosphereSample(
		viewRay,
		lightRay0,
		cloudsIntersection(lightRay0, planet),
		max(sign(lightAngle-acos(directLight)), 0.0),
		directLight,
		sqrt(to01(lightDirection.y)),
		clamp(altitude / planetRadius, 0.0, 1.0),
		pow(clamp(
			distance(lightRay0.origin, planet.center) - planet.radius /
			atmosphere.radius - planet.radius,
			0.0, 1.0), 1.21),
		atmDistance,
		atmDistance / atmThickness,
		thinCloudDensity(lightRay0.origin, planet),
		planetShadow);
}
//------------------------------------------------------------------------------
AtmosphereShadow atmShadow0(
	AtmosphereSample a,
	Sphere planet,
	AtmosphereShadow accum) {
	return AtmosphereShadow(
		mix(pow(accum.planetShadow, 2.0), 1.0, a.planetShadow),
		1.0);
}
//------------------------------------------------------------------------------
AtmosphereShadow atmShadow1(
	AtmosphereSample a,
	Sphere planet,
	AtmosphereShadow accum) {
	float shadow = 1.0;
	if(a.cloudsIntersection.is_valid) {
		vec3 direction = a.cloudsIntersection.far - a.lightRay.origin;
		float l = length(direction);
		float sf = mix(0.850, 0.987, accum.planetShadow);
		int sampleCount = min(int(l * 4.0), 192);
		if(sampleCount > 0) {
			float isc = 1.0 / float(sampleCount);
			for(int s = 1; s <= sampleCount; ++s) {
				vec3 location = a.lightRay.origin + direction * float(s) * isc;
				float tcd = thickCloudDensity(location, planet);
				float density = min(2.5 * cloudiness * tcd, 1.0);
				shadow = shadow * mix(1.0, sf, density);
			}
		}
	}
	return AtmosphereShadow(
		mix(pow(accum.planetShadow, 2.0), 1.0, a.planetShadow),
		shadow);
}
//------------------------------------------------------------------------------
AtmosphereShadow atmShadow2(
	AtmosphereSample a,
	Sphere planet,
	AtmosphereShadow accum) {
	float shadow = 1.0;
	if(a.cloudsIntersection.is_valid) {
		const int sampleCount = 50;
		const float isc = 1.0 / float(sampleCount);
		float density = 0.0;
		for(int s = 0; s <= sampleCount; ++s) {
			vec3 location = mix(
				a.cloudsIntersection.far,
				a.cloudsIntersection.near,
				float(s) * isc);
			density += sign(thickCloudDensity(location, planet));
		}
		shadow = 1.0 - density * isc;
	}
	return AtmosphereShadow(
		mix(pow(accum.planetShadow, 2.0), 1.0, a.planetShadow),
		shadow);
}
//------------------------------------------------------------------------------
vec4 sunlightColor(AtmosphereSample a, AtmosphereShadow s) {
	return mixColor012n(
		vec4(1.5), vec4(1.4),
		vec4(1.3, 1.15, 1.0, 1.1),
		vec4(1.3, 0.7, 0.5, 0.9),
		a.atmDistRatio, 0.5);
}
//------------------------------------------------------------------------------
vec4 clearAirColor(AtmosphereSample a, AtmosphereShadow s) {
	float shadow = s.planetShadow * s.cloudShadow;
	float directLight = pow(to01(a.directLight), 16.0) * shadow;

	vec4 lightAirColor = mix(
		mix(
			mixColor012n(
				vec4(0.30, 0.45, 0.75, 0.60),
				vec4(0.25, 0.40, 0.70, 0.53),
				vec4(0.50, 0.40, 0.25, 0.30),
				vec4(0.40, 0.10, 0.25, 0.20),
				a.atmDistRatio, 1.0),
			mixColor012n(
				vec4(0.30, 0.45, 0.75, 0.60),
				vec4(0.30, 0.40, 0.65, 0.53),
				vec4(0.35, 0.40, 0.60, 0.30),
				vec4(0.50, 0.40, 0.35, 0.20),
				a.atmDistRatio * 0.7, 0.5),
			a.sunUp),
		mix(
			mixColor012n(
				vec4(0.32, 0.45, 0.73, 0.60),
				vec4(0.30, 0.40, 0.60, 0.53),
				vec4(0.45, 0.40, 0.30, 0.30),
				vec4(0.38, 0.13, 0.28, 0.20),
				a.atmDistRatio, 1.0),
			mixColor012n(
				vec4(0.34, 0.45, 0.71, 0.60),
				vec4(0.30, 0.40, 0.66, 0.53),
				vec4(0.35, 0.40, 0.55, 0.30),
				vec4(0.37, 0.38, 0.39, 0.20),
				a.atmDistRatio * 0.7, 0.5),
			a.sunUp),
		pow(cloudiness, 3.0));

	lightAirColor = lightAirColor + directLight * mix(
		mixColor012n(
			vec4(0.50, 0.50, 0.49, 0.50),
			vec4(0.44, 0.42, 0.41, 0.38),
			vec4(0.39, 0.35, 0.32, 0.34),
			vec4(0.38, 0.32, 0.30, 0.30),
			a.atmDistRatio, 1.0),
		mixColor012n(
			vec4(0.37, 0.30, 0.25, 0.30),
			vec4(0.34, 0.27, 0.23, 0.25),
			vec4(0.32, 0.23, 0.20, 0.23),
			vec4(0.23, 0.18, 0.15, 0.20),
			a.atmDistRatio * 0.7, 0.5),
		a.sunUp);

	vec4 darkAirColor = mix(
		mix(
			mixColor012n(
				vec4(0.50, 0.30, 0.70, 0.50),
				vec4(0.45, 0.30, 0.66, 0.33),
				vec4(0.35, 0.20, 0.45, 0.15),
				vec4(0.23, 0.17, 0.26, 0.10),
				a.atmDistRatio, 1.0),
			mixColor012n(
				vec4(0.45, 0.30, 0.75, 0.60),
				vec4(0.40, 0.25, 0.70, 0.35),
				vec4(0.30, 0.20, 0.45, 0.20),
				vec4(0.21, 0.19, 0.27, 0.15),
				a.atmDistRatio * 0.7, 0.5),
			a.sunUp),
		mix(
			mixColor012n(
				vec4(0.58, 0.58, 0.56, 0.57),
				vec4(0.42, 0.42, 0.40, 0.42),
				vec4(0.32, 0.31, 0.30, 0.30),
				vec4(0.22, 0.19, 0.19, 0.20),
				a.atmDistRatio, 1.0),
			mixColor012n(
				vec4(0.50, 0.50, 0.52, 0.50),
				vec4(0.39, 0.39, 0.41, 0.39),
				vec4(0.27, 0.27, 0.29, 0.27),
				vec4(0.11, 0.11, 0.12, 0.11),
				a.atmDistRatio * 0.7, 0.5),
			a.sunUp),
		pow(cloudiness, 3.0));

	vec4 airColor = mix(darkAirColor, lightAirColor, shadow);
	airColor = vec4(
		airColor.rgb,
		mix(airColor.a * shadow, 1.0, a.hitLight * shadow));


	airColor += vec4(0.20, 0.35, 0.60, 0.25) *
		 (1.0 - shadow) * pow(a.altitude, 0.25);

	return airColor;
}
//------------------------------------------------------------------------------
vec4 vaporColor(AtmosphereSample a, AtmosphereShadow s) {
	const float alp = 1.0;
	float shadow = s.planetShadow * s.cloudShadow;
	float directLight = abs(a.directLight) * shadow;

	vec4 clearVaporColor = mix(
		mixColor012n(
			vec4(0.95, 0.95, 0.95, alp),
			vec4(0.91, 0.91, 0.92, alp),
			vec4(0.86, 0.85, 0.87, alp),
			vec4(0.83, 0.82, 0.84, alp),
			a.atmDistRatio * 0.7, 0.5),
		mixColor012n(
			vec4(1.02, 1.02, 1.01, alp),
			vec4(1.00, 0.99, 0.98, alp),
			vec4(0.96, 0.95, 0.94, alp),
			vec4(0.92, 0.91, 0.90, alp),
			a.atmDistRatio, 1.0),
		shadow);

	vec4 overcastVaporColor = mix(
		mixColor012n(
			vec4(0.95, 0.94, 0.97, alp),
			vec4(0.90, 0.89, 0.92, alp),
			vec4(0.85, 0.85, 0.86, alp),
			vec4(0.74, 0.74, 0.75, alp),
			a.atmDistRatio * 0.7, 0.5),
		mixColor012n(
			vec4(1.01, 1.01, 1.01, alp),
			vec4(0.98, 0.98, 0.97, alp),
			vec4(0.96, 0.95, 0.94, alp),
			vec4(0.89, 0.89, 0.89, alp),
			a.atmDistRatio, 1.0),
		shadow);

	vec4 lightVaporColor = mix(
		mix(clearVaporColor, sunlightColor(a, s), directLight),
		overcastVaporColor,
		pow(cloudiness, 3.0));

	vec4 darkVaporColor = mixColor012n(
			vec4(0.21, 0.21, 0.22, alp),
			vec4(0.16, 0.16, 0.16, alp),
			vec4(0.12, 0.12, 0.12, alp),
			vec4(0.05, 0.05, 0.05, alp),
			a.atmDistRatio * 0.7, 0.5);

	float dl = to01(a.directLight);
	float pcs = mix(
		mix(mix(0.04, 0.06, dl), mix(0.06, 0.10, dl), s.cloudShadow),
		mix(mix(0.50, 0.55, dl), mix(0.53, 0.59, dl), s.cloudShadow),
		pow(a.sunUp, 2.0));
	float cs = mix(s.cloudShadow, 1.0, 0.7);
	return cs * mix(
		darkVaporColor,
		lightVaporColor,
		mix(pcs, cs, s.planetShadow) * cs);
}
//------------------------------------------------------------------------------
vec4 skyColor(Ray viewRay, Sphere planet, Sphere atmosphere) {
	float cloudHitNear = hitNear(sphereHit(viewRay, Sphere(
		planet.center,
		planet.radius+cloudAltitude-cloudThickness*0.5)));
	float cloudHitFar = hitFar(sphereHit(viewRay, Sphere(
		planet.center,
		planet.radius+cloudAltitude+cloudThickness*0.5)));

	float directLight = max(dot(viewRay.direction, sunDirection), 0.0);
	float atmosphereHit = hitNear(sphereHit(viewRay, atmosphere));

	vec4 color = vec4(0.0);

	const float airDensityMult = 5.0;
	float sampleLen = 50.0;
	float sampleRatio = sampleLen / atmThickness;

	float shadowNum = 1.0;
	AtmosphereShadow accumShadow = AtmosphereShadow(1.0, 1.0);
	for(float dist = atmosphereHit; dist > cloudHitFar; dist -= sampleLen) {
		AtmosphereSample a = atmSample(
			sunDirection,
			sunApparentAngle,
			viewRay,
			dist,
			planet,
			atmosphere);

		AtmosphereShadow as = AtmosphereShadow(
			accumShadow.planetShadow / shadowNum,
			accumShadow.cloudShadow);

		AtmosphereShadow s = atmShadow0(a, planet, as);
		accumShadow.planetShadow += mix(a.planetShadow, s.planetShadow, 0.5);
		accumShadow.cloudShadow *= s.cloudShadow;
		shadowNum += 1.0;

		vec4 airColor = clearAirColor(a, as);
		float density = min(a.vaporDensity, 1.0);
		airColor = mix(airColor, vaporColor(a, as), density);
		airColor = mix(airColor, sunlightColor(a, as), a.hitLight);

		color = mix(color, airColor, airDensityMult * sampleRatio * airColor.a);
	}

	int cloudSamples = 200;
	float isc = 1.0 / float(cloudSamples);
	sampleLen = (cloudHitFar - cloudHitNear) * isc;
	sampleRatio = sampleLen / atmThickness;

	for(int si = 0; si <= cloudSamples; ++si) {
		float dist = mix(cloudHitFar, cloudHitNear, float(si) * isc);
		AtmosphereSample a = atmSample(
			sunDirection,
			sunApparentAngle,
			viewRay,
			dist,
			planet,
			atmosphere);

		AtmosphereShadow as = AtmosphereShadow(
			accumShadow.planetShadow / shadowNum,
			accumShadow.cloudShadow);

		AtmosphereShadow s = atmShadow1(a, planet, as);
		accumShadow.planetShadow += mix(a.planetShadow, s.planetShadow, 0.5);
		accumShadow.cloudShadow *= s.cloudShadow;
		shadowNum += 1.0;

		vec4 airColor = clearAirColor(a, as);
		vec4 vapColor = vaporColor(a, as);

		float density = min(a.vaporDensity, 1.0);
		airColor = mix(airColor, vapColor, density);

		density = min(5.0 * thickCloudDensity(a.lightRay.origin, planet), 1.0);
		airColor = mix(airColor, sunlightColor(a, as), a.hitLight * as.cloudShadow);

		color = mix(color, airColor, airDensityMult * sampleRatio * airColor.a);
		color = mix(color, vapColor, density);
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

		AtmosphereShadow as = AtmosphereShadow(
			accumShadow.planetShadow / shadowNum,
			accumShadow.cloudShadow);

		AtmosphereShadow s = atmShadow2(a, planet, as);
		accumShadow.planetShadow += mix(a.planetShadow, s.planetShadow, 0.5);
		accumShadow.cloudShadow *= s.cloudShadow;
		shadowNum += 1.0;

		vec4 airColor = clearAirColor(a, as);
		float density = min(a.vaporDensity, 1.0);
		float airMult = airDensityMult * sampleRatio * airColor.a;
		airColor = mix(airColor, vaporColor(a, as), density);

		color = mix(color, airColor, airMult);
		color += mix(airColor, sunlightColor(a, as), 0.5) *
			airMult * glowStrength * as.cloudShadow;
	}

	return color;
}
//------------------------------------------------------------------------------
vec4 surfaceColor(Ray viewRay, float rayDist, Sphere planet) {
	float f = exp(-rayDist / 500.0);
	return mix(
		mix(vec4(0.23, 0.20, 0.17, 0.0), vec4(0.40, 0.30, 0.20, 0.0), f),
		mix(vec4(0.13, 0.11, 0.08, 0.0), vec4(0.30, 0.25, 0.20, 0.0), f),
		cloudiness);
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

