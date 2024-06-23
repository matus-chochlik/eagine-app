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
uniform float haziness;
uniform float glowStrength;
uniform float aboveGround;
uniform vec2 cloudOffset;
uniform vec3 sunDirection;
uniform float sunApparentAngle;
uniform float tilingSide = 512;
uniform sampler2D tilingTex;

float vaporTop = atmThickness * vaporThickness * 1.0;
float vaporBtm = atmThickness * vaporThickness * 0.6;
float cloudTop = cloudAltitude + cloudThickness * 0.5;
float cloudBtm = cloudAltitude - cloudThickness * 0.5;
float cloudinessSqrt = mix(0.19, 1.0, sqrt(cloudiness));

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
struct SampleAccumulatedInfo {
	float vaporShadow;
	float planetShadow;
};
//------------------------------------------------------------------------------
struct TraceInfo {
	Ray viewRay;
	Sphere planet;
	Sphere atmosphere;
};
//------------------------------------------------------------------------------
struct SampleInfo {
	TraceInfo trace;
	SampleAccumulatedInfo accumulated;
	Ray lightRay;
	float hitLight;
	float lightDot;
	float toLight;
	float sunUp;
	float viewUp;
	float atmDensity;
	float atmAltitudeRatio;
	float atmViewDistRatio;
	float atmLightDistRatio;
	float planetShadow;
	float sampleAtmRatio;
};
//------------------------------------------------------------------------------
SampleInfo getSample(
	TraceInfo trace,
	SampleAccumulatedInfo accumulated,
	float rayDist,
	float atmDist,
	float sampleLength,
	vec3 lightDirection,
	float lightAngle) {

	Ray sampleRay = raySample(trace.viewRay, rayDist);
	Ray lightRay = Ray(sampleRay.origin, lightDirection);
	float lightDot = dot(trace.viewRay.direction, lightRay.direction);
	float hitLight = max(sign(lightAngle-acos(lightDot)), 0.0);
	float sunUp = to01(lightDirection.y);
	float viewUp = to01(trace.viewRay.direction.y);

	float atmViewDist =
		max(hitNear(sphereHit(sampleRay, trace.atmosphere)), 0.0);
	float atmLightDist =
		max(hitNear(sphereHit(lightRay, trace.atmosphere)), 0.0);
	float atmAltitude =
		distance(lightRay.origin, trace.planet.center) - trace.planet.radius;
	float atmDensity = pow(clamp(atmAltitude / atmThickness, 0.0, 1.0), 1.21);

	SphereHit planetHit = sphereHit(lightRay, trace.planet);
	float hitDiff = hitFar(planetHit) - hitNear(planetHit);
	float planetShadow = exp(-hitDiff * 2.0 / planetRadius);

	accumulated.planetShadow = mix(planetShadow,
		accumulated.planetShadow,
		sampleLength / atmDist);

	return SampleInfo(
		trace,
		accumulated,
		lightRay,
		hitLight,
		lightDot,
		to01(lightDot),
		sunUp,
		viewUp,
		atmDensity,
		atmAltitude / atmThickness,
		atmViewDist / atmThickness,
		atmLightDist / atmThickness,
		planetShadow,
		sampleLength / atmThickness);
}
//------------------------------------------------------------------------------
SampleInfo getSampleSun(
	TraceInfo trace,
	SampleAccumulatedInfo accumulated,
	float rayDist,
	float atmDist,
	float sampleLength) {
	return getSample(
			trace,
			accumulated,
			rayDist,
			atmDist,
			sampleLength,
			sunDirection,
			sunApparentAngle);
}
//------------------------------------------------------------------------------
vec4 sunlightColor(SampleInfo s) {
	return mixColor012n(
		vec4(1.5), vec4(1.4),
		vec4(1.3, 1.3, 0.9, 1.1),
		vec4(1.3, 0.5, 0.5, 0.9),
		s.atmLightDistRatio * 0.7, 0.5);
}
//------------------------------------------------------------------------------
vec4 clearAirColor(SampleInfo s, float cloudShadow) {
	vec4 lightColor = sunlightColor(s);
	float shadow = s.planetShadow * cloudShadow;
	float groundHaze = pow(
		1.0 - exp(-s.atmViewDistRatio * mix(0.5, 1.0, haziness)), 8.0);
	float lightHaze = pow(s.toLight, 8.0);

	vec4 lightAirColor = mix(
		mixColor012n(
			vec4(0.40, 0.43, 0.87, 0.70),
			vec4(0.44, 0.47, 0.78, 0.63),
			vec4(0.47, 0.43, 0.75, 0.63),
			vec4(0.57, 0.54, 0.46, 0.50),
			s.atmLightDistRatio * 0.7, 0.5),
		mixColor012n(
			vec4(0.50, 0.63, 0.98, 0.80),
			vec4(0.54, 0.57, 0.87, 0.73),
			vec4(0.57, 0.54, 0.85, 0.73),
			vec4(0.67, 0.64, 0.56, 0.60),
			s.atmLightDistRatio * 0.7, 0.5),
		s.toLight);

	vec4 darkAirColor = mix(
		vec4(0.0),
		mix(
			mixColor012n(
				vec4(0.67, 0.62, 0.54, 0.60),
				vec4(0.65, 0.50, 0.55, 0.50),
				vec4(0.40, 0.43, 0.87, 0.35),
				vec4(0.30, 0.43, 0.75, 0.20),
				s.atmLightDistRatio * 0.7, 0.5),
			mixColor012n(
				vec4(0.65, 0.60, 0.58, 0.60),
				vec4(0.59, 0.54, 0.54, 0.50),
				vec4(0.45, 0.45, 0.47, 0.35),
				vec4(0.32, 0.32, 0.33, 0.20),
				s.atmLightDistRatio * 0.7, 0.5),
			cloudiness),
		s.toLight);

	vec4 hazeColor = vec4(mix(
		mix(lightAirColor.rgb, vec3(1.0), 0.92),
		lightColor.rgb,
		shadow * (1.0 - cloudiness)),
		s.toLight * s.planetShadow * mix(1.0, 2.0, haziness));

	vec4 airColor = mix(darkAirColor, lightAirColor, shadow);
	airColor = mix(airColor, hazeColor, groundHaze * haziness);
	airColor = mix(airColor, hazeColor, lightHaze  * haziness);
	
	return mix(airColor, lightColor, s.hitLight * s.accumulated.vaporShadow);
}
//------------------------------------------------------------------------------
vec4 vaporColor(SampleInfo s, vec4 airColor, float cloudShadow) {
	float pshadow = mix(s.planetShadow, s.accumulated.planetShadow, 0.25);
	float cshadow = mix(0.8, 1.0, s.accumulated.vaporShadow) * cloudShadow;

	vec4 vaporColor = vec4(1.0, 1.0, 1.0, 0.95) * pshadow * cshadow;
	vec4 lightColor = sunlightColor(s) *
		mix(0.1 * s.planetShadow, 1.1, cloudShadow);

	return mix(
		mix(vaporColor, airColor, s.accumulated.planetShadow * 0.5),
		mixColor012n(
			mix(vaporColor, lightColor, 0.2),
			mix(vaporColor, lightColor, 0.4),
			mix(vaporColor, lightColor, 0.6),
			mix(vaporColor, lightColor, 0.8),
			s.atmLightDistRatio * 0.7, 0.5),
		pshadow * cshadow);
}
//------------------------------------------------------------------------------
float clearAirDensity(SampleInfo sample) {
	return sample.sampleAtmRatio;
}
//------------------------------------------------------------------------------
struct CloudLayerInfo {
	float topAlt;
	float bottomAlt;
	float thickness;
	float altitudeRatio;
	float steps;
	bool isInside;
};
//------------------------------------------------------------------------------
CloudLayerInfo getCloudLayer(
	SampleInfo sample,
	float steps,
	float top,
	float bottom) {
	float altitudeRatio =
		distance(sample.lightRay.origin, sample.trace.planet.center) -
		sample.trace.planet.radius;
	bool isIn = (altitudeRatio <= top) || (altitudeRatio >= bottom);
	altitudeRatio = clamp((altitudeRatio - bottom) / (top - bottom), 0.0, 1.0);

	return CloudLayerInfo(top, bottom, top - bottom, altitudeRatio, steps, isIn);
}
//------------------------------------------------------------------------------
CloudLayerInfo thinCloudLayer(SampleInfo sample) {
	return getCloudLayer(sample, 257.1, vaporTop, vaporBtm);
}
//------------------------------------------------------------------------------
CloudLayerInfo thickCloudLayer(SampleInfo sample) {
	return getCloudLayer(sample, 33.7, cloudTop, cloudBtm);
}
//------------------------------------------------------------------------------
struct Segment {
	float near;
	float far;
};
//------------------------------------------------------------------------------
Segment layerIntersection(Ray ray, SampleInfo sample, CloudLayerInfo layer) {
	SphereHit topHit = sphereHit(ray, Sphere(
		sample.trace.planet.center,
		sample.trace.planet.radius + layer.topAlt));
	SphereHit bottomHit = sphereHit(ray, Sphere(
		sample.trace.planet.center,
		sample.trace.planet.radius + layer.bottomAlt));

	if(isValidHit(topHit) && isValidHit(bottomHit)) {
		vec2 hMinMax = vec2(
			min(hitNear(topHit), hitNear(bottomHit)),
			max(hitFar(topHit), hitFar(bottomHit)));

		return Segment(hMinMax.x, hMinMax.y);
	} else if(isValidHit(topHit)) {
		return Segment(hitNear(topHit), hitFar(topHit));
	} else if(isValidHit(bottomHit)) {
		return Segment(hitNear(bottomHit), hitFar(bottomHit));
	} else {
		return Segment(0.0, 0.0);
	}
}
//------------------------------------------------------------------------------
vec2 cloudCoord(
	vec3 location,
	Sphere planet,
	CloudLayerInfo layer,
	vec2 offset,
	float scale) {
	vec3 loc = normalize(location - planet.center);
	vec2 sph = vec2(atan(loc.y, loc.x) + pi, asin(loc.z));
	vec2 sca = vec2(8.0 / scale);
	float alt = pow(layer.altitudeRatio, mix(0.5, 2.0, to01(sin(sph.x*3.1))));
	float layerofs = ceil(alt * layer.steps / scale);
	offset = (sca * offset / tilingSide);
	return vec2(
		offset + sph * sca + vec2(phi * scale * layerofs) +
		(cloudOffset * pow(offset.x, 0.025 * pi) * 1.5 * phi) / scale);
}
//------------------------------------------------------------------------------
float tilingSample(vec2 coord) {
	coord = coord * (512.0 / tilingSide);
	return texture(tilingTex, coord).r;
}
//------------------------------------------------------------------------------
float tilingSample(
	vec3 location,
	Sphere planet,
	CloudLayerInfo layer,
	vec2 offset,
	float scale) {
	return tilingSample(cloudCoord(location, planet, layer, offset, scale));
}
//------------------------------------------------------------------------------
float thinCloudSample(
	vec3 location,
	SampleInfo sample,
	CloudLayerInfo layer,
	vec2 offset,
	float scale) {
	return tilingSample(location, sample.trace.planet, layer, offset, scale);
}
//------------------------------------------------------------------------------
float thinCloudDensity(
	SampleInfo sam,
	CloudLayerInfo layer) {
	if(!layer.isInside) {
		return 0.0;
	}
	vec3 loc = sam.lightRay.origin;

	float s256000 = thinCloudSample(loc, sam, layer, fib2( 1, 2),257.131*phi);
	float s128000 = thinCloudSample(loc, sam, layer, fib2( 2, 3),119.903*phi);
	float s016000 = thinCloudSample(loc, sam, layer, fib2( 3, 4), 23.111*phi);
	float s004000 = thinCloudSample(loc, sam, layer, fib2( 4, 5),  4.531*phi);
	float s002000 = thinCloudSample(loc, sam, layer, fib2( 5, 7),  2.313*phi);
	float s000500 = thinCloudSample(loc, sam, layer, fib2( 5, 7),  0.523*phi);
	float s000125 = thinCloudSample(loc, sam, layer, fib2( 6, 8),  0.131*phi);

	float dfac = mix(0.5, 4.0, haziness);
	float dens = sqrt(4.0 * s256000 * s128000 * s016000 * s004000);
	dens -= pow(s002000, 2.0) * 0.41;
	dens -= pow(s000500, 2.0) * 0.37;
	dens -= pow(s000125, 2.0) * 0.31;
	dens *= dfac;

	return min(pow(max(dens, 0.0), 8.0), 2.0) * sam.sampleAtmRatio * dfac;
}
//------------------------------------------------------------------------------
float thickCloudSample(
	vec3 location,
	SampleInfo sample,
	CloudLayerInfo layer,
	vec2 offset,
	float scale) {
	return tilingSample(location, sample.trace.planet, layer, offset, scale);
}
//------------------------------------------------------------------------------
float cloudCutout(float sam) {
	return sqrt(max(0.0, cloudinessSqrt - sam));
}
//------------------------------------------------------------------------------
float thickCloudDensity(
	vec3 loc,
	SampleInfo sam,
	CloudLayerInfo layer,
	float sampleAtmRatio) {
	float s2560000 = thickCloudSample(loc, sam, layer, fib2( 1, 2), 253.0017);
	float s0640000 = thickCloudSample(loc, sam, layer, fib2( 2, 3),  64.0013);
	float s0320000 = thickCloudSample(loc, sam, layer, fib2( 3, 4),  32.0011);
	float s0160000 = thickCloudSample(loc, sam, layer, fib2( 4, 5),  16.0023);
	float s0080000 = thickCloudSample(loc, sam, layer, fib2( 5, 6),   8.0019);
	float s0040000 = thickCloudSample(loc, sam, layer, fib2( 6, 7),   4.0003);
	float s0020000 = thickCloudSample(loc, sam, layer, fib2( 7, 8),   2.1817);
	float s0010000 = thickCloudSample(loc, sam, layer, fib2( 8, 9),   1.1313);
	float s0005000 = thickCloudSample(loc, sam, layer, fib2( 9,10),   0.5111);
	float s0002500 = thickCloudSample(loc, sam, layer, fib2(10,11),   0.2531);
	float s0001250 = thickCloudSample(loc, sam, layer, fib2(11,12),   0.1253);
	float s0000625 = thickCloudSample(loc, sam, layer, fib2(12,13),   0.0627);
	float snoise1  = thickCloudSample(loc, sam, layer, fib2(13,14),  0.01*pi);

	float cc256 = cloudCutout(s2560000);
	float cc064 = cloudCutout(s0640000);
	float cc032 = cloudCutout(s0320000);
	float cc016 = cloudCutout(s0160000);
	float cc008 = cloudCutout(s0080000);
	float cc004 = cloudCutout(s0040000);
	float densi = sqrt(4.0 * cc256 * cc064 * cc032 * cc016 * cc008 * cc004);

	float mask0 = (1.0 - sqrt(max(densi - mix(0.0, 0.011, s0160000),0.0)))*0.37;
	float mask1 = (1.0 - sqrt(max(densi - mix(0.0, 0.005, s0080000),0.0)))*3.00;

	densi -= pow(s0020000, 3.0) * mask0;
	densi -= pow(s0010000, 3.0) * mask0;
	densi -= pow(s0005000, 3.0) * mask0;
	densi -= pow(s0002500, 3.0) * mask0;

	densi = clamp(densi * 16.0, 0.0, 4.0);

	densi -= pow(s0001250, 3.0) * mask1;
	densi -= pow(s0000625, 3.0) * mask1;
	densi -= pow(snoise1 , 3.0) * mask1;

	densi = clamp(densi, 0.0, 3.0);

	return densi * sampleAtmRatio * 30.0;
}
//------------------------------------------------------------------------------
float thickCloudDensity(vec3 loc, SampleInfo sam, CloudLayerInfo layer) {
	return thickCloudDensity(loc, sam, layer, sam.sampleAtmRatio);
}
//------------------------------------------------------------------------------
float thickCloudDensity(SampleInfo sam, CloudLayerInfo layer) {
	return thickCloudDensity(sam.lightRay.origin, sam, layer);
}
//------------------------------------------------------------------------------
float cloudShadowIn(SampleInfo sample, CloudLayerInfo layer) {
	float shadow = 1.0;
	Segment section = layerIntersection(sample.lightRay, sample, layer);

	float l = section.far;
	if(l >= 1.0) {
		l = min(l, layer.thickness);
		float il = 1.0 / l;
		float st = l;
		float sl = 60.0;
		float sar = sl / atmThickness;
		while(st > 0.0) {
			vec3 location = raySample(sample.lightRay, st).origin;
			float altitude =
				distance(location, sample.trace.planet.center) -
				sample.trace.planet.radius;
			if(altitude >= layer.bottomAlt) {
				float density = thickCloudDensity(location, sample, layer, sar);
				density = mix(density, 0.0, st * il);
				shadow = max(shadow - density * mix(0.0, 3.0, shadow), 0.0);
			}
			st -= sl;
		}
	}

	return mix(
		mix(shadow, 1.0, mix(0.25, 0.05, cloudiness)),
		mix(shadow, 1.0, mix(0.75, 0.25, cloudiness)),
		sample.accumulated.planetShadow);
}
//------------------------------------------------------------------------------
float cloudShadowOut(SampleInfo sample, CloudLayerInfo layer) {
	float shadow = 1.0;
	Segment section = layerIntersection(sample.lightRay, sample, layer);

	float l = section.far - section.near;
	if(l >= 1.0) {
		l = min(l, layer.thickness);
		float il = 1.0 / l;
		float st = l;
		float sl = 150.0;
		float sar = sl / atmThickness;
		while(st > 0.0) {
			vec3 location = raySample(sample.lightRay, section.near+st).origin;
			float altitude =
				distance(location, sample.trace.planet.center) -
				sample.trace.planet.radius;
			if(altitude >= layer.bottomAlt) {
				float density = thickCloudDensity(location, sample, layer, sar);
				density = mix(density, 0.0, st * il);
				shadow = max(shadow - density, 0.0);
			}
			st -= sl;
		}
	}

	return shadow;
}
//------------------------------------------------------------------------------
vec4 skyColor(TraceInfo trace) {
	float atmDist = hitNear(sphereHit(trace.viewRay, trace.atmosphere));
	float rayDist = atmDist - 1.0;

	SampleAccumulatedInfo accumulated = SampleAccumulatedInfo(1.0, 1.0);
	SampleInfo sample = getSampleSun(trace, accumulated, rayDist, atmDist, 1.0);

	CloudLayerInfo vaporLayer = thinCloudLayer(sample);
	CloudLayerInfo cloudLayer = thickCloudLayer(sample);

	Segment vaporSection = layerIntersection(trace.viewRay, sample, vaporLayer);
	Segment cloudSection = layerIntersection(trace.viewRay, sample, cloudLayer);

	vec4 color = vec4(sunlightColor(sample) * sample.hitLight);
	
	float layer1Top = max(vaporSection.far, cloudSection.far);
	float layer2Top = cloudSection.far;
	float layer3Top = cloudSection.near;

	// Top of atmosphere above thin and thick clouds
	float sampleLen = 100.0;

	while(rayDist > layer1Top) {
		sample = getSampleSun(trace, accumulated, rayDist, atmDist, sampleLen);

		float airDensity = clearAirDensity(sample);

		vec4 airColor = clearAirColor(sample, 1.0);

		color = mix(color, airColor, airDensity * airColor.a);

		accumulated = sample.accumulated;
		rayDist -= sampleLen;
	}

	// Thin cloud layer
	sampleLen = 50.0;

	while(rayDist > layer2Top) {
		sample = getSampleSun(trace, accumulated, rayDist, atmDist, sampleLen);
		vaporLayer = thinCloudLayer(sample);

		float airDensity = clearAirDensity(sample);
		float vaporDensity = thinCloudDensity(sample, vaporLayer);

		vec4 airColor = clearAirColor(sample, 1.0);
		vec4 cloudColor = vaporColor(sample, airColor, 1.0);

		color = mix(color, cloudColor, vaporDensity);
		color = mix(color, airColor, airDensity * airColor.a);

		accumulated = sample.accumulated;
		accumulated.vaporShadow = max(
			accumulated.vaporShadow -
			sample.sampleAtmRatio * vaporDensity * 4.0,
			0.0);
		rayDist -= sampleLen;
	}

	// Thick cloud layer
	sampleLen = 25.0;
	float sct = sampleLen / cloudThickness;

	while(rayDist > layer3Top) {
		sample = getSampleSun(trace, accumulated, rayDist, atmDist, sampleLen);
		vaporLayer = thinCloudLayer(sample);
		cloudLayer = thickCloudLayer(sample);
		cloudSection = layerIntersection(trace.viewRay, sample, cloudLayer);

		float shadow = cloudShadowIn(sample, cloudLayer);
		float airDensity = clearAirDensity(sample);
		float vaporDensity = thinCloudDensity(sample, vaporLayer);
		float cloudDensity = thickCloudDensity(sample, cloudLayer);

		vec4 airColor = clearAirColor(sample, shadow);
		vec4 cloudColor = vaporColor(sample, airColor, shadow);

		color = mix(color, cloudColor, vaporDensity);
		color = mix(color, cloudColor, cloudDensity);
		color = mix(color, airColor, airDensity * airColor.a);

		accumulated = sample.accumulated;
		accumulated.vaporShadow = max(
			accumulated.vaporShadow - sct * cloudDensity * 256.0,
			0.0);
		rayDist -= sampleLen;
	}

	// Layer below the thick clouds
	sampleLen = 50.0;

	while(rayDist > 0.0) {
		sample = getSampleSun(trace, accumulated, rayDist, atmDist, sampleLen);
		vaporLayer = thinCloudLayer(sample);

		float shadow = cloudShadowOut(sample, cloudLayer);
		float airDensity = clearAirDensity(sample);
		float vaporDensity = thinCloudDensity(sample, vaporLayer);

		vec4 airColor = clearAirColor(sample, shadow);
		vec4 cloudColor = vaporColor(sample, airColor, shadow);

		color = mix(color, cloudColor, vaporDensity);
		color = mix(color, airColor, airDensity * airColor.a);

		accumulated = sample.accumulated;
		rayDist -= sampleLen;
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
		TraceInfo trace = TraceInfo(viewRay, planet, getAtmosphere());
		fragColor = skyColor(trace);
	}
}

