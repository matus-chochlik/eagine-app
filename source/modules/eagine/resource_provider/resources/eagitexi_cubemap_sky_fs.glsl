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
const float phi = 1.6180339887;

//------------------------------------------------------------------------------
float to01(float x) {
	return 0.5 + 0.5 * x;
}

float fib(int n) {
	vec2 f = vec2(1.0, 1.0);
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
	bool isValid;
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
		vec2(ceil(alt * steps / pow(scale, 1.0 / 3.0)) * phi * sca) +
		(cloudOffset * pow(offset.x, 0.0125 * pi)) / scale +
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
		7.1,
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
	return tilingSample(cloudCoord(
		location,
		planet,
		offset,
		scale,
		23.1,
		0.0,
		atmThickness*vaporThickness).xy);
}
//------------------------------------------------------------------------------
float thinCloudDensity(vec3 location, Sphere planet) {
	float alt = distance(location, planet.center) - planet.radius;
	alt = alt / (atmThickness*vaporThickness);
	if(alt > 1.0) {
		return 0.0;
	}
	float gnd = clamp(alt, 0.0, 1.0);
	float s256000 = thinCloudSample(location, planet, fib2( 1, 2),257.131*phi);
	float s128000 = thinCloudSample(location, planet, fib2( 2, 3),127.903*phi);
	float s016000 = thinCloudSample(location, planet, fib2( 3, 4), 16.111*phi);
	float s004000 = thinCloudSample(location, planet, fib2( 4, 5),  4.531*phi);
	float s000500 = thinCloudSample(location, planet, fib2( 5, 7),  0.523*phi);
	float s000125 = thinCloudSample(location, planet, fib2( 6, 8),  0.131*phi);
	float s000065 = thinCloudSample(location, planet, fib2( 7, 9),  0.061*phi);
	float s000032 = thinCloudSample(location, planet, fib2( 8,10),  0.033*phi);
	float s000016 = thinCloudSample(location, planet, fib2( 9,11),  0.017*phi);

	float d = mix(sqrt(s256000*s128000*s016000*s004000), 1.0, 0.25);
	d -= s000500 * 0.19;
	d -= s000125 * 0.18;
	d -= s000065 * 0.17;
	d -= s000032 * 0.16;
	d -= s000016 * 0.15;

	return max(d, 0.0) * pow(gnd, 2.0) * atmThickness;
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
float cloudCutout(float sam, float lo, float hi) {
	float sc = sqrt(cloudiness);
	return (sam - mix(hi, lo, sc)) * mix(0.90/(1.0 - hi), 1.05/(1.0 - lo), sc);
}
//------------------------------------------------------------------------------
float thickCloudDensity(vec3 location, Sphere planet) {
	float s2560000 = thickCloudSample(location, planet, fib2( 1, 2), 253.0017);
	float s0640000 = thickCloudSample(location, planet, fib2( 2, 3),  64.0013);
	float s0320000 = thickCloudSample(location, planet, fib2( 3, 4),  32.0011);
	float s0160000 = thickCloudSample(location, planet, fib2( 4, 5),  16.0023);
	float s0080000 = thickCloudSample(location, planet, fib2( 5, 6),   8.0019);
	float s0040000 = thickCloudSample(location, planet, fib2( 6, 7),   4.0003);
	float s0020000 = thickCloudSample(location, planet, fib2( 7, 8),   2.1817);
	float s0010000 = thickCloudSample(location, planet, fib2( 8, 9),   1.1313);
	float s0005000 = thickCloudSample(location, planet, fib2( 9,10),   0.5111);
	float s0002500 = thickCloudSample(location, planet, fib2(10,11),   0.2531);
	float s0001250 = thickCloudSample(location, planet, fib2(11,12),   0.1253);
	float s0000625 = thickCloudSample(location, planet, fib2(12,13),   0.0627);
	float snoise1  = thickCloudSample(location, planet, fib2(13,14), 0.01*phi);
	float snoise2  = thickCloudSample(location, planet, fib2(14,15), 0.002*pi);

	float sc = sqrt(cloudiness);
	float cc256 = cloudCutout(sqrt(s2560000), 0.01, 0.41);
	float cc064 = cloudCutout(sqrt(s0640000), 0.01, 0.43);
	float cc032 = cloudCutout(sqrt(s0320000), 0.01, 0.47);
	float cc016 = cloudCutout(sqrt(s0160000), 0.03, 0.53);
	float cc008 = cloudCutout(sqrt(s0080000), 0.07, 0.57);
	float cc004 = cloudCutout(sqrt(s0040000), 0.13, 0.61);
	float d0 = cc256 * cc064 * cc032 * cc016 * cc008 * cc004;
	float m2 = 1.0 - max(d0 - mix(0.005, mix(0.03, 0.17, cloudiness), cc016), 0.0);
	float m1 = 1.0 - max(d0 - mix(0.003, mix(0.01, 0.13, cloudiness), cc008), 0.0);
	float m0 = 1.0 - sign(max(d0 - mix(0.002, 0.03, s0005000), 0.0));
	float d1 = mix(4.0, 32.0, sc) * (d0 - mix(1.0/4.0, 1.0/32.0, sc));
	d1 = mix(min(d1, 1.4), sign(d1), 0.75);

	d1 -= m2 * pow(s0020000, 3.0);
	d1 -= m2 * pow(s0010000, 3.0);
	d1 -= m2 * pow(s0005000, 3.0);
	d1 -= m1 * pow(s0002500, 3.0);
	d1 -= m1 * pow(s0001250, 3.0);
	d1 -= m1 * pow(s0000625, 3.0);
	d1 -= m0 * pow(snoise1,  3.0);
	d1 -= m0 * pow(snoise2,  3.0);

	return clamp(d1, 0.0, 1.0);
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
	if(a.cloudsIntersection.isValid) {
		vec3 direction = a.cloudsIntersection.far - a.lightRay.origin;
		float l = length(direction);
		if(l > 1.0) {
			direction /= l;
			l = min(l, cloudThickness * 2.0);
			float invl = 1.0 / l;

			float cloudsBtm = cloudAltitude - cloudThickness*0.5;

			vec3 location = a.lightRay.origin;
			float alt = distance(location, planet.center) - planet.radius;
			alt = sqrt(clamp((alt - cloudsBtm) / cloudThickness, 0.0, 1.0));
			alt = 1.0 - pow(5.0 * alt * exp(-5.0 * alt * alt), 3.0);

			float sl = 20.0;
			float st = 0.0;
			while(st < l) {
				location = a.lightRay.origin + direction * st;
				float density = thickCloudDensity(location, planet);
				float lf =  mix(0.00015, 0.0015, alt) / shadow;
				float sf = -mix(0.030, 0.015, alt) * pow(shadow, 4.0);
				float of = mix(mix(lf, sf, density), 0.0, pow(st*invl, 2.0));
				shadow = clamp(shadow + of, 0.01, 1.0);
				st += sl;
				sl *= 1.1;
			}
		}
	}
	return AtmosphereShadow(
		mix(sqrt(accum.planetShadow), 1.0, a.planetShadow),
		shadow);
}
//------------------------------------------------------------------------------
AtmosphereShadow atmShadow2(
	AtmosphereSample a,
	Sphere planet,
	AtmosphereShadow accum) {
	float shadow = 1.0;
	if(a.cloudsIntersection.isValid) {
		vec3 direction = a.cloudsIntersection.far - a.cloudsIntersection.near;
		float l = length(direction);
		if(l > 1.0) {
			direction /= l;
			l = min(l, cloudThickness * 2.0);
			float invl = 1.0 / l;

			float sl = 100.0;
			float st = 0.0;
			while(st < l) {
				vec3 location = a.cloudsIntersection.near + direction * st;
				float density = thickCloudDensity(location, planet);
				float sf = -0.020 * pow(shadow, 4.0);
				float of = mix(mix(0.0001, sf, density), 0.0, st*invl);
				shadow = clamp(shadow + of, 0.01, 1.0);
				st += sl;
				sl *= 1.1;
			}
		}
	}
	return AtmosphereShadow(
		mix(pow(accum.planetShadow, 2.0), 1.0, a.planetShadow),
		shadow);
}
//------------------------------------------------------------------------------
vec4 sunlightColor(AtmosphereSample a, AtmosphereShadow s) {
	return mixColor012n(
		vec4(1.5), vec4(1.4),
		vec4(1.3, 1.2, 1.0, 1.1),
		vec4(1.3, 0.8, 0.5, 0.9),
		a.atmDistRatio, 0.5);
}
//------------------------------------------------------------------------------
vec4 ambientColor(AtmosphereSample a, AtmosphereShadow s) {
	return mixColor012n(
		vec4(1.3), vec4(1.2),
		vec4(0.97, 0.95, 0.93, 0.9),
		vec4(0.81, 0.80, 0.83, 0.8),
		a.atmDistRatio, 0.5) * s.planetShadow;
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
				vec4(0.30, 0.40, 0.45, 0.30),
				vec4(0.15, 0.20, 0.40, 0.20),
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
			vec4(0.47, 0.46, 0.40, 0.45),
			vec4(0.47, 0.42, 0.40, 0.43),
			vec4(0.45, 0.38, 0.35, 0.40),
			vec4(0.35, 0.28, 0.25, 0.30),
			a.atmDistRatio * 0.7, 0.5),
		mixColor012n(
			vec4(0.50, 0.50, 0.49, 0.50),
			vec4(0.44, 0.42, 0.41, 0.38),
			vec4(0.39, 0.35, 0.32, 0.34),
			vec4(0.38, 0.32, 0.30, 0.30),
			a.atmDistRatio, 1.0),
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
			vec4(0.94, 0.93, 0.95, alp),
			vec4(0.91, 0.90, 0.92, alp),
			vec4(0.86, 0.85, 0.87, alp),
			vec4(0.83, 0.82, 0.84, alp),
			a.atmDistRatio * 0.7, 0.5),
		mixColor012n(
			vec4(1.01, 1.01, 1.01, alp),
			vec4(0.97, 0.97, 0.98, alp),
			vec4(0.95, 0.95, 0.95, alp),
			vec4(0.93, 0.91, 0.89, alp),
			a.atmDistRatio, 1.0),
		shadow);

	vec4 overcastVaporColor = mix(
		mixColor012n(
			vec4(0.83, 0.82, 0.85, alp),
			vec4(0.75, 0.74, 0.77, alp),
			vec4(0.66, 0.65, 0.67, alp),
			vec4(0.54, 0.53, 0.55, alp),
			a.atmDistRatio * 0.7, 0.5),
		mixColor012n(
			vec4(0.99, 0.99, 1.00, alp),
			vec4(0.95, 0.95, 0.96, alp),
			vec4(0.91, 0.91, 0.91, alp),
			vec4(0.87, 0.86, 0.85, alp),
			a.atmDistRatio, 1.0),
		shadow);

	vec4 darkVaporColor = mixColor012n(
			vec4(0.21, 0.20, 0.22, alp),
			vec4(0.16, 0.15, 0.17, alp),
			vec4(0.12, 0.11, 0.13, alp),
			vec4(0.05, 0.05, 0.05, alp),
			a.atmDistRatio * 0.7, 0.5);

	float dl = to01(a.directLight);
	float pcs = mix(
		mix(mix(0.04, 0.08, dl), mix(0.34, 0.46, dl), s.cloudShadow),
		mix(mix(0.29, 0.37, dl), mix(0.75, 0.95, dl), s.cloudShadow),
		pow(a.sunUp, 2.0));

	float cs = mix(s.cloudShadow, 1.0, mix(0.97, 0.75, cloudiness));

	vec4 lightVaporColor = mix(
		mix(clearVaporColor, overcastVaporColor, pow(cloudiness, 3.0)),
		mix(ambientColor(a, s), sunlightColor(a, s), shadow),
		s.cloudShadow);

	return mix(
		darkVaporColor * pcs,
		lightVaporColor * cs,
		mix(pcs, cs, s.planetShadow) *
		mix(pcs, cs, mix(1.0, sqrt(dl), cloudiness)));
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
		float density = a.vaporDensity * sampleRatio;
		airColor = mix(airColor, vaporColor(a, as), density);
		airColor = mix(airColor, sunlightColor(a, as), a.hitLight);

		color = mix(color, airColor, airDensityMult * sampleRatio * airColor.a);
	}

	sampleLen = 15.0;
	sampleRatio = sampleLen / atmThickness;

	for(float dist = cloudHitFar; dist > cloudHitNear; dist -= sampleLen) {
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

		float density = a.vaporDensity * sampleRatio;
		airColor = mix(airColor, vapColor, density);
		airColor = mix(airColor, sunlightColor(a, as), a.hitLight * as.cloudShadow);
		color = mix(color, airColor, airDensityMult * sampleRatio * airColor.a);

		float densityMult =
			thickCloudSample(a.lightRay.origin, planet, vec2(1, 1), 23.1817);
		density = thickCloudDensity(a.lightRay.origin, planet) *
			mix(0.003, 0.1, densityMult);
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
		float density = a.vaporDensity * sampleRatio;
		float airMult = airDensityMult * sampleRatio * airColor.a;
		airColor = mix(airColor, vaporColor(a, as), density);

		color = mix(color, airColor, airMult);
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

