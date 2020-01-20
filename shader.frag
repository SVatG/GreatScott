#version 420

out vec4 f;
in vec4 gl_Color;
layout(size4x32) coherent uniform image3D g;
uniform sampler3D k;
layout(binding=1) uniform sampler3D h;

vec4 transfer(float inf, vec4 base) {
	float val = inf > 0.02f ? inf < 0.17 ? 0.2f + inf : 0.0f : 0.0f;
	float val2 = inf > 0.2 && inf < 0.4 ? 0.05 + inf : 0.0f;
	return base * val + val2;
}

void main() {
	const vec2 params[9] = vec2[9](
		vec2(35, 64),
		vec2(35, 60.9),
		vec2(76.3, 60.9),
		vec2(41.3, 60.9),
		vec2(62, 60.9),
		vec2(22, 60.9),
		vec2(22, 51),
		vec2(14, 51),
		vec2(14, 45)
	);
	float t=gl_Color.x*128.0f*128.0f*4.0f;
	float td=gl_Color.y*128.0f*128.0f*28.0f;
	int paramId = int(gl_Color.z*2048.0f);
	int paramId2 = (paramId/32)%9;
	vec2 o = vec2(1.0f,0.0f)/128.0f;
	for(int j = 0; j < 6; j++) {
		int i = j % 3;
		int idx = int(gl_FragCoord.x+gl_FragCoord.y*1280.0)*3+i;
		if(idx < 128*128*128) {
			ivec3 pi = ivec3((idx/(128*128)), (idx/128)%128, idx%128);
			vec3 pd = vec3(pi)/128.0f+0.5/128.0f;
			vec4 lap = -texture(k,pd) * 6.0 +
				texture(k,pd+o.xyy) + 
				texture(k,pd-o.xyy) +
				texture(k,pd+o.yxy) + 
				texture(k,pd-o.yxy) +
				texture(k,pd+o.yyx) + 
				texture(k,pd-o.yyx);
			vec4 cen = texture(k, pd);
			vec2 help = vec2(cen.x*cen.y*cen.y, cen.z*cen.w*cen.w);
			vec4 rd = vec4(
				0.082f * lap.x - help.x + params[paramId2].x/1000.0f * (1.0f - cen.x), 
				0.041f * lap.y + help.x - (params[paramId2].x + params[paramId2].y)/1000.0f * cen.y,
				0.082f * lap.z - help.y + params[8-paramId2].x/1000.0f * (1.0f - cen.z), 
				0.041f * lap.w + help.y - (params[8-paramId2].x + params[8-paramId2].y)/1000.0f * cen.w
			);
			cen = cen + rd * td;
			imageStore(g, pi, cen);
		}
	}

	if(gl_FragCoord.y<128 && gl_Color.w >= 0.99f && paramId >= 89 && (paramId - 89) % 2 == 0) {
		int cx = int(gl_FragCoord.x) / 10;
		int cz = int(gl_FragCoord.x) % 10;
		vec3 c = vec3(cx,gl_FragCoord.y,(paramId - 89)/4)/128.0f;
		vec4 v = texture(k, c);
		int randn = int(t*1000.0f);
		if(paramId < 117) {
			v.y = texture(h, c).x;
			v.w *= 1.0f - v.y;
		}
		else {
			v.w = texture(h, c).x;
		}
		if((paramId - 89) % 4 == 0 || paramId >= 117) {
			imageStore(g, ivec3((cz + randn)%128,(int(gl_FragCoord.y) + (randn * 311)) % 128,127 - cx), v);
		}
	}

	vec2 v=-1.+gl_FragCoord.xy/vec2(640.0,360.);
    vec3 r=normalize(vec3(v.x,v.y,1.0));
	float forward_mult = paramId < 49 ? 0 : paramId < 57 ? -0.7f : 0.0f;
	float sideward_mult = paramId < 49 ? 1.0f : paramId < 57 ? 0.0f : 1.0f;
	vec3 p=vec3(0.5+t*forward_mult,0.5+t*0.1*sideward_mult,0.5+t*0.05*sideward_mult);

	float angle = paramId < 25 ? t * 0.05 : paramId < 41 ? -t*0.05 : paramId < 49 ? 1.57 : 3.14;
	if(paramId > 56 && paramId % 2 == 0 && paramId < 73) {
		angle = float(paramId - 57)/3.0f + angle;
	}
	mat3 rot = mat3(sin(angle), 0.0, cos(angle), 0.0, 1.0, 0.0, cos(angle), 0.0, -sin(angle));
	r *= rot;
	
    float weight = 0.0f;
	vec4 c = vec4(0.0);
	for(int i=0; i < 300; i++) {
		p+=r/256.0;
		vec4 v = texture(k,p);
		float transmult = paramId < 65 ? 1.0f : (1.0f + gl_Color.w * 3.0f);
		c+=(transfer(v.y*2.5f, vec4(11, 167, 167, 0))*transmult+transfer(v.w*2.5f, vec4(219, 52, 36, 0))/transmult) * weight * (pow(1.0f + abs(mod(p.x + t*0.3f, 1.0f)-0.5f),6.0f) / 4.0f) * 0.01;
        weight += (1.0/256.0f);
	}

	// Background: A grid
	vec4 colc = c / 6.0f;
	float colv = min(length(colc), 1.0);
	float gridc = (1.0f - colv) * pow(abs( (mod(p.x, 0.05f) - 0.025f) / 0.1f ), 8.0f) * 128.0f * 128.0f * 3.0f;

	f = (colc + vec4(gridc)) * clamp(1.4f - length(v), 0.1f, 1.0f);
}