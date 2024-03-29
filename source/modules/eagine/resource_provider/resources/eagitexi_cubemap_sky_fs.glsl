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
//------------------------------------------------------------------------------
struct Ray {
	vec3 origin;
	vec3 direction;
};

struct Segment {
	vec3 start;
	vec3 end;
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
float thinCloudSample(Ray sample, Sphere planet, vec2 offset, float scale) {
	vec3 coord = cloudCoord(
		sample.origin,
		planet,
		offset,
		scale,
		planetRadius-atmThickness*0.05,
		planetRadius+atmThickness*0.05);

	return tilingSample(coord.xy+offset*float(int(coord.z*15.0))*0.05)*
		max(1.0-sample.direction.y, 0.0)*
		exp(-coord.z * 2.0);
}
//------------------------------------------------------------------------------
float thinCloudDensity(Ray sample, Sphere planet) {
	return 0.3*
		mix(
			thinCloudSample(sample, planet, vec2(11.1, 63.1), 0.33),
			thinCloudSample(sample, planet, vec2(23.7, 31.5), 0.19),
			0.5)*
		mix(
			thinCloudSample(sample, planet, vec2(93.1, 99.3), 0.07),
			thinCloudSample(sample, planet, vec2(41.7, 31.5), 0.03),
			0.5)*
		mix(
			thinCloudSample(sample, planet, vec2(29.1, 59.1), 0.02),
			thinCloudSample(sample, planet, vec2(71.3, 97.1), 0.01),
			0.5);
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
	int sampleCount = 1000;
	float isc = 1.0 / float(sampleCount);
	float sampleLen = atmHit * isc ;
	for(int s=1; s<=sampleCount; ++s) {
		vapor += sampleLen * thinCloudDensity(
			raySample(viewRay, s*sampleLen),
			planet);
	}
	return pow(vapor * isc, 0.25);
}
//------------------------------------------------------------------------------
Segment cloudIntersection(Ray ray, Sphere planet) {
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
float thickCloudSample(Ray sample, Sphere planet, vec2 offset, float scale) {
	vec3 coord = cloudCoord(
		sample.origin,
		planet,
		offset,
		scale,
		planetRadius-cloudAltitude*0.5,
		planetRadius+cloudAltitude*0.5);

	return tilingSample(coord.xy);
}
//------------------------------------------------------------------------------
float thickCloudDensity(Ray sample, Sphere planet) {
	return 
		thickCloudSample(sample, planet, vec2(0.0), 1.0)*
		thickCloudSample(sample, planet, vec2(0.0), 2.0);
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
vec4 cloudColor(Ray ray, Sphere planet, vec4 color) {
	Segment intersection = cloudIntersection(ray, planet);
	int sampleCount = 1000;
	float isc = 1.0 / float(sampleCount);
	float d = 0.0;
	for(int s = 0; s <= sampleCount; ++s) {
		Ray sampleRay = Ray(
			mix(intersection.end, intersection.start, float(s)*isc),
			sunDirection);
		d += thickCloudDensity(sampleRay, planet);
	}
	return vec4(vec3(0.7), d * isc);
}
//------------------------------------------------------------------------------
vec3 planetCenter() {
	return vec3(0.0, -planetRadius-aboveGround, 0.0);
}
//------------------------------------------------------------------------------
vec4 skyColor(Ray viewRay, Sphere planet, Sphere atmosphere) {
	vec4 air = clearAirColor(viewRay, planet, atmosphere);
	vec4 cloud = cloudColor(viewRay, planet, air);

	return vec4(mix(air.rgb, cloud.rgb, cloud.a), air.a);
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

