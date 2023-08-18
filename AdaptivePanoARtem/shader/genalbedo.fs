#version 430 core

layout(location = 0) out vec3 albedo;

in vec2 TexCoords;

uniform vec2 panoSize;
uniform sampler2D panoTex;
uniform sampler2D panoPosTex;
uniform sampler2D panoNormalTex;
uniform sampler2D mipmapDepthTex;

float RayPerPixel = 64;
float PI = 3.141592653;

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

void main()
{
	vec3 pixelpos = texture(panoPosTex, TexCoords).bgr;
	vec3 normal = texture(panoNormalTex, TexCoords).bgr;
	vec3 color = texture(panoTex,TexCoords).rgb;
	vec3 irrad = vec3(0);
	vec2 iPos = TexCoords * (panoSize);
    int offset = int(iPos.y + iPos.x * panoSize.y);
	uint seed = initRand(offset, 1, 16);
	vec3 li_rad,envtraceRes;
	vec3 ray_dir,ray_orig;

	vec3 right = (1.f - abs(normal.y) < 1e-3)?vec3(1,0,0):normalize(cross(vec3(0,1,0), normal));
	vec3 up = normalize(cross(normal, right));

	for(float i = 0;i<RayPerPixel;i+=1)
	{
        vec2 uv;
        uv.x = nextRand(seed);
        uv.y = nextRand(seed);
        vec3 localDir = getTracingDirformUV(uv);

		li_rad = vec3(0);
		ray_dir = normalize(right *localDir.x + up*localDir.y + normal*localDir.z);
		ray_orig = pixelpos + normal*1e-4;

		fastSSRTrace(ray_orig, ray_dir, normal, li_rad);
        envtraceRes = li_rad * PI;
        irrad += envtraceRes;
	}

	irrad /= RayPerPixel;
	albedo = color / irrad;
}