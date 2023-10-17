function normal		(shader, t_base, t_second, t_detail)
	shader:begin	("particle",	"particle_add")
			: sorting	(3, false)
			: blend		(true,blend.one,blend.one)
			: aref 		(false,0)
			: zb 		(true,false)
			: fog		(false)
			: distort 	(true)

	shader:dx10texture	("s_base"		,t_base)
	shader:dx10texture	("s_depth",		"$user$copy_depth")

	shader:dx10sampler	("smp_base")
	shader:dx10sampler	("smp_nofilter")
end

function l_special	(shader, t_base, t_second, t_detail)
	shader:begin	("particle",	"particle_distort")
			: sorting	(3, false)
			: blend		(true,blend.srcalpha,blend.invsrcalpha)
			: zb 		(true,false)
			: fog		(false)
			: distort 	(true)

	shader:dx10texture	("s_base"		,t_base)
	shader:dx10texture	("s_distort"	,t_second)	-- "pfx\\pfx_distortion"
	shader:dx10texture	("s_depth",		"$user$copy_depth")

	shader:dx10sampler	("smp_base")
	shader: dx10sampler ("smp_linear")
	shader:dx10sampler	("smp_nofilter")
end
