#version 430 core

layout (location = 0) out vec4 rtNewColor;
layout (location = 1) out vec4 rtIrrad;
layout (location = 2) out vec4 rtIrradRV;
layout (location = 3) out vec3 traceData;
layout (location = 4) out vec3 traceDataRV;
layout (location = 5) out vec4 rtTempRV;

in vec2 TexCoords;

uniform mat4 projection;
uniform mat4 camView;
struct Material{
    //vec3 rho;
    vec3 eta;
    vec3 micro_k;
    vec3 refspec;
    float alpha;
};
uniform Material material;
uniform vec2 iResolution;
uniform vec2 panoSize;
uniform int frameCount;
uniform float min_depth;
uniform float shadowfade;
uniform sampler2D mipmapDepthTex;
uniform sampler2D colorTex;
uniform sampler2D normalTex;
uniform sampler2D worldPosTex;
uniform sampler2D mat_prevCoordTex;
uniform sampler2D panoTex;
uniform sampler2D mipmapedFrameDepthTex;
//uniform sampler2D ambientOcclusion;
uniform sampler2D diffTextureTex;
uniform sampler2D adaptStencilTex;
float PI = 3.141592653;
float maxRayTraceDistance = 5.f;

uint initRand(uint val0, uint val1, uint backoff = 16)
{
	uint v0 = val0, v1 = val1, s0 = 0;

	for (uint n = 0; n < backoff; n++)
	{
		s0 += 0x9e3779b9;
		v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
		v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
	}
	return v0;
}

float nextRand(inout uint s)
{
	s = (1664525u * s + 1013904223u);
	return float(s & 0x00FFFFFF) / float(0x01000000);
}

float FastFMod(float x, float m) {
	return x - (m * floor(x / m));
}
int Intmod(int x, int m) {
	return x - (m * int(floor(1.f * x / m)));
}

float luminance(vec3 color)
{
	return dot(color, vec3(0.299, 0.587, 0.114));
}

vec2 ConcentricSampleDisk(vec2 u) {
	vec2 uOffset = 2.f * u - vec2(1, 1);

	// Handle degeneracy at the origin
	if (uOffset.x == 0 && uOffset.y == 0) return vec2(0.f, 0.f);

	// Apply concentric mapping to point
	float theta, r;
	if (abs(uOffset.x) > abs(uOffset.y)) {
		r = uOffset.x;
		theta = PI/4.f * (uOffset.y / uOffset.x);
	}
	else {
		r = uOffset.y;
		theta = PI / 2.f - PI / 4.f  * (uOffset.x / uOffset.y);
	}
	return r * vec2(cos(theta), sin(theta));
}

vec3 getTracingDirformUV(vec2 uv)
{
    vec2 d = ConcentricSampleDisk(uv);
	float z = sqrt(max(0.001f, 1.f - d.x*d.x - d.y *d.y));
	return vec3(d.x, d.y, z);
}

float CosTheta(vec3 w) { return w.z; }
float Cos2Theta(vec3 w) { return w.z* w.z; }
//float AbsCosTheta(vec3 w) { return abs(w.z); }
float Sin2Theta(vec3 w)
{
	return max(0.f, 1.f - Cos2Theta(w));
}
float SinTheta(vec3 w) { return sqrt(Sin2Theta(w)); }
float TanTheta(vec3 w) { return SinTheta(w) / CosTheta(w); }
//float Tan2Theta(vec3 w) {
//	return Sin2Theta(w) / Cos2Theta(w);
//}
float CosPhi(vec3 w) {
	float sinTheta = SinTheta(w);
	return (sinTheta == 0) ? 1 : clamp(w.x / sinTheta, -1.f, 1.f);
}
float SinPhi(vec3 w) {
	float sinTheta = SinTheta(w);
	return (sinTheta == 0) ? 0 : clamp(w.y / sinTheta, -1.f, 1.f);
}
float Cos2Phi(vec3 w) { return CosPhi(w)* CosPhi(w); }
float Sin2Phi(vec3 w) { return SinPhi(w)* SinPhi(w); }
//float CosDPhi(vec3 wa, vec3 wb) {
//	return clamp(
//		(wa.x * wb.x + wa.y * wb.y) / sqrt((wa.x * wa.x + wa.y * wa.y) *
//		(wb.x * wb.x + wb.y * wb.y)),
//		-1.f, 1.f);
//}
vec3 safe_sqrt(vec3 v)
{
	return vec3(sqrt(max(0.0f, v.x)), sqrt(max(0.0f, v.y)), sqrt(max(0.0f, v.z)));
}

vec3 sampleWh(float r1, float r2, float alpha)
{
	float tan2Theta, phi;
	float logSample = log(1.0f - r1);
	tan2Theta = -alpha * alpha * logSample;
	phi = r2 * 2 * PI;

	// Map sampled Beckmann angles to normal direction _wh_
	float cosTheta = 1.f / sqrt(1.f + tan2Theta);
	float sinTheta = sqrt(max(0.f, 1.f - cosTheta * cosTheta));
	vec3 wh = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

	if (wh.z < 0.f)
		wh = -wh;
	return wh;
}
float lambda_beckmann(vec3 w, float alpha)
{
	float absTanTheta = abs(TanTheta(w));
	// Check for infinity
	if ((2.0f * absTanTheta) == absTanTheta) return 0.f;

	// Compute _alpha_ for direction _w_
	float _alpha = sqrt(Cos2Phi(w) * alpha * alpha + Sin2Phi(w) * alpha * alpha);
	float a = 1.0f / (_alpha * absTanTheta);

	if (a >= 1.6f) return 0.f;

	return (1.f - 1.259f * a + 0.396f * a * a) / (3.535f * a + 2.181f * a * a);
}
float NDF(vec3 wm, float alpha)
{
	float nwh2 = Cos2Theta(wm);
	float alpha2 = pow(alpha, 2.f);
	return exp((nwh2 - 1.f) / (alpha2 * nwh2)) / (pow(nwh2, 2.f) * alpha2 * PI);   //Beckmann-CSpizzichino model
}
float ComputePDF(vec3 wi, vec3 wh, float alpha)
{
	float temp = 1.0f / (1.0f + lambda_beckmann(wi, alpha));
	float pdf = abs(dot(wi, wh)) * NDF(wh, alpha) * temp / max(1.0e-8f, abs(CosTheta(wi)));
	return pdf;
}

float G1Schlick(vec3 w, vec3 n, float alpha) {
	float k = alpha * sqrt(2.f / PI);
	return float(dot(n, w)) / float((dot(n, w) * (1.0 - k) + k));
}
float G1Smith(vec3 w, float alpha2, vec3 n) {
	return float(2.f * dot(n, w)) / float((dot(n, w) + sqrt(alpha2 + (1.f - alpha2) * pow(dot(n, w), 2.f))));
}
vec3 fresnelConductorExact(float cosThetaI, inout vec3 eta, inout vec3 k) {
	/* Modified from "Optics" by K.D. Moeller, University Science Books, 1988 */

	float cosThetaI2 = cosThetaI*cosThetaI,
		sinThetaI2 = 1 - cosThetaI2,
		sinThetaI4 = sinThetaI2*sinThetaI2;

	vec3 temp1 = eta*eta - k*k - vec3(sinThetaI2);
	vec3 a2pb2 = safe_sqrt(temp1*temp1 + k*k*eta*eta * 4);
	vec3 a = safe_sqrt((a2pb2 + temp1) * 0.5f);

	vec3 term1 = a2pb2 + vec3(cosThetaI2),
		term2 = a*(2 * cosThetaI);

	vec3 Rs2 = (term1 - term2) / (term1 + term2);

	vec3 term3 = a2pb2*cosThetaI2 + vec3(sinThetaI4),
		term4 = term2*sinThetaI2;

	vec3 Rp2 = Rs2 * (term3 - term4) / (term3 + term4);

	return 0.5f * (Rp2 + Rs2);
}
vec3 microFacetFs(vec3 n, vec3 wi, vec3 wo, vec3 wh, vec3 eta, vec3 micro_k, float alpha)
{
	float nwh2 = pow(dot(n, wh), 2.f);
	vec3 F0 = fresnelConductorExact(dot(wi, wh), eta, micro_k);
	//vec3 F0 = fresnelConductorExact(1.f, eta, micro_k);
	vec3 F = F0 +(vec3(1.f, 1.f, 1.f) - F0) * pow(1.f - max(0.f, dot(wi, wh)), 5.f);


	float Dterm = NDF(wh, alpha);   //Beckmann-CSpizzichino model

    //float shading = 2.0 * (dot(n, wh) * dot(n, wi)) / dot(wo, wh);
    //float masking = 2.0 * (dot(n, wh) * dot(n, wo)) / dot(wo, wh);
    //float G = min(1.0, min(shading, masking));
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //float alpha2 = pow(alpha, 2.0);
    //float D = alpha2 / (M_PI * pow(1.0 + (alpha2 - 1.0) * nwh2, 2.0));           //Trowbridge-Reitz GGX
	float G;
	//if(schlick == false) 
	float alpha2 = alpha * alpha;
	G = G1Smith(wi, alpha2, n) * G1Smith(wo, alpha2, n);									//approximation of Smith
	//else 								 	
	//G = G1Schlick(wi, n, alpha) * G1Schlick(wo, n, alpha); 							//approximation of Schlick

	return (Dterm * F * G) / (4.f * dot(n, wi) * dot(n, wo));
}
vec3 computeLightRadiance(vec3 n, vec3 wo, vec3 wi, vec3 Li)
{
	vec3 radiance = vec3(0.f);
	vec3 wh = normalize(wi + wo);

	vec3 fs = microFacetFs(n, wi, wo, wh, material.eta, material.micro_k, material.alpha);
	//radiance = Li *  max(dot(n, wi), 0.0) * (fs + fd* albedo) ;
	radiance = Li *  max(dot(n, wi), 0.f) * (fs * material.refspec);
	return radiance;
}

bool amongInterval(int a, int b, int c)
{ 
	if (a >= min(b, c) && a <= max(b, c)) return true;
	return false;
}

bool isIntersectTheta(float x0, float x1, float y0, float y1, float z0, float z1, float theta, float t, inout float tempt,
	vec3 orig, vec3 ray_dir, inout float phi, int lod)
{
	float A, B, C;
	float tantheta = tan(theta);
	A = tantheta * tantheta * y1*y1 - x1*x1 - z1*z1;
	B = 2 * (y0*y1*tantheta*tantheta - x0*x1 - z0*z1);
	C = y0*y0*tantheta*tantheta - x0*x0 - z0*z0;
	float delta = B*B - 4 * A*C;
	if (delta < 0) return false;
	float t1, t2;
	t1 = (-B - sqrt(delta)) / (2.f * A);
	t2 = (-B + sqrt(delta)) / (2.f * A);
	//t = (abs(t1 - t) > abs(t2 - t)) ? t2 : t1;
	if (t1 > t2) {
		float temp = t1;
		t1 = t2;
		t2 = temp;
	}
	if (t1 >= t)
		tempt = t1;
	else if (t2 >= t)
		tempt = t2;
	if (tempt < 0) return false;

	vec3 p = orig + t * ray_dir;
	float plen = length(p);
	vec3 unitp = normalize(p);
	phi = atan(unitp.z, unitp.x);
	phi = clamp(FastFMod(phi, 2.f*PI), 0.0, 2.f*PI);

	if (phi < 0) {
		return false;
	}

	if (theta != theta || phi != phi) return false;

	theta = clamp(FastFMod(theta, PI), 0.0, PI);
	phi = clamp(FastFMod(phi, 2.0*PI), 0.0, 2.0*PI);
	float v = theta / PI;
	float u = phi / (2 * PI);
	float texdepth = textureLod(mipmapDepthTex, vec2(u,v), lod).x;
	return plen >= texdepth;
}

bool amongIntervalPHI(float a, float b, float c, int panow)
{
	bool ans;
	if (abs(c - b) <= panow / 2)
	{
		if (a <= max(b, c) && a >= min(b, c)) ans = true;
		else ans = false;
	}
	else
	{
		if (a < max(b, c) && a >= min(b, c)) ans = false;
		else ans = true;
	}
	return ans;
}

bool
isIntersectPhi(float x0, float x1, float z0, float z1, vec3 orig, vec3 dir, inout float theta, float phi,
	int phi_id, int panow, int lod)
{
	float t;
	if (phi_id == panow / 4 || phi_id == panow * 3 / 4) {
		t = -x0 / x1;
	}
	else {
		float tanphi = tan(phi);
		t = (z0 - x0*tanphi) / (x1*tanphi - z1);
	}
	vec3 p = orig + t*dir;// r.point_at_parameter(t);
	float plen = length(p);
	theta = acos(clamp(normalize(p).y, -1.0f, 1.0f));
	theta = clamp(FastFMod(theta, PI), 0.0, PI);

	if (theta < 0) {
		return false;
	}

	if (theta != theta || phi != phi) return false;

	theta = clamp(FastFMod(theta, PI), 0.0, PI);
	phi = clamp(FastFMod(phi, 2.0*PI), 0.0, 2.0*PI);
	float v = theta / PI;
	float u = phi / (2 * PI);
	float texdepth = textureLod(mipmapDepthTex, vec2(u,v), lod).x;//tex2DLod<float>(tex, u, v, lod);
	return plen >= texdepth;
}

vec3 findNewOrig(vec3 o, vec3 d)
{
	//return o;
	vec3 result;
	float radius = min_depth;
	float A = dot(d, d);
	float B = 2 * dot(o,d);
	float C = dot(o,o) - radius * radius;
	float delta = B * B - 4 * A * C;
	if(delta < 0) return o;
	float t1 = (-B + sqrt(delta))/(2 * A);
	float t2 = (-B - sqrt(delta))/(2 * A);
	float t = max(t1,t2);
	if(t > 0) 
		result = o + t * d;
	else 
		result = o;
	return result;
}

void fastSSRTrace(vec3 _orig, vec3 ray_dir, vec3 normal, inout vec3 li_rad)
{
	vec3 orig = _orig;//findNewOrig(_orig, ray_dir);
    int panow = int(panoSize.x);
    int panoh = int(panoSize.y);

    float x0, x1, y0, y1, z0, z1;
	x0 = orig.x; y0 = orig.y; z0 = orig.z;
	x1 = ray_dir.x; y1 = ray_dir.y; z1 = ray_dir.z;
	float dtheta = PI / panoh;
	float dphi = 2 * PI / panow;

	float theta_end = acos(clamp(y1, -1.f, 1.f));
	float tttt = acos(clamp((x0 * x1 + z0 * z1) / (sqrt(x0 * x0 + z0 * z0)*sqrt(z1*z1 + x1 *x1)), -1.0, 1.0));

	float phi = 0, theta = 0;
	int theta_id = 0, phi_id = 0;
	float tempt;

	if ((theta_end < PI / 4 || theta_end > PI * 3 / 4) || (tttt < 1e-2 || tttt> PI - (1e-2))) {
		float theta_start = acos(clamp(normalize(orig).y, -1.0, 1.0));
		int theta_start_id = int(theta_start / PI*(panoh - 1));
		float theta_end = acos(clamp(y1, -1.0, 1.0));
		int theta_end_id = int(theta_end / PI*(panoh - 1));

		float limit_theta;
		vec3 normal = normalize(cross(orig, ray_dir));
		float xn = normal.x, zn = normal.z;
		float limit_t = (xn*z0 - x0 *zn) / (x1*zn - z1*xn);
		vec3 Q = orig + limit_t * ray_dir;
		limit_theta = acos(clamp(normalize(Q).y, -1.0, 1.0));
		int limit_theta_id = int(limit_theta / PI*(panoh - 1));

		int step, stepsig;
		bool hitflag = false;
		float t = 0; tempt = 0;
		int lod = 0;
		int test_theta_id = 0;
		float test_theta = 0, test_phi = 0;
		if (limit_t > 0) {
			step = limit_theta_id - theta_start_id > 0 ? 1 : -1;
			stepsig = step;
			theta_id = theta_start_id;

			while (!hitflag) {
				step = int(stepsig * pow(2, lod));
				test_theta_id = theta_id + step;
				test_theta = test_theta_id	*dtheta;
				if (!amongInterval(test_theta_id, theta_start_id, limit_theta_id)) {
					if (lod>0) lod--;
					else break;
				}
				else {
					if (isIntersectTheta(x0, x1, y0, y1, z0, z1, test_theta, t, tempt, orig, ray_dir, test_phi, lod)) {
						if (lod <= 0) {
							lod--;
							hitflag = true;
							break;
						}
						int id1 = theta_id + int(stepsig * pow(2, lod - 1));
						float id1theta = id1 * dtheta;
						float id2theta = test_theta;
						float id1phi, id2phi;
						float tt;
						if (isIntersectTheta(x0, x1, y0, y1, z0, z1, id1theta, t, tt, orig, ray_dir, id1phi, lod - 1)) {
							lod--;
						}
						else if (isIntersectTheta(x0, x1, y0, y1, z0, z1, id2theta, t, tempt, orig, ray_dir, id2phi, lod - 1)) {
							theta_id = id1;
							t = tt;
							lod--;
						}
						else {
							theta_id = test_theta_id;
							t = tempt;
						}

					}
					else {
						theta_id = test_theta_id;
						t = tempt;
						lod++;
					}
				}
			}
			phi = test_phi;
			theta = test_theta;

			if (!hitflag) {
				lod = 0;
				step = theta_end_id - limit_theta_id > 0 ? 1 : -1;
				stepsig = step;
				theta_id = limit_theta_id;

				while (!hitflag) {
					step = int(stepsig * pow(2, lod));
					test_theta_id = theta_id + step;
					test_theta = test_theta_id	*dtheta;
					if (!amongInterval(test_theta_id, limit_theta_id, theta_end_id)) {
						if (lod > 0) lod--;
						else break;
					}
					else {
						if (isIntersectTheta(x0, x1, y0, y1, z0, z1, test_theta, t, tempt, orig, ray_dir, test_phi, lod)) {
							if (lod <= 0) {
								lod--;
								hitflag = true;
								break;
							}
							int id1 = theta_id + int(stepsig * pow(2, lod - 1));
							float id1theta = id1 * dtheta;
							float id2theta = test_theta;
							float id1phi, id2phi;
							float tt;
							if (isIntersectTheta(x0, x1, y0, y1, z0, z1, id1theta, t, tt, orig, ray_dir, id1phi, lod - 1)) {
								lod--;
							}
							else if (isIntersectTheta(x0, x1, y0, y1, z0, z1, id2theta, t, tempt, orig, ray_dir, id2phi, lod - 1)) {
								theta_id = id1;
								t = tt;
								lod--;
							}
							else {
								theta_id = test_theta_id;
								t = tempt;
							}

						}
						else {
							theta_id = test_theta_id;
							t = tempt;
							lod++;
						}
					}
				}

				phi = test_phi;
				theta = test_theta;
			}
		}
		else
		{
			step = theta_end_id - theta_start_id > 0 ? 1 : -1;
			stepsig = step;
			theta_id = theta_start_id;
			lod = 0;
			while (!hitflag) {
				step = int(stepsig * pow(2, lod));
				test_theta_id = theta_id + step;
				test_theta = test_theta_id	*dtheta;
				if (!amongInterval(test_theta_id, theta_start_id, theta_end_id)) {
					if (lod > 0) lod--;
					else break;
				}
				else {
					if (isIntersectTheta(x0, x1, y0, y1, z0, z1, test_theta, t, tempt, orig, ray_dir, test_phi, lod)) {
						if (lod <= 0) {
							lod--;
							hitflag = true;
							break;
						}
						int id1 = theta_id + int(stepsig * pow(2, lod - 1));
						float id1theta = id1 * dtheta;
						float id2theta = test_theta;
						float id1phi, id2phi;
						float tt;
						if (isIntersectTheta(x0, x1, y0, y1, z0, z1, id1theta, t, tt, orig, ray_dir, id1phi, lod - 1)) {
							lod--;
						}
						else if (isIntersectTheta(x0, x1, y0, y1, z0, z1, id2theta, t, tempt, orig, ray_dir, id2phi, lod - 1)) {
							theta_id = id1;
							t = tt;
							lod--;
						}
						else {
							theta_id = test_theta_id;
							t = tempt;
						}

					}
					else {
						theta_id = test_theta_id;
						t = tempt;
						lod++;
					}
				}
			}

			phi = test_phi;
			theta = test_theta;
		}
	}
	else {
		float phi_start = atan(z0, x0);
		phi_start = clamp(FastFMod(phi_start, 2.0*PI), 0.0, 2.0*PI);
		float phi_end = atan(z1, x1);
		phi_end = clamp(FastFMod(phi_end, 2.0*PI), 0.0, 2.0*PI);

		int phi_start_id = int(phi_start / (2 * PI) * (panow - 1));
		int phi_end_id = int(phi_end / (2 * PI) * (panow - 1));

		int step, stepsig;
		bool hitflag = false;
		int lod = 0;

		if (abs(phi_end_id - phi_start_id) <= panow / 2) step = (phi_end_id - phi_start_id) > 0 ? 1 : -1;
		else step = (phi_end_id - phi_start_id) > 0 ? -1 : 1;

		stepsig = step;
		phi_id = phi_start_id;

		int test_phi_id;
		float test_theta, test_phi;

		while (!hitflag) {
			step = int(stepsig * pow(2, lod));
			test_phi_id = phi_id + step;
			test_phi_id = clamp(Intmod(test_phi_id, panow), 0, panow);
			test_phi = test_phi_id	*dphi;
			if (!amongIntervalPHI(test_phi_id, phi_start_id, phi_end_id, panow)) {
				if (lod > 0) lod--;
				else break;
			}
			else {
				if (isIntersectPhi(x0, x1, z0, z1, orig, ray_dir, test_theta, test_phi, test_phi_id, panow, lod)) {
					if (lod <= 0) {
						lod--;
						hitflag = true;
						break;
					}
					int id1 = phi_id + int(stepsig * pow(2, lod - 1));
					id1 = clamp(Intmod(id1, panow), 0, panow);
					float id1phi = id1 * dphi;
					float id2phi = test_phi;
					float id1theta, id2theta;
					if (isIntersectPhi(x0, x1, z0, z1, orig, ray_dir, id1theta, id1phi, id1, panow, lod - 1)) {
						lod--;
					}
					else if (isIntersectPhi(x0, x1, z0, z1, orig, ray_dir, id2theta, id2phi, test_phi_id, panow, lod - 1)) {
						phi_id = id1;
						lod--;
					}
					else {
						phi_id = test_phi_id;
					}

				}
				else {
					phi_id = test_phi_id;
					lod++;
				}
			}
		}

		phi = test_phi;
		theta = test_theta;

	}

	if (theta != theta || phi != phi) {
		li_rad = vec3(0);
	}
	else {
		theta = clamp(FastFMod(theta, PI), 0.0, PI);
		phi = clamp(FastFMod(phi, 2.0*PI), 0.0, 2.0*PI);
		li_rad = texture(panoTex, vec2(phi / (2*PI), theta / PI)).rgb;
	}
}

float distanceSquared(vec2 A, vec2 B) {
	A -= B;
	return dot(A, A);
}

bool traceGbuffer(vec3 orig, vec3 dir, vec3 normal, inout vec3 li_rad, inout vec3 hitnormal, 
    inout vec3 hitpos, inout int hitmat, inout vec2 out_hitpixel)
{
	vec3 csOrigin = (camView * vec4(orig, 1.f)).rgb;
	vec3 csDirection = (camView * vec4(dir, 0.f)).rgb;

	float rayLength = ((csOrigin.z + csDirection.z * maxRayTraceDistance) > -0.01f) ?
		(-0.01f - csOrigin.z) / csDirection.z : maxRayTraceDistance;

	vec3 csEndPoint = csDirection * rayLength + csOrigin;

    vec4 H0 = projection * vec4(csOrigin, 1.f);
    vec4 H1 = projection * vec4(csEndPoint, 1.f);
    
	float k0 = 1.0 / H0.w;
    float k1 = 1.0 / H1.w;

    vec3 Q0 = csOrigin * k0;
    vec3 Q1 = csEndPoint * k1;

    vec2 P0 = H0.xy * k0;
    vec2 P1 = H1.xy * k1;
	P0 = (1 + P0) * 0.5 * iResolution + 0.5;
	P1 = (1 + P1) * 0.5 * iResolution + 0.5;

    vec2 hitPixel = vec2(-1.f,-1.f);

    P1 += vec2((distanceSquared(P0,P1) < 0.0001) ? 0.01: 0.0);
    vec2 delta = P1 - P0;

    bool permute = false;
    if(abs(delta.x)<abs(delta.y)){
        permute = true;
        delta = delta.yx;
        P0 = P0.yx;
        P1 = P1.yx;
    }

	float stepDir = sign(delta.x);
    float invdx = stepDir/delta.x;
    vec2 dP = vec2(stepDir, invdx * delta.y);

    vec3 dQ = (Q1 - Q0) *invdx;
    float dk = (k1 - k0) * invdx;

    vec3 Q = Q0; 
    float k = k0;

    float end = P1.x * stepDir;
	vec2 P = P0;

	float step;
	bool hitflag = false;
	int lod = 0;
	vec2 testP;
	vec3 testQ;
	float testk;
	hitPixel = permute ? P.yx : P;
	float lasthitmat = texture(mat_prevCoordTex, hitPixel/iResolution).x, currenthitmat;
	float lastSceneDepth = 0;

	while(!hitflag)
	{
		step = pow(2, lod);
		testP = P + step * dP;
		testQ = Q + step * dQ;
		testk = k + step * dk;
		float rayZMax = length((dQ * 0.5 + testQ) / (dk * 0.5 + testk));
		hitPixel = permute ? testP.yx:testP;
		vec2 hitTexCoords = vec2(hitPixel/iResolution);
		float sceneDepth = textureLod(mipmapedFrameDepthTex, hitTexCoords, lod).r;
		currenthitmat = int(texture(mat_prevCoordTex, hitTexCoords).r);
			
		if(hitPixel.x >= iResolution.x || hitPixel.x < 0 || hitPixel.y >= iResolution.y || hitPixel.y < 0 || testP.x * stepDir > end) {
			if(lod > 0) lod--;
			else break;
		}
		else{
			if(rayZMax > sceneDepth)
			{
				if(lod <= 0){
					if(currenthitmat == 0){
						return false;				//intersect with panobackgroud
					}
					else{
						// vec2 lastP = testP - dP;
						// vec3 lastQ = testQ - dQ;
						// float lastk = testk - dk;
						// vec2 lastHitPixel = permute ? lastP.yx : lastP;
						// vec2 lastHitTexCoords = vec2(lastHitPixel/iResolution);
						// float lastsDepth = texture(mipmapedFrameDepthTex, lastHitTexCoords, lod).r;
						// float lastrayZ = length((dQ * 0.5 + lastQ) / (dk * 0.5 + lastk));
						// float deltaRayZ = abs(lastrayZ - rayZMax);
						// float deltasDep = abs(lastsDepth - sceneDepth);
						// if(deltaRayZ)
						//if(abs(rayZMax - sceneDepth)>0.01) return false;
						if(lasthitmat != currenthitmat && rayZMax - sceneDepth >=0.001 && 
						rayZMax - sceneDepth < 0.05 || abs(rayZMax - sceneDepth)<0.001){// 
							hitflag = true;
							//to do
							hitmat = int(currenthitmat);
							li_rad = texture(colorTex, hitTexCoords).rgb;
							hitpos = texture(worldPosTex, hitTexCoords).rgb;
							hitnormal = texture(normalTex, hitTexCoords).rgb;
							out_hitpixel = hitPixel;
							return true;
						}
						else{
							// P = testP;
							// Q = testQ;
							// k = testk;
							// lasthitmat = currenthitmat;
							return false;
						}
					}
				}
				else{
					step = pow(2, lod - 1);
					vec2 tP1 = P + step*dP;
					vec3 tQ1 = Q + step*dQ;
					float tk1 = k + step*dk;
					vec2 hitP1 = permute?tP1.yx:tP1;
					vec2 hitCoords1 = vec2(hitP1/iResolution);
					float sDepth1 = textureLod(mipmapedFrameDepthTex, hitCoords1, lod - 1).r;
					float rayDepth1 = length((dQ * 0.5 + tQ1) / (dk * 0.5 + tk1));
					float sDepth2 = textureLod(mipmapedFrameDepthTex, hitTexCoords, lod - 1).r;
					if(rayDepth1 > sDepth1){
						lod--;
					}
					else if(rayZMax > sDepth2){
						P = tP1;
						Q = tQ1;
						k = tk1;
						float hitmat1 = int(texture(mat_prevCoordTex, hitCoords1).r);
						lasthitmat = hitmat1;
						lod--;
					}
					else{
						P = testP;
						Q = testQ;
						k = testk;
						lasthitmat = currenthitmat;
					}
				}
			}
			else{
				P = testP;
				Q = testQ;
				k = testk;
				lasthitmat = currenthitmat;
				lod++;
			}
		}
	}
	return false;
}


// bool traceGbuffer(vec3 orig, vec3 dir, vec3 normal, inout vec3 li_rad, inout vec3 hitnormal, 
//     inout vec3 hitpos, inout int hitmat, inout vec2 out_hitpixel)
// {
// 	vec3 csOrigin = (camView * vec4(orig, 1.f)).rgb;
// 	vec3 csDirection = (camView * vec4(dir, 0.f)).rgb;

// 		float rayLength = ((csOrigin.z + csDirection.z * maxRayTraceDistance) > -0.01f) ?
// 		(-0.01f - csOrigin.z) / csDirection.z : maxRayTraceDistance;

// 	vec3 csEndPoint = csDirection * rayLength + csOrigin;

//     vec4 H0 = projection * vec4(csOrigin, 1.f);
//     vec4 H1 = projection * vec4(csEndPoint, 1.f);
    
// 	float k0 = 1.0 / H0.w;
//     float k1 = 1.0 / H1.w;

//     vec3 Q0 = csOrigin * k0;
//     vec3 Q1 = csEndPoint * k1;

//     vec2 P0 = H0.xy * k0;
//     vec2 P1 = H0.xy * k1;
// 	P0 = (1 + P0) * 0.5 * iResolution - 0.5;
// 	P1 = (1 + P1) * 0.5 * iResolution - 0.5;

//     vec2 hitPixel = vec2(-1.f,-1.f);

//     P1 += vec2((distanceSquared(P0,P1) < 0.0001) ? 0.01: 0.0);
//     vec2 delta = P1 - P0;

//     bool permute = false;
//     if(abs(delta.x)<abs(delta.y)){
//         permute = true;
//         delta = delta.yx;
//         P0 = P0.yx;
//         P1 = P1.yx;
//     }

// 	float stepDir = sign(delta.x);
//     float invdx = stepDir/delta.x;
//     vec2 dP = vec2(stepDir, invdx * delta.y);

//     vec3 dQ = (Q1 - Q0) *invdx;
//     float dk = (k1 - k0) * invdx;

//     vec3 Q = Q0; 
//     float k = k0;

//     float end = P1.x * stepDir;
// 	vec2 P = P0;

// 	float step;
// 	bool hitflag = false;
// 	int lod = 0;
// 	vec2 testP;
// 	vec3 testQ;
// 	float testk;
// 	hitPixel = permute ? P.yx : P;
// 	float lasthitmat = texture(mat_prevCoordTex, hitPixel/iResolution).x, currenthitmat;

// 	while(!hitflag)
// 	{
// 		step = pow(2, lod);
// 		testP = P + step * dP;
// 		testQ = Q + step * dQ;
// 		testk = k + step * dk;
// 		float rayZMax = length((dQ * 0.5 + testQ) / (dk * 0.5 + testk));
// 		hitPixel = permute ? testP.yx:testP;
// 		vec2 hitTexCoords = vec2(hitPixel/iResolution);
// 		float sceneDepth = texture(mipmapedFrameDepthTex, hitTexCoords, lod).r;
// 		currenthitmat = int(texture(mat_prevCoordTex, hitTexCoords).r);
			
// 		if(hitPixel.x >= iResolution.x || hitPixel.x < 0 || hitPixel.y >= iResolution.y || hitPixel.y < 0 || testP.x * stepDir > end) {
// 			if(lod > 0) lod--;
// 			else break;
// 		}
// 		else{
// 			if(rayZMax > sceneDepth)
// 			{
// 				if(lod <= 0){
// 					if(currenthitmat == lasthitmat){
// 						lod--;
// 						hitflag = true;
// 						//to do
// 						hitmat = int(currenthitmat);
// 						li_rad = texture(colorTex, hitTexCoords).rgb;
// 						hitpos = texture(worldPosTex, hitTexCoords).rgb;
// 						hitnormal = texture(normalTex, hitTexCoords).rgb;
// 						out_hitpixel = hitPixel;
// 						return true;
// 					}
// 					else{
// 						P = testP;
// 						Q = testQ;
// 						k = testk;
// 						lasthitmat = currenthitmat;
// 						lod++;
// 					}
// 				}
// 				else{
// 					step = pow(2, lod - 1);
// 					vec2 tP1 = P + step*dP;
// 					vec3 tQ1 = Q + step*dQ;
// 					float tk1 = k + step*dk;
// 					vec2 hitP1 = permute?tP1.yx:tP1;
// 					vec2 hitCoords1 = vec2(tP1/iResolution);
// 					float sDepth1 = texture(mipmapedFrameDepthTex, hitCoords1, lod - 1).r;
// 					float hitmat1 = int(texture(mat_prevCoordTex, hitCoords1).r);
// 					float rayDepth1 = length((dQ * 0.5 + tQ1) / (dk * 0.5 + tk1));
// 					float sDepth2 = texture(mipmapedFrameDepthTex, hitTexCoords, lod - 1).r;
// 					if(rayDepth1 > sDepth1){
// 						lod--;
// 					}
// 					else if(rayZMax > sDepth2){
// 						P = tP1;
// 						Q = tQ1;
// 						k = tk1;
// 						lasthitmat = hitmat1;
// 						lod--;
// 					}
// 					else{
// 						P = testP;
// 						Q = testQ;
// 						k = testk;
// 						lasthitmat = currenthitmat;
// 					}
// 				}
// 			}
// 			else{
// 				P = testP;
// 				Q = testQ;
// 				k = testk;
// 				lasthitmat = currenthitmat;
// 				lod++;
// 			}
// 		}
// 	}
// 	return false;
// }

// bool traceGbuffer(vec3 orig, vec3 dir, vec3 normal, inout vec3 li_rad, inout vec3 hitnormal, 
//     inout vec3 hitpos, inout int hitmat, inout vec2 out_hitpixel)
// {
// 	vec3 csOrigin = (camView * vec4(orig, 1.f)).rgb;
// 	vec3 csorig_dir = (camView * vec4(orig + dir, 1.f)).rgb;
// 	vec3 csDirection = normalize(csorig_dir - csOrigin);

// 	float rayLength = ((csOrigin.z + csDirection.z * maxRayTraceDistance) > -0.01f) ?
// 		(-0.01f - csOrigin.z) / csDirection.z : maxRayTraceDistance;

// 	vec3 csEndPoint = csDirection * rayLength + csOrigin;

//     vec4 H0 = projection * vec4(csOrigin, 1.f);
//     vec4 H1 = projection * vec4(csEndPoint, 1.f);
    
// 	float k0 = 1.0 / H0.w;
//     float k1 = 1.0 / H1.w;

//     vec3 Q0 = csOrigin * k0;
//     vec3 Q1 = csEndPoint * k1;

//     vec2 P0 = H0.xy * k0;
//     vec2 P1 = H0.xy * k1;
// 	P0 = (1 + P0) * 0.5 * iResolution - 0.5;
// 	P1 = (1 + P1) * 0.5 * iResolution - 0.5;

//     vec2 hitPixel = vec2(-1.f,-1.f);

//     P1 += vec2((distanceSquared(P0,P1) < 0.0001) ? 0.01: 0.0);
//     vec2 delta = P1 - P0;

//     bool permute = false;
// 	float maxSteps = iResolution.x;
//     if(abs(delta.x)<abs(delta.y)){
//         permute = true;
//         delta = delta.yx;
//         P0 = P0.yx;
//         P1 = P1.yx;
// 		maxSteps = iResolution.y;
//     }

//     float stepDir = sign(delta.x);
//     float invdx = stepDir/delta.x;
//     vec2 dP = vec2(stepDir, invdx * delta.y);

//     vec3 dQ = (Q1 - Q0) *invdx;
//     float dk = (k1 - k0) * invdx;

//     vec3 Q = Q0; 
//     float k = k0;

//     float prevZMaxEstimate = length(Q / k);
//     float stepCount = 0.f, fstep = 0.0;
//     float rayZMax = prevZMaxEstimate, rayZMin = prevZMaxEstimate;
//     float sceneZMax = rayZMax + 1e4;
//     float end = P1.x * stepDir;

// 	vec2 P = P0;
// 	hitPixel = permute ? P.yx : P;
// 	int lasthitmat = int(texture(mat_prevCoordTex, hitPixel/iResolution).x), currenthitmat;
// 	float lastSceneDepth = prevZMaxEstimate;
// 	float addflag = 2;

// 	vec2 tempP;float tempk, tempStepCount; vec3 tempQ;
// 	while((P.x * stepDir)<=end &&
// 		(stepCount < maxSteps) || fstep > 0)
// 	{
// 		float deltaStep = pow(2, fstep);
// 		tempStepCount =  deltaStep; //stepCount +
// 		tempP = P + deltaStep *dP;
// 		tempQ = Q + deltaStep *dQ;
// 		tempk = k + deltaStep *dk;

// 		hitPixel = permute ? tempP.yx : tempP;
// 		if (hitPixel.x >= iResolution.x || hitPixel.x < 0 || hitPixel.y >= iResolution.y || hitPixel.y < 0) {
// 			if(fstep > 0) {
// 				fstep -=1.f;
// 				addflag = 0;
// 			}
// 			else 
// 				return false;
// 		}
// 		else{
// 			float rayZMin = prevZMaxEstimate;
// 			float rayZMax = length((dQ * 0.5 + tempQ) / (dk * 0.5 + tempk));
// 			//float thickness = rayZMax - rayZMin;

// 			vec2 hitTexCoords = vec2(hitPixel/iResolution);
// 			float sceneDepth = texture(mipmapedFrameDepthTex, hitTexCoords, fstep).r;
// 			currenthitmat = int(texture(mat_prevCoordTex, hitTexCoords).r);
// 			if (rayZMin <= lastSceneDepth && lasthitmat == currenthitmat && rayZMax >= sceneDepth) {//
// 				if(fstep <= 0){
// 					hitmat = currenthitmat;
// 					li_rad = texture(colorTex, hitTexCoords).rgb;
// 					hitpos = texture(worldPosTex, hitTexCoords).rgb;
// 					hitnormal = texture(normalTex, hitTexCoords).rgb;
// 					out_hitpixel = hitPixel;
// 					return true;
// 				}
// 				else{
// 					fstep -=1.f;
// 					addflag = 0;
// 				}
// 			}
// 			else{
// 				stepCount += tempStepCount;
// 				P = tempP;
// 				Q = tempQ;
// 				k = tempk;
// 				if(addflag == 2) fstep += 1;
// 				else{
// 					addflag += 1;
// 				}
// 				lasthitmat = currenthitmat;
// 				lastSceneDepth = sceneDepth;
// 				prevZMaxEstimate = rayZMax;
// 			}
// 		}
// 	}
// 	return false;
// }

// bool traceGbuffer(vec3 orig, vec3 dir, vec3 normal, inout vec3 li_rad, inout vec3 hitnormal, inout vec3 hitpos, inout float hitmat, inout vec2 out_hitpixel)
// {
// 	vec3 csOrigin = (camView * vec4(orig, 1.f)).rgb;
// 	//vec3 csorig_dir = (camView * vec4(orig + dir, 1.f)).rgb;
// 	//vec3 csDirection = normalize(csorig_dir - csOrigin);
// 	vec3 csDirection = (camView * vec4(dir, 0)).rgb;


// 	float rayLength = ((csOrigin.z + csDirection.z * maxRayTraceDistance) > -0.01f) ?
// 		(-0.01f - csOrigin.z) / csDirection.z : maxRayTraceDistance;

// 	vec3 csEndPoint = csDirection * rayLength + csOrigin;

// 	vec2 hitPixel = vec2(-1.f, -1.f);
// 	out_hitpixel = hitPixel;

// 	vec4 H0 = projection * vec4(csOrigin, 1.f);
// 	vec4 H1 = projection * vec4(csEndPoint, 1.f);

// 	float k0 = 1.0 / H0.w, k1 = 1.0 / H1.w;

// 	vec3 Q0 = csOrigin * k0, Q1 = csEndPoint * k1;

// 	vec2 P0 = H0.xy * k0, P1 = H1.xy * k1;
// 	P0 = (1 + P0) * 0.5 * iResolution - 0.5;
// 	P1 = (1 + P1) * 0.5 * iResolution - 0.5;


// 	P1 += vec2((distanceSquared(P0, P1) < 0.0001) ? 0.01 : 0.0);
// 	vec2 delta = P1 - P0;

// 	bool permute = false;
// 	if (abs(delta.x) < abs(delta.y)) {
// 		// This is a more-vertical line
// 		permute = true;
// 		delta = delta.yx; P0 = P0.yx; P1 = P1.yx;
// 	}

// 	float stepDir = sign(delta.x);
// 	float invdx = stepDir / delta.x;

// 	vec3  dQ = (Q1 - Q0) * invdx;
// 	float dk = (k1 - k0) * invdx;
// 	vec2  dP = vec2(stepDir, delta.y * invdx);

// 	vec3 Q = Q0;
// 	float  k = k0, stepCount = 0.0;
// 	float prevZMaxEstimate = length(Q / k);
// 	float  end = P1.x * stepDir;
// 	vec2 P = P0;
// 	float lasthitmat = 0, currenthitmat;
// 	float lastSceneDepth = 0;
// 	while (((P.x * stepDir) <= end) && (stepCount < 50))//iResolution.x
// 	{
// 		P += dP, Q += dQ, k += dk, stepCount += 1.0;
// 		hitPixel = permute ? P.yx : P;
// 		if (hitPixel.x >= iResolution.x || hitPixel.x < 0 || hitPixel.y >= iResolution.y || hitPixel.y < 0) break;

// 		float rayZMin = prevZMaxEstimate;
// 		float rayZMax = length((dQ * 0.5 + Q) / (dk * 0.5 + k));
// 		prevZMaxEstimate = rayZMax;
// 		//if (rayZMin > rayZMax) { swap(rayZMin, rayZMax); }
// 		float thickness = rayZMax - rayZMin;

//         vec2 hitTexCoords = vec2(hitPixel/iResolution);
// 		float sceneDepth = length(texture(worldPosTex, hitTexCoords).rgb);
// 		currenthitmat = int(texture(mat_prevCoordTex, hitTexCoords).x);
// 		if (rayZMin <= lastSceneDepth && lasthitmat == currenthitmat && rayZMax >= sceneDepth) {
// 			hitmat = currenthitmat;
// 			li_rad = texture(colorTex, hitTexCoords).rgb;
// 			hitpos = texture(worldPosTex, hitTexCoords).rgb;
// 			hitnormal = texture(normalTex, hitTexCoords).rgb;
// 			out_hitpixel = hitPixel;
// 			return true;
// 		}
// 		lasthitmat = currenthitmat;
// 		lastSceneDepth = sceneDepth;
// 	}
// 	return false;
// }


bool sampleBRDF(float u, float v, vec3 world_wo, vec3 right, vec3 up, vec3 n, vec3 pos, inout float pdf, inout vec3 orig, 
    inout vec3 dir, inout vec3 li_rad, inout vec3 hitnormal, inout vec3 hitpos, inout int hitmat, bool isFirstBounce)
{
	li_rad = vec3(0.f);
	vec3 wh = sampleWh(u, v, material.alpha);

	vec3 wo = normalize(vec3(dot(right, world_wo), dot(up, world_wo), dot(n, world_wo)));

	vec3 wi = normalize(reflect(-wo, wh));
	if (wi.z * wo.z < 0.0) {
		return false;
	}

	pdf = ComputePDF(wi, wh, material.alpha);
	pdf /= (4.f *  dot(wo, wh));

	vec3 world_wi = normalize(right *wi.x + up*wi.y + n*wi.z);

	orig = pos + 1e-4 * n;
	dir = world_wi;

	vec2 hitpixel;
	if (isFirstBounce) {
		// state = traceGbuffer2(orig, dir, n, li_rad, hitnormal, hitpos, hitmat, hitpixel);
		// if (state == 0)
		// {
		// 	if (hitmat == 0) {
        //         li_rad = texture(newColorTex, vec2(hitpixel / iResolution)).rgb;
		// 	}
		// }
		// if (state == 1) {
		// 	if (!fastSSRTrace2(orig, dir, n, li_rad))
		// 	{
        //         li_rad = texture(newColorTex, vec2(hitpixel / iResolution)).rgb;
		// 	}
		// 	hitmat = 0;
		// }
		// if (state == 2) {
		// 	fastSSRTrace(orig, dir, n, li_rad);
		// 	hitmat = 0;
		// }
		// li_rad = computeLightRadiance(vec3(0, 0, 1), wo, wi, li_rad) / pdf;
	}
	else
	{
		fastSSRTrace(orig, dir, n, li_rad);
		li_rad = computeLightRadiance(vec3(0, 0, 1), wo, wi, li_rad) / pdf;
	}
	return true;
}


vec3 iterateScatter(vec3 dir, vec3 hitpos, vec3 hitnormal, float hitmat, float u, float v, vec3 rho)
{
	vec3 result = vec3(0);
	if (hitmat == 1) {
		float costheta, pdf;
		vec3 nex_orig, nex_dir;
		vec3 right = (1.f - abs(hitnormal.y) < 1e-3) ? vec3(1, 0, 0) : normalize(cross(vec3(0, 1, 0), hitnormal));
		vec3 up = normalize(cross(hitnormal, right));

		vec3 localdir = getTracingDirformUV(vec2(u, v));;

		nex_dir = normalize(right *localdir.x + up*localdir.y + hitnormal*localdir.z);
		nex_orig = hitpos + hitnormal*1e-4;
		costheta = localdir.z;
		pdf = costheta / PI;

		int nex_hitmat;
		vec3 nex_hitnormal, nex_hitpos, li_rad;
		fastSSRTrace(nex_orig, nex_dir, hitnormal, li_rad);
		li_rad *= (rho/PI);//(material.rho / PI);
		result = li_rad * PI;//costheta / pdf;
	}
	if (hitmat == 2) {
		float pdf;
		vec3 nex_orig, nex_dir;
		vec3 right = (1.f - abs(hitnormal.y) < 1e-3) ? vec3(1, 0, 0) : normalize(cross(vec3(0, 1, 0), hitnormal));
		vec3 up = normalize(cross(hitnormal, right));

		int nex_hitmat;
		vec3 nex_hitnormal, nex_hitpos, li_rad;
		if (sampleBRDF(u, v, -dir, right, up, hitnormal, hitpos, pdf, nex_orig, nex_dir, li_rad, nex_hitnormal, nex_hitpos, nex_hitmat,
			false))
		{
			result = li_rad;
		}
	}

	return result;
}


void main()
{
    float hitmat = texture(mat_prevCoordTex, TexCoords).x;
    if(hitmat == 0){
        vec2 iPos = TexCoords * (iResolution - vec2(1,1));
        int offset = int(iPos.x + iPos.y * iResolution.x);
        vec3 pixelpos = texture(worldPosTex, TexCoords).rgb;
        vec3 normal = texture(normalTex, TexCoords).rgb;
        vec3 ray_direction = normalize(pixelpos);

        vec3 irrad = vec3(1e-4), irradRV = vec3(1e-4), tempirradRV = vec3(1e-4);
        vec3 right = (1.f - abs(normal.y) < 1e-3)?vec3(1,0,0):normalize(cross(vec3(0,1,0), normal));
        vec3 up = normalize(cross(normal, right));

        uint seed = initRand(offset, frameCount, 16);
        vec2 uv;
        uv.x = nextRand(seed);
        uv.y = nextRand(seed);
        vec3 localDir = getTracingDirformUV(uv);

        vec3 li_rad = vec3(0);
		float pdf, costheta, dis;
		vec2 hitpixel;

        vec3 ray_dir = normalize(right *localDir.x + up*localDir.y + normal*localDir.z);
        vec3 ray_orig = pixelpos + normal*1e-4;
        costheta = localDir.z;
        pdf = costheta / PI;

        float hitmat2 = -1;
        vec3 hitnormal, hitpos;
        vec3 envtraceRes;
        fastSSRTrace(ray_orig, ray_dir, normal, li_rad);
        envtraceRes = li_rad*PI;//costheta / pdf;
        irrad += envtraceRes;
		
		//irradRV += envtraceRes;
        if (traceGbuffer(ray_orig, ray_dir, normal, li_rad, hitnormal, hitpos, hitmat2, hitpixel)) {
            if (hitmat2 == 0) {
                irradRV = envtraceRes;
				tempirradRV = irradRV;
				traceDataRV = vec3(0,0,0);
            }
            else {
                //next tracing
				//irradRV = vec3(0);
				vec2 hitpixelTex = vec2(hitpixel/iResolution);
				vec3 rho = texture(diffTextureTex, hitpixelTex).rgb;
                vec3 pp = iterateScatter(ray_dir, hitpos, hitnormal, hitmat2, nextRand(seed), nextRand(seed), rho) *PI;//costheta / pdf;
				//if(luminance(pp) < luminance(envtraceRes)){//
					//pp = pp.zyz;
					tempirradRV =  min(1.0f, 7.0 * length(hitpos - ray_orig)) * pp;//min(1.0f, pow(1.0 * length(hitpos - ray_orig), shadowfade))*min(1.0f, pow(length(hitpos - ray_orig), shadowfade)) * pp;//length(hitpos - ray_orig) * pp;//7.0 * min(1.0f, shadowfade * length(hitpos - ray_orig))
					traceDataRV = vec3(1,1,1);//pp;//pp/envtraceRes;//
				//}
				//else tempirradRV = pp;
				
				irradRV = pp;// pp;// 10.0 * length(hitpos - ray_orig) * pp;
				
            }
        }
        else
        {
            irradRV = envtraceRes;
			tempirradRV = irradRV;
			traceDataRV = vec3(0,0,0);
        }

		if(irrad.x != irrad.x || irrad.y != irrad.y || irrad.z != irrad.z) irrad = vec3(1);
		if(irradRV.x != irradRV.x || irradRV.y != irradRV.y || irradRV.z != irradRV.z) irradRV = vec3(1);
		if(tempirradRV.x != tempirradRV.x || tempirradRV.y != tempirradRV.y || tempirradRV.z != tempirradRV.z) tempirradRV = vec3(1);
		
		//irradRV *= texture(ambientOcclusion, TexCoords).x;
		//tempirradRV *= texture(ambientOcclusion, TexCoords).x;


		vec3 ratio =  min(vec3(2.f), tempirradRV / (irrad + 1e-4));
		traceData = ratio;
		if(ratio.x !=ratio.x || ratio.y !=ratio.y || ratio.z !=ratio.z) ratio = vec3(1.f);
		// if(ratio.x < 1) ratio.x = pow(ratio.x, 64);
		// if(ratio.y < 1) ratio.y = pow(ratio.y, 64);
		// if(ratio.z < 1) ratio.z = pow(ratio.z, 64);
		//if(traceDataRV.x!=0) traceDataRV = ratio;
		
        rtIrrad = vec4(irrad, 1);
        rtIrradRV = vec4(tempirradRV , 1); //* ratio
		rtTempRV = vec4(irradRV, 1);

        rtNewColor = vec4(texture(colorTex, TexCoords).rgb * ratio, min(ratio, 1.f));
    }
	else rtNewColor = vec4(0,0,0,1);
}