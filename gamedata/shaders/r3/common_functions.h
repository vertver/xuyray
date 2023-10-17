#ifndef	common_functions_h_included
#define	common_functions_h_included

//	contrast function
float Contrast(float Input, float ContrastPower)
{
     //piecewise contrast function
     bool IsAbovefloat = Input > 0.5f ;
     float ToRaise = saturate(2.0f*(IsAbovefloat ? 1.0f-Input : Input));
     float Output = 0.5f*pow(ToRaise, ContrastPower); 
     Output = IsAbovefloat ? 1.0f-Output : Output;
     return Output;
}

void tonemap( out float4 low, out float4 high, float3 rgb, float scale)
{
	rgb		=	rgb*scale;
	const float fWhiteIntensity = 1.7f;
	const float fWhiteIntensitySQR = fWhiteIntensity*fWhiteIntensity;
	low		=	( (rgb*(1+rgb/fWhiteIntensitySQR)) / (rgb+1.0f) ).xyzz;
	high	=	rgb.xyzz/def_hdr;	// 8x dynamic range
}

float4 combine_bloom( float3  low, float4 high)	
{
        return float4( low + high*high.a, 1.h );
}

float calc_fogging( float4 w_pos )      
{
	return dot(w_pos,fog_plane);         
}

float2 unpack_tc_base( float2 tc, float du, float dv )
{
		return (tc.xy + float2	(du,dv))*(32.f/32768.f); //!Increase from 32bit to 64bit floating point
}

float3 calc_sun_r1( float3 norm_w )    
{
	return L_sun_color*saturate(dot((norm_w),-L_sun_dir_w));                 
}

float3 calc_model_hemi_r1( float3 norm_w )    
{
 return max(0,norm_w.y)*L_hemi_color;
}

float3 calc_model_lq_lighting( float3 norm_w )    
{
	return L_material.x*calc_model_hemi_r1(norm_w) + L_ambient + L_material.y*calc_sun_r1(norm_w);
}

float3 	unpack_normal( float3 v )	{ return 2*v-1; }
float3 	unpack_bx2( float3 v )	{ return 2*v-1; }
float3 	unpack_bx4( float3 v )	{ return 4*v-2; } //!reduce the amount of stretching from 4*v-2 and increase precision
float2 	unpack_tc_lmap( float2 tc )	{ return tc*(1.f/32768.f);	} // [-1  .. +1 ] 
float4	unpack_color( float4 c ) { return c.bgra; }
float4	unpack_D3DCOLOR( float4 c ) { return c.bgra; }
float3	unpack_D3DCOLOR( float3 c ) { return c.bgr; }

float3   p_hemi( float2 tc )
{
//	float3	t_lmh = tex2D (s_hemi, tc);
//	float3	t_lmh = s_hemi.Sample( smp_rtlinear, tc);
//	return	dot(t_lmh,1.h/4.h);
	float4	t_lmh = s_hemi.Sample( smp_rtlinear, tc);
	return	t_lmh.a;
}

float   get_hemi( float4 lmh)
{
	return lmh.a;
}

float   get_sun( float4 lmh)
{
	return lmh.g;
}

float3	v_hemi(float3 n)
{
	return L_hemi_color*(.5f + .5f*n.y);                   
}

float3	v_sun(float3 n)                        	
{
	return L_sun_color*dot(n,-L_sun_dir_w);                
}

float3	calc_reflection( float3 pos_w, float3 norm_w )
{
    return reflect(normalize(pos_w-eye_position), norm_w);
}

#define USABLE_BIT_1                uint(0x00002000)
#define USABLE_BIT_2                uint(0x00004000)
#define USABLE_BIT_3                uint(0x00008000)
#define USABLE_BIT_4                uint(0x00010000)
#define USABLE_BIT_5                uint(0x00020000)
#define USABLE_BIT_6                uint(0x00040000)
#define USABLE_BIT_7                uint(0x00080000)
#define USABLE_BIT_8                uint(0x00100000)
#define USABLE_BIT_9                uint(0x00200000)
#define USABLE_BIT_10               uint(0x00400000)
#define USABLE_BIT_11               uint(0x00800000)   // At least two of those four bit flags must be mutually exclusive (i.e. all 4 bits must not be set together)
#define USABLE_BIT_12               uint(0x01000000)   // This is because setting 0x47800000 sets all 5 FP16 exponent bits to 1 which means infinity
#define USABLE_BIT_13               uint(0x02000000)   // This will be translated to a +/-MAX_FLOAT in the FP16 render target (0xFBFF/0x7BFF), overwriting the 
#define USABLE_BIT_14               uint(0x04000000)   // mantissa bits where other bit flags are stored.
#define USABLE_BIT_15               uint(0x80000000)
#define MUST_BE_SET                 uint(0x40000000)   // This flag *must* be stored in the floating-point representation of the bit flag to store

// Holger Gruen AMD - I change normal packing and unpacking to make sure N.z is accessible without ALU cost
// this help the HDAO compute shader to run more efficiently
float2 gbuf_pack_normal( float3 norm )
{
   float2 res;

   res.x  = norm.z;
   res.y  = 0.5f * ( norm.x + 1.0f ) ;
   res.y *= ( norm.y < 0.0f ? -1.0f : 1.0f );

   return res;
}

float3 gbuf_unpack_normal( float2 norm )
{
   float3 res;

   res.z  = norm.x;
   res.x  = ( 2.0f * abs( norm.y ) ) - 1.0f;
   res.y = ( norm.y < 0 ? -1.0 : 1.0 ) * sqrt( abs( 1 - res.x * res.x - res.z * res.z ) );

   return res;
}

float gbuf_pack_hemi_mtl( float hemi, float mtl )
{
	uint packed_mtl = uint( ( mtl / 1.333333333 ) * 31.0 );
	uint packed = ( MUST_BE_SET + ( uint( saturate(hemi) * 255.9 ) << 13 ) + ( ( packed_mtl & uint( 31 ) ) << 21 ) );

	if( ( packed & USABLE_BIT_13 ) == 0 )
		packed |= USABLE_BIT_14;

	if( packed_mtl & uint( 16 ) )
		packed |= USABLE_BIT_15;

   return asfloat( packed );
}

float gbuf_unpack_hemi( float mtl_hemi )
{
	return float((asuint( mtl_hemi) >> 13) & uint(255)) * (1.0/254.8);
}

float gbuf_unpack_mtl( float mtl_hemi )
{
	uint packed       = asuint( mtl_hemi );
	uint packed_hemi  = ( ( packed >> 21 ) & uint(15) ) + ( ( packed & USABLE_BIT_15 ) == 0 ? 0 : 16 );
	return float( packed_hemi ) * (1.0/31.0) * 1.333333333;
}

float2 PackNormalOctEncode(float3 n)
{
	float l1norm    = dot(abs(n), 1.0);
	float2 res0     = n.xy * (1.0 / l1norm);
	float2 val      = 1.0 - abs(res0.yx);
	return (n.zz < float2(0.0, 0.0) ? (res0 >= 0.0 ? val : -val) : res0);
}

float3 UnpackNormalOctEncode(float2 f)
{
	float3 n = float3(f.x, f.y, 1.0 - abs(f.x) - abs(f.y));
	float2 val = 1.0 - abs(n.yx);
	n.xy = (n.zz < float2(0.0, 0.0) ? (n.xy >= 0.0 ? val : -val) : n.xy);
	return normalize(n);
}

#ifndef EXTEND_F_DEFFER
f_deffer pack_gbuffer( float3 norm, float material_id, float hemi, float4 col, float4 motion)
#else
f_deffer pack_gbuffer( float3 norm, float material_id, float hemi, float4 col, float4 motion, uint imask )
#endif
{
	f_deffer res;

	res.Ne.xy = PackNormalOctEncode(norm.xyz);
	res.Ne.z = material_id;
	res.Ne.w = hemi;

	res.C = col;
	res.motion = motion;

#ifdef EXTEND_F_DEFFER
   res.mask = imask;
#endif

	return res;
}

float3 gbuf_unpack_position(float2 tc)
{
	float depth	= s_depth.Sample(smp_nofilter, tc);
   tc = tc * 2.f - 1.f;
   depth = m_P._34 / (depth - m_P._33);  
   return float3(tc * pos_decompression_params.xy, 1.f) * depth;
}

float3 gbuf_unpack_position_unjiterred(float2 tc)
{	
	float depth	= s_depth.Sample(smp_nofilter, tc);
	float j_x = camera_jitter.x;
	float j_y = camera_jitter.y;
	tc.x += j_x;
	tc.y += j_y;
	tc += pos_decompression_params2.zw * -0.5f;
   tc = tc * 2.f - 1.f;
   depth = m_P._34 / (depth - m_P._33);  
   return float3(tc * pos_decompression_params.xy, 1.f) * depth;
}

gbuffer_data gbuffer_load_data( float2 tc : TEXCOORD, uint iSample )
{
	gbuffer_data gbd;

	float4 N				= s_normal.Sample( smp_nofilter, tc );
	gbd.P 				= gbuf_unpack_position(tc);
	gbd.P_Unjittered 	= gbuf_unpack_position_unjiterred(tc);
	gbd.N					= UnpackNormalOctEncode(N.xy);
	gbd.hemi				= N.w;
	gbd.mtl				= N.z;

	float4	C			= s_diffuse.Sample(  smp_nofilter, tc );
	float4	motion 	= s_motion.Load( int3( tc * pos_decompression_params2.xy, 0 ), iSample );

	gbd.C					= C.xyz;
	gbd.gloss			= C.w;
	gbd.motion  		= motion;

	return gbd;
}

gbuffer_data gbuffer_load_data( float2 tc : TEXCOORD  )
{
   return gbuffer_load_data( tc, 0 );
}

gbuffer_data gbuffer_load_data_offset( float2 tc : TEXCOORD, float2 OffsetTC : TEXCOORD, uint iSample )
{
   return gbuffer_load_data( OffsetTC, iSample );
}

#endif	//	common_functions_h_included
