
//=================================================
// Physical Material ParamBlock index
//=================================================
#if MAX_RELEASE <= 22000
#endif
#define fm_material_mode		0
#define fm_base_weight			1
#define fm_base_color			2
#define fm_reflectivity			3
#define fm_roughness			4
#define fm_roughness_inv		5
#define fm_metalness			6
#define fm_refl_color			10
#define fm_diff_roughness		11
#define fm_brdf_mode			12
#define fm_brdf_low				13
#define fm_brdf_high			14
#define fm_brdf_curve			15
#define fm_anisotropy			20
#define fm_anisoangle			21
#define fm_aniso_mode			22
#define fm_aniso_channel		23
#define fm_transparency			30
#define fm_trans_color			31
#define fm_trans_depth			32
#define fm_trans_roughness		35
#define fm_trans_roughness_inv	36
#define fm_trans_roughness_lock	34
#define fm_trans_ior			33
#define fm_thin_walled			37
#define fm_dispersion			38
#define fm_scattering			40
#define fm_sss_color			41
#define fm_sss_depth			42
#define fm_sss_scale			43
#define fm_sss_scatter_color	44
#define fm_emission				50
#define fm_emit_color			51
#define fm_emit_luminance		52
#define fm_emit_kelvin			54
#define fm_coating				60
#define fm_coat_color			61
#define fm_coat_roughness		62
#define fm_coat_roughness_inv 	63
#define fm_coat_affect_color	64
#define fm_coat_affect_roughness 65
#define fm_coat_ior				66
#define fm_sheen_weight			70	
#define fm_sheen_color			71
#define fm_sheen_roughness		72
#define fm_thin_film_weight		80
#define fm_thin_film_thickness	81
#define fm_thin_film_ior		82

#define fm_base_weight_map		100
#define fm_base_color_map		101
#define fm_reflectivity_map		102
#define fm_refl_color_map		103
#define fm_roughness_map		104
#define fm_metalness_map		105
#define fm_diff_rough_map		106
#define fm_anisotropy_map		107
#define fm_aniso_angle_map		108
#define fm_transparency_map		109
#define fm_trans_color_map		110
#define fm_trans_rough_map		111
#define fm_trans_ior_map		112
#define fm_scattering_map		113
#define fm_sss_color_map		114
#define fm_sss_scale_map		115
#define fm_emission_map			116
#define fm_emit_color_map		117
#define fm_coat_map				118
#define fm_coat_color_map		119
#define fm_coat_rough_map		120

#define fm_coat_aniso_map		121
#define fm_coat_aniso_angle_map	122
#define fm_sheen_map			123
#define fm_sheen_color_map		124
#define fm_sheen_rough_map		125
#define fm_thin_film_map		126
#define fm_thin_film_ior_map	127


#define fm_bump_map				130
#define fm_coat_bump_map		131
#define fm_displacement_map		132
#define fm_cutout_map			133
#define fm_base_weight_map_on	150
#define fm_base_color_map_on	151
#define fm_reflectivity_map_on	152
#define fm_refl_color_map_on	153
#define fm_roughness_map_on		154
#define fm_metalness_map_on		155
#define fm_diff_rough_map_on	156
#define fm_anisotropy_map_on	157
#define fm_aniso_angle_map_on	158
#define fm_transparency_map_on	159
#define fm_trans_color_map_on	160
#define fm_trans_rough_map_on	161
#define fm_trans_ior_map_on		162
#define fm_scattering_map_on	163
#define fm_sss_color_map_on		164
#define fm_sss_scale_map_on		165
#define fm_emission_map_on		166
#define fm_emit_color_map_on	167
#define fm_coat_map_on 			168
#define fm_coat_color_map_on	169
#define fm_coat_rough_map_on	170
#define fm_bump_map_on			180
#define fm_coat_bump_map_on		181
#define fm_displacement_map_on	182
#define fm_cutout_map_on		183
#define fm_bump_map_amt			230
#define fm_clearcoat_bump_map_amt	231
#define fm_displacement_map_amt		232
/*
#define fm_trans_scatter_color		//(Transparency_Scatter_Color) : color
#define fm_trans_scatter_aniso		//(Transparency_Scatter_Anisotropy) : float

#define fm_coat_anisotropy			//(Coating_Anisotropy) : float
#define fm_coat_anisoangle			//(Coating_Anisotropy_Angle) : float
#define fm_coat_aniso_map			//(Coating_Anisotropy_Map) : texturemap
#define fm_coat_aniso_angle_map		//(Coating_Anisotropy_Angle_Map) : texturemap

#define fm_coat_aniso_map_on		//: boolean
#define fm_coat_aniso_angle_map_on	//: boolean
#define fm_sheen_map_on				// : boolean
#define fm_sheen_color_map_on		// : boolean
#define fm_sheen_rough_map_on		// : boolean
#define fm_thin_film_map_on			// : boolean
#define fm_thin_film_ior_map_on		// : boolean
*/

//#define fm_EffectiveLuminance

//=================================================
// PBR (Metal/Rough) ParamBlock index
//=================================================
//  ParameterBlock0
#define pbr_ao_affects_diffuse		0
#define pbr_ao_affects_reflection	1
#define pbr_normal_flip_red			2
#define pbr_normal_flip_green		3

//  ParameterBlock1
#define pbr_base_color		0
#define pbr_base_color_map	1
#define pbr_metalness		2
#define pbr_metalness_map	3
#define pbr_roughness		4
#define pbr_roughness_map	5
#define pbr_useGrossiness	6
#define pbr_ao_map			7
#define pbr_bump_map_amt	8
#define pbr_norm_map		9
#define pbr_emit_color		10
#define pbr_emit_color_map	11
#define pbr_opacity_map		12
#define pbr_displacement_amt 13
//#define pbr_displacement_amt 14

//=================================================
// PBR (Specular/Glossines) ParamBlock index
//=================================================
//  ParameterBlock0
#define pbr_sg_ao_affects_diffuse		0
#define pbr_sg_ao_affects_reflection	1
#define pbr_sg_normal_flip_red			2
#define pbr_sg_normal_flip_green		3

//  ParameterBlock1
#define pbr_sg_base_color		0
#define pbr_sg_base_color_map	1
#define pbr_sg_specular			2
#define pbr_sg_specular_map		3
#define pbr_sg_glossiness		4
#define pbr_sg_glossiness_map	5
#define pbr_sg_useGrossiness	6
#define pbr_sg_ao_map			7
#define pbr_sg_bump_map_amt		8
#define pbr_sg_norm_map			9
#define pbr_sg_emit_color		10
#define pbr_sg_emit_color_map	11
#define pbr_sg_opacity_map		12
#define pbr_sg_displacement_amt 13
#define pbr_sg_displacement_map 14

//=================================================
// OpenPBR Material ParamBlock index
//=================================================
#define opbr_notes									0	// : string
#define opbr_base_weight							1	// : float
#define opbr_base_color								2	// : color
#define opbr_base_metalness							3	// : float
#define opbr_base_diffuse_roughness					4	// : float
#define opbr_specular_weight						5	//: float
#define opbr_specular_color							6	// : color
#define opbr_specular_roughness						7	// : float
#define opbr_specular_roughness_anisotropy			8	// : float
#define opbr_specular_ior							9	//: float
#define opbr_transmission_weight					10	// : float
#define opbr_transmission_color						11	// : color
#define opbr_transmission_depth						12	// : float
#define opbr_transmission_scatter					13	// : color
#define opbr_transmission_scatter_anisotropy		14	// : float
#define opbr_transmission_dispersion_scale			15	// : float
#define opbr_transmission_dispersion_abbe_number	16	// : float
#define opbr_subsurface_weight						17	// : float
#define opbr_subsurface_color						18	// : color
#define opbr_subsurface_radius						19	// : float
#define opbr_subsurface_radius_scale				20	// : color
#define opbr_subsurface_scatter_anisotropy			21	// : float
#define opbr_coat_weight							22	// : float
#define opbr_coat_color								23	// : color
#define opbr_coat_roughness							24	// : float
#define opbr_coat_roughness_anisotropy				25	// : float
#define opbr_coat_ior								26	// : float
#define opbr_coat_darkening							27	//: float
#define opbr_fuzz_weight							28	// : float
#define opbr_fuzz_color								29	// : color
#define opbr_fuzz_roughness							30	// : float
#define opbr_emission_weight						31	// : float
#define opbr_emission_color							32	// : color
#define opbr_emission_luminance						33	// : float
#define opbr_thin_film_weight						34	// : float
#define opbr_thin_film_thickness					35	// : float
#define opbr_thin_film_ior							36	// : float
#define opbr_geometry_thin_walled					38	// : boolean
#define opbr_base_weight_map						500	// : texturemap
#define opbr_base_weight_map_on						600	// : boolean
#define opbr_base_color_map							501	// : texturemap
#define opbr_base_color_map_on						601	// : boolean
#define opbr_base_metalness_map						502	// : texturemap
#define opbr_base_metalness_map_on					602	// : boolean
#define opbr_base_diffuse_roughness_map				503	// : texturemap
#define opbr_base_diffuse_roughness_map_on			603 //: boolean
#define opbr_specular_weight_map					504	// : texturemap
#define opbr_specular_weight_map_on					604	// : boolean
#define opbr_specular_color_map						505	// : texturemap
#define opbr_specular_color_map_on					605	// : boolean
#define opbr_specular_roughness_map					506	// : texturemap
#define opbr_specular_roughness_map_on				606	// : boolean
#define opbr_specular_roughness_anisotropy_map		507	// : texturemap
#define opbr_specular_roughness_anisotropy_map_on	607	// : boolean
#define opbr_specular_ior_map						508	// : texturemap
#define opbr_specular_ior_map_on					608	// : boolean
#define opbr_transmission_weight_map				509	// : texturemap
#define opbr_transmission_weight_map_on				609	// : boolean
#define opbr_transmission_color_map					510	// : texturemap
#define opbr_transmission_color_map_on				610	// : boolean
#define opbr_transmission_depth_map					511	// : texturemap
#define opbr_transmission_depth_map_on				611	// : boolean
#define opbr_transmission_scatter_map				512	// : texturemap
#define opbr_transmission_scatter_map_on			612	// : boolean
#define opbr_transmission_scatter_anisotropy_map	513	// : texturemap
#define opbr_transmission_scatter_anisotropy_map_on	613	// : boolean
#define opbr_transmission_dispersion_scale_map		514	// : texturemap
#define opbr_transmission_dispersion_scale_map_on	614	// : boolean
#define opbr_transmission_dispersion_abbe_number_map	515	// : texturemap
#define opbr_transmission_dispersion_abbe_number_map_on	615	// : boolean
#define opbr_subsurface_weight_map					516	// : texturemap
#define opbr_subsurface_weight_map_on				616	// : boolean
#define opbr_subsurface_color_map					517 //: texturemap
#define opbr_subsurface_color_map_on				617	// : boolean
#define opbr_subsurface_radius_map					518	// : texturemap
#define opbr_subsurface_radius_map_on				618	// : boolean
#define opbr_subsurface_radius_scale_map			519	// : texturemap
#define opbr_subsurface_radius_scale_map_on			619	// : boolean
#define opbr_subsurface_scatter_anisotropy_map		520	// : texturemap
#define opbr_subsurface_scatter_anisotropy_map_on	620	// : boolean
#define opbr_coat_weight_map						521	// : texturemap
#define opbr_coat_weight_map_on						621	// : boolean
#define opbr_coat_color_map							522	// : texturemap
#define opbr_coat_color_map_on						622	// : boolean
#define opbr_coat_roughness_map						523	// : texturemap
#define opbr_coat_roughness_map_on					623	// : boolean
#define opbr_coat_roughness_anisotropy_map			524	// : texturemap
#define opbr_coat_roughness_anisotropy_map_on		624	// : boolean
#define opbr_coat_ior_map							525	// : texturemap
#define opbr_coat_ior_map_on						625	// : boolean
#define opbr_coat_darkening_map						526	// : texturemap
#define opbr_coat_darkening_map_on					626	// : boolean
#define opbr_fuzz_weight_map						527	// : texturemap
#define opbr_fuzz_weight_map_on						627	// : boolean
#define opbr_fuzz_color_map							528 // : texturemap
#define opbr_fuzz_color_map_on						628	// : boolean
#define opbr_fuzz_roughness_map						529	// : texturemap
#define opbr_fuzz_roughness_map_on					629	// : boolean
#define opbr_emission_weight_map					530	// : texturemap
#define opbr_emission_weight_map_on					630	// : boolean
#define opbr_emission_color_map						531	// : texturemap
#define opbr_emission_color_map_on					631	// : boolean
#define opbr_emission_luminance_map					532	// : texturemap
#define opbr_emission_luminance_map_on				632	// : boolean
#define opbr_thin_film_weight_map					533	// : texturemap
#define opbr_thin_film_weight_map_on				633	// : boolean
#define opbr_thin_film_thickness_map				534	// : texturemap
#define opbr_thin_film_thickness_map_on				634	// : boolean
#define opbr_thin_film_ior_map						535	// : texturemap
#define opbr_thin_film_ior_map_on					635	// : boolean
#define opbr_bump_map								580	// : texturemap
#define opbr_bump_map_on							680	// : boolean
#define opbr_coat_bump_map							581	// : texturemap
#define opbr_coat_bump_map_on						681	// : boolean
#define opbr_displacement_map						582	// : texturemap
#define opbr_displacement_map_on					682	// : boolean
#define opbr_geometry_normal_map					583	// : texturemap
#define opbr_geometry_normal_map_on					683	// : boolean
#define opbr_geometry_tangent_map					584	// : texturemap
#define opbr_geometry_tangent_map_on				684	// : boolean
#define opbr_geometry_coat_normal_map				585	// : texturemap
#define opbr_geometry_coat_normal_map_on			685	// : boolean
#define opbr_geometry_coat_tangent_map				586	// : texturemap
#define opbr_geometry_coat_tangent_map_on			686	// : boolean
#define opbr_geometry_opacity_map					587	// : texturemap
#define opbr_geometry_opacity_map_on				687	// : boolean
#define opbr_bump_map_amt							780	// : float
#define opbr_coat_bump_map_amt						781	// : float
#define opbr_displacement_map_amt					782	// : float
//#define opbr_EffectiveLuminance : color



//=================================================
// glTF Material ParamBlock index
//=================================================
// ParamBlock 0
#define glTF_baseColor 			0		//: color
#define glTF_baseColorMap 		1	//: texturemap
#define glTF_alphaMode 			2	//: integer
#define glTF_alphaMap 			3	//: texturemap
#define glTF_alphaCutoff 		4	//: float
#define glTF_metalness		 	5	//: float
#define glTF_metalnessMap 		6	//: texturemap
#define glTF_roughness		 	7	//: float
#define glTF_roughnessMap 		8	//: texturemap
#define glTF_normal		 		9	//: float
#define glTF_normalMap 			10	//: texturemap
#define glTF_ambientOcclusion	 11	//: float
#define glTF_ambientOcclusionMap 12	//: texturemap
#define glTF_emissionColor 		13	//: color
#define glTF_emissionMap 		14	//: texturemap
#define glTF_Doublesided	 	15	//: boolean
// ParamBlock 1
#define glTF_unlit			 	0	//: boolean
#define glTF_enableClearcoat 	1	//: boolean
#define glTF_clearcoat		 	2	//: float
#define glTF_clearcoatMap 		3	//: texturemap
#define glTF_clearcoatRoughness 4	//: float
#define glTF_clearcoatRoughnessMap 5	//: texturemap
#define glTF_clearcoatNormal	6	//: float
#define glTF_clearcoatNormalMap 7	//: texturemap
#define glTF_enableSheen 		8	//: boolean
#define glTF_sheenColor 		9	//: color
#define glTF_sheenColorMap 		10	//: texturemap
#define glTF_sheenRoughness 	11	//: float
#define glTF_sheenRoughnessMap 	12	//: texturemap

#define glTF_enableSpecular		13	// : boolean
#define glTF_specular			14	// : float
#define glTF_specularMap		15	// : texturemap
#define glTF_specularcolor		16	// : color
#define glTF_specularColorMap	17	// : texturemap

#define glTF_enableTransmission 18	// : boolean
#define glTF_transmission		19	// : float
#define glTF_transmissionMap 	20	// : texturemap

#define glTF_enableVolume 		21	// : boolean
#define glTF_volumeThickness 	22	// : float
#define glTF_volumeThicknessMap 23	// : texturemap
#define glTF_volumeDistance 	24	// : float
#define glTF_volumeColor 		25	// : color

#define glTF_enableIndexOfRefraction 26	// : boolean
#define glTF_indexOfRefraction 	27	// : float



//#define glTF_enableAlpha 			//: boolean

//=================================================
// Arnold (Standard Surface) ParamBlock index
//=================================================
//  ParameterBlock0
//#define an_sf_TheList 						// maxObject array
#define an_sf_Shader_Version 				0	// integer

//  ParameterBlock1
#define an_sf_base							0	// float
#define an_sf_base_connected 				1	// boolean
#define an_sf_base_shader 					2	// texturemap
#define an_sf_base_color 					3	// color
#define an_sf_base_color_connected 			4	// boolean
#define an_sf_base_color_shader 			5	// texturemap
#define an_sf_diffuse_roughness 			6	// float
#define an_sf_diffuse_roughness_connected 	7	// boolean
#define an_sf_diffuse_roughness_shader 		8	// texturemap
#define an_sf_specular 						9	// float
#define an_sf_specular_connected 			10	// boolean
#define an_sf_specular_shader 				11	// texturemap
#define an_sf_specular_color 				12	// color
#define an_sf_specular_color_connected 		13	// boolean
#define an_sf_specular_color_shader 		14	// texturemap
#define an_sf_specular_roughness 			15	// float
#define an_sf_specular_roughness_connected 	16	// boolean
#define an_sf_specular_roughness_shader 	17	// texturemap
#define an_sf_specular_IOR 					18	// float
#define an_sf_specular_IOR_connected 		19	// boolean
#define an_sf_specular_IOR_shader 			20	// texturemap
#define an_sf_specular_anisotropy 			21	// float
#define an_sf_specular_anisotropy_connected 22	// boolean
#define an_sf_specular_anisotropy_shader 	23	// texturemap
#define an_sf_specular_rotation 			24	// float
#define an_sf_specular_rotation_connected 	25	// boolean
#define an_sf_specular_rotation_shader 		26	// texturemap
#define an_sf_metalness 					27	// float
#define an_sf_metalness_connected 			28	// boolean
#define an_sf_metalness_shader 				29	// texturemap
#define an_sf_transmission 					30	// float
#define an_sf_transmission_connected 		31	// boolean
#define an_sf_transmission_shader 			32	// texturemap
#define an_sf_transmission_color 			33	// color
#define an_sf_transmission_color_connected 	34	// boolean
#define an_sf_transmission_color_shader 	35	// texturemap
#define an_sf_transmission_depth 			36	// float
#define an_sf_transmission_depth_connected 	37	// boolean
#define an_sf_transmission_depth_shader 	38	// texturemap
#define an_sf_transmission_scatter 			39	// color
#define an_sf_transmission_scatter_connected 	40	// boolean
#define an_sf_transmission_scatter_shader 		41	// texturemap
#define an_sf_transmission_scatter_anisotropy 	42	// float
#define an_sf_transmission_scatter_anisotropy_connected 43	// boolean
#define an_sf_transmission_scatter_anisotropy_shader 	44	// texturemap
#define an_sf_transmission_dispersion 			45			// float
#define an_sf_transmission_dispersion_connected 46			// boolean
#define an_sf_transmission_dispersion_shader 	47	// texturemap
#define an_sf_transmission_extra_roughness 		48	// float
#define an_sf_transmission_extra_roughness_connected 49	// boolean
#define an_sf_transmission_extra_roughness_shader 	50	// texturemap
#define an_sf_transmit_aovs 				51	// boolean
#define an_sf_subsurface 					52	// float
#define an_sf_subsurface_connected 			53	// boolean
#define an_sf_subsurface_shader 			54	// texturemap
#define an_sf_subsurface_color 				55	// color
#define an_sf_subsurface_color_connected 	56	// boolean
#define an_sf_subsurface_color_shader 		57	// texturemap
#define an_sf_subsurface_radius 			58	// color
#define an_sf_subsurface_radius_connected 	59	// boolean
#define an_sf_subsurface_radius_shader 		60	// texturemap
#define an_sf_subsurface_scale 				61	// float
#define an_sf_subsurface_scale_connected 	62	// boolean
#define an_sf_subsurface_scale_shader 		63	// texturemap
#define an_sf_subsurface_anisotropy 		64	// float
#define an_sf_subsurface_anisotropy_connected 	65	// boolean
#define an_sf_subsurface_anisotropy_shader 		66	// texturemap
#define an_sf_subsurface_type 				67	// integer
#define an_sf_sheen 						68	// float
#define an_sf_sheen_connected 				69	// boolean
#define an_sf_sheen_shader 					70	// texturemap
#define an_sf_sheen_color 					71	// color
#define an_sf_sheen_color_connected 		72	// boolean
#define an_sf_sheen_color_shader 			73	// texturemap
#define an_sf_sheen_roughness 				74	// float
#define an_sf_sheen_roughness_connected 	75	// boolean
#define an_sf_sheen_roughness_shader 		76	// texturemap
#define an_sf_thin_walled 					77	// boolean
#define an_sf_normal 						78	// point3
#define an_sf_normal_connected 				79	// boolean
#define an_sf_normal_shader 				80	// texturemap
#define an_sf_tangent 						81	// point3
#define an_sf_tangent_connected 			82	// boolean
#define an_sf_tangent_shader 				83	// texturemap
#define an_sf_coat 							84	// float
#define an_sf_coat_connected 				85	// boolean
#define an_sf_coat_shader 					86	// texturemap
#define an_sf_coat_color 					87	// color
#define an_sf_coat_color_connected 			88	// boolean
#define an_sf_coat_color_shader 			89	// texturemap
#define an_sf_coat_roughness 				90	// float
#define an_sf_coat_roughness_connected 	91	// boolean
#define an_sf_coat_roughness_shader 		92	// texturemap
#define an_sf_coat_IOR 						93	// float
#define an_sf_coat_IOR_connected 			94	// boolean
#define an_sf_coat_IOR_shader 				95	// texturemap
#define an_sf_coat_anisotropy 				96	// float
#define an_sf_coat_anisotropy_connected 	97	// boolean
#define an_sf_coat_anisotropy_shader 		98	// texturemap
#define an_sf_coat_rotation 				99	// float
#define an_sf_coat_rotation_connected 		100	// boolean
#define an_sf_coat_rotation_shader 			101	// texturemap
#define an_sf_coat_normal 					102	// point3
#define an_sf_coat_normal_connected 		103	// boolean
#define an_sf_coat_normal_shader 			104	// texturemap
#define an_sf_coat_affect_color 			105	// float
#define an_sf_coat_affect_color_connected 	106	// boolean
#define an_sf_coat_affect_color_shader 		107	// texturemap
#define an_sf_coat_affect_roughness 		108	// float
#define an_sf_coat_affect_roughness_connected 	109	// boolean
#define an_sf_coat_affect_roughness_shader 		110	// texturemap
#define an_sf_thin_film_thickness 			111	// float
#define an_sf_thin_film_thickness_connected 112	// boolean
#define an_sf_thin_film_thickness_shader 	113	// texturemap
#define an_sf_thin_film_IOR 				114	// float
#define an_sf_thin_film_IOR_connected 		115	// boolean
#define an_sf_thin_film_IOR_shader 			116	// texturemap
#define an_sf_emission 						117	// float
#define an_sf_emission_connected 			118	// boolean
#define an_sf_emission_shader 				119	// texturemap
#define an_sf_emission_color 				120	// color
#define an_sf_emission_color_connected 		121	// boolean
#define an_sf_emission_color_shader 		122	// texturemap
#define an_sf_opacity 						123	// color
#define an_sf_opacity_connected 			124	// boolean
#define an_sf_opacity_shader 				125	// texturemap
#define an_sf_caustics 						126	// boolean
#define an_sf_internal_reflections 			127	// boolean
#define an_sf_exit_to_background 			128	// boolean
#define an_sf_indirect_diffuse 				129	// float
#define an_sf_indirect_specular 			130	// float
#define an_sf_dielectric_priority 			131	// integer
#define an_sf_aov_id1 						132	// string
#define an_sf_id1 							133	// color
#define an_sf_id1_connected 				135	// boolean
#define an_sf_id1_shader 					136	// texturemap
#define an_sf_aov_id2 						137	// string
#define an_sf_id2 							139	// color
#define an_sf_id2_connected 				140	// boolean
#define an_sf_id2_shader 					141	// texturemap
#define an_sf_aov_id3 						142	// string
#define an_sf_id3 							144	// color
#define an_sf_id3_connected 				145	// boolean
#define an_sf_id3_shader 					146	// texturemap
#define an_sf_aov_id4 						147	// string
#define an_sf_id4 							149	// color
#define an_sf_id4_connected 				150	// boolean
#define an_sf_id4_shader 					151	// texturemap
#define an_sf_aov_id5 						152	// string
#define an_sf_id5 							154	// color
#define an_sf_id5_connected 				155	// boolean
#define an_sf_id5_shader 					156	// texturemap
#define an_sf_aov_id6 						157	// string
#define an_sf_id6 							159	// color
#define an_sf_id6_connected 				160	// boolean
#define an_sf_id6_shader 					161	// texturemap
#define an_sf_aov_id7 						162	// string
#define an_sf_id7 							164	// color
#define an_sf_id7_connected 				165	// boolean
#define an_sf_id7_shader 					166	// texturemap
#define an_sf_aov_id8 						167	// string
#define an_sf_id8 							169	// color
#define an_sf_id8_connected 				170	// boolean
#define an_sf_id8_shader 					171	// texturemap

//=================================================
// VRay ParamBlock index
//=================================================
// ParameterBlock0
#define vr_preset									93 //  integer
#define vr_diffuse 									1 // RGB color
#define vr_diffuse_roughness 						68 // float
#define vr_selfIllumination 						81 // RGB color
#define vr_selfIllumination_gi 						82 // boolean
#define vr_selfIllumination_multiplier 			83 // float
#define vr_compensate_camera_exposure 				90 // boolean
#define vr_reflection 								2 // RGB color
#define vr_reflection_glossiness 					3 // float
#define vr_hilight_glossiness						62 // float (notUsed) 
#define vr_reflection_subdivs 						4 // integer
#define vr_reflection_fresnel 						8 // boolean
#define vr_reflection_maxDepth 						20 // integer
#define vr_reflection_exitColor						51 // RGB color (reflect_exitColor) 
#define vr_reflection_useInterpolation				56 // boolean (useInterpolation) 	
#define vr_reflection_ior 							63 // float
#define vr_reflection_metalness 					91 // float
#define vr_reflection_lockGlossiness 				64 // boolean
#define vr_reflection_lockIOR 						65 // boolean
#define vr_reflection_dimDistance 					71 // worldUnits
#define vr_reflection_dimDistance_on 				73 // boolean
#define vr_reflection_dimDistance_falloff			72 // float
#define vr_reflection_affectAlpha 					80 // integer
#define vr_refraction 								5 // RGB color
#define vr_refraction_glossiness 					6 // float
#define vr_refraction_subdivs 						7 // integer
#define vr_refraction_ior 							9 // float
#define vr_refraction_fogColor 						23 // RGB color
#define vr_refraction_fogMult						24 // float (refraction_fogMultiplier) 
#define vr_refraction_fogBias 						66 // float
#define vr_refraction_affectShadows 				32 // boolean
#define vr_refraction_affectAlpha 					61 // integer
#define vr_refraction_maxDepth 						21 // integer
#define vr_refraction_exitColor						52 // RGB color (refract_exitColor) 	
#define vr_refraction_useExitColor					53 // boolean (refract_useExitColor) 
#define vr_refraction_useInterpolation 			57 // boolean
#define vr_refraction_dispersion					74 // float (refract_dispersion) 
#define vr_refraction_dispersion_on				75 // boolean (refract_dispersion_on) 
#define vr_translucency_on							25 // integer (translucency_type) 	
#define vr_translucency_thickness 					26 // worldUnits
#define vr_translucency_scatterCoeff 				29 // float
#define vr_translucency_fbCoeff 					30 // float
#define vr_translucency_multiplier 				31 // float
#define vr_translucency_color						37 // RGB color (translucent_color) 	

// ParameterBlock1
#define vr_brdf_type 								10 // integer
#define vr_anisotropy								54 // float (brdf_anisotropy) 
#define vr_anisotropy_rotation						58 // float (brdf_anisotropy_rotation) 
#define vr_anisotropy_derivation					59 // integer (brdf_anisotropy_derivation) 
#define vr_anisotropy_axis							60 // integer (brdf_anisotropy_axis) 
#define vr_anisotropy_channel						55 // integer (brdf_anisotropy_channel) 
#define vr_soften									69 // float (brdf_soften) 
#define vr_brdf_fixDarkEdges 						77 // boolean
#define vr_gtr_gamma 								86 // float
#define vr_gtr_oldGamma 							87 // boolean
#define vr_brdf_useRoughness						89 // boolean (option_useRoughness) 	

// ParameterBlock2
#define vr_option_traceDiffuse 						15 // boolean
#define vr_option_traceReflection 					11 // boolean
#define vr_option_traceRefraction 					12 // boolean
#define vr_option_doubleSided 						13 // boolean
#define vr_option_reflectOnBack 					14 // boolean
#define vr_option_useIrradMap 						17 // boolean
#define vr_refraction_fogUnitsScale_on				76 // boolean (refraction_fogUnitScale_on) 
#define vr_option_traceDiffuseAndGlossy 			18 // integer
#define vr_option_cutOff 							22 // float
#define vr_preservationMode 						36 // integer
#define vr_option_environment_priority 			67 // integer
#define vr_effect_id 								78 // integer
#define vr_override_effect_id 						79 // boolean
#define vr_option_clampTextures 					70 // boolean
#define vr_option_opacityMode 						84 // integer
#define vr_option_glossyFresnel 					88 // boolean
#define vr_option_diffuse_roughness_model 			92 // integer

// ParameterBlock2 3
#define vr_texmap_diffuse 							_T("texmap_diffuse") // texturemap
#define vr_texmap_diffuse_on 						150 // boolean
#define vr_texmap_diffuse_multiplier 				200 // float
#define vr_texmap_reflection 						101 // texturemap
#define vr_texmap_reflection_on 					151 // boolean
#define vr_texmap_reflection_multiplier 			201 // float
#define vr_texmap_refraction_Str						_T("texmap_refraction")  // texturemap
#define vr_texmap_refraction_on 					152 // boolean
#define vr_texmap_refraction_multiplier			202 // float
#define vr_texmap_bump_Str							_T("texmap_bump") // texturemap
#define vr_texmap_bump_on 							153 // boolean
#define vr_texmap_bump_multiplier 					102 // float
#define vr_texmap_reflectionGlossiness				104 // texturemap
#define vr_texmap_reflectionGlossiness_Str			_T("texmap_reflectionGlossiness") // texturemap
#define vr_texmap_reflectionGlossiness_on 			154 // boolean
#define vr_texmap_reflectionGlossiness_multiplier 	204 // float
#define vr_texmap_refractionGlossiness_Str			_T("texmap_refractionGlossiness") // texturemap
#define vr_texmap_refractionGlossiness_on 			155 // boolean
#define vr_texmap_refractionGlossiness_multiplier 	205 // float
#define vr_texmap_refractionIOR						109 // texturemap (texmap_ior) 
#define vr_texmap_refractionIOR_on					159 // boolean (texmap_ior_on) 	
#define vr_texmap_refractionIOR_multiplier			209 // float (texmap_ior_multiplier) 
#define vr_texmap_displacement 						106 // texturemap
#define vr_texmap_displacement_on 					156 // boolean
#define vr_texmap_displacement_multiplier 			206 // float
#define vr_texmap_translucent 						108 // texturemap
#define vr_texmap_translucent_on 					158 // boolean
#define vr_texmap_translucent_multiplier 			208 // float
#define vr_texmap_environment 						107 // texturemap
#define vr_texmap_environment_on 					157 // boolean
#define vr_texmap_hilightGlossiness 				110 // texturemap
#define vr_texmap_hilightGlossiness_on 			160 // boolean
#define vr_texmap_hilightGlossiness_multiplier		210 // float (notUsed) 
#define vr_texmap_reflectionIOR 					111 // texturemap
#define vr_texmap_reflectionIOR_on 				161 // boolean
#define vr_texmap_reflectionIOR_multiplier 		211 // float
#define vr_texmap_opacity 							112 // texturemap
#define vr_texmap_opacity_on 						162 // boolean
#define vr_texmap_opacity_multiplier 				212 // float
#define vr_texmap_roughness 						113 // texturemap
#define vr_texmap_roughness_on 						163 // boolean
#define vr_texmap_roughness_multiplier 			213 // float
#define vr_texmap_anisotropy 						114 // texturemap
#define vr_texmap_anisotropy_on 					164 // boolean
#define vr_texmap_anisotropy_multiplier 			214 // float
#define vr_texmap_anisotropy_rotation 				115 // texturemap
#define vr_texmap_anisotropy_rotation_on 			165 // boolean
#define vr_texmap_anisotropy_rotation_multiplier 	215 // float
#define vr_texmap_refraction_fog (texmap_fog) 		116 // texturemap
#define vr_texmap_refraction_fog_on				166 // boolean (texmap_fog_on) 
#define vr_texmap_refraction_fog_multiplier 		216 // float (texmap_fog_multiplier)
#define vr_texmap_self_illumination_Str			_T("texmap_self_illumination") // texturemap (texmap_selfIllumination)
#define vr_texmap_self_illumination_on 			167 // boolean (texmap_selfIllumination_on)
#define vr_texmap_self_illumination_multiplier		217 // float (texmap_selfIllumination_multiplier)
#define vr_texmap_gtr_tail_falloff 				118 // texturemap
#define vr_texmap_gtr_tail_falloff_on 				168 // boolean
#define vr_texmap_gtr_tail_falloff_multiplier 		218 // float
#define vr_texmap_metalness_Str						_T("texmap_metalness")  // texturemap
#define vr_texmap_metalness_on 						169 // boolean
#define vr_texmap_metalness_multiplier 			219 // float

#define vr_sheen_color_Str							_T("sheen_color")				//  color
#define vr_sheen_glossiness_Str						_T("sheen_glossiness")			//  float

#define vr_texmap_sheen_Str							_T("texmap_sheen")				//  texturemap
#define vr_texmap_sheen_glossiness_Str				_T("texmap_sheen_glossiness")	//  texturemap
#define vr_texmap_sheen_on_Str						_T("texmap_sheen_on")			//  boolean
#define vr_texmap_sheen_multiplier_Str				_T("texmap_sheen_multiplier")	//  float
#define vr_texmap_sheen_glossiness_Str				_T("texmap_sheen_glossiness")	//  texturemap
#define vr_texmap_sheen_glossiness_on_Str			_T("texmap_sheen_glossiness_on")//  boolean
#define vr_texmap_sheen_glossiness_multiplier_Str	_T("texmap_sheen_glossiness_multiplier")	//  float



#define vr_coat_color_Str							_T("coat_color")					//  color
#define vr_coat_amount_Str							_T("coat_amount")					//  float
#define vr_coat_amount								96	//  float
#define vr_coat_glossinessStr						_T("coat_glossiness")				//  float
#define vr_coat_glossiness							98				//  float
#define vr_coat_ior									_T("coat_ior")						//  float
#define vr_coat_bump_lock							_T("coat_bump_lock")				//  boolean
#define vr_coat_bump_on_Str							_T("coat_bump_on")					//  boolean
#define vr_coat_bump_multiplier	_Str				_T("coat_bump_multiplier")			//  float
#define vr_texmap_coat_color_Str					_T("texmap_coat_color")				//  texturemap
#define vr_texmap_coat_color_on_Str				_T("texmap_coat_color_on")			//  boolean
#define vr_texmap_coat_color_multiplier_Str		_T("texmap_coat_color_multiplier")	//  float
#define vr_texmap_coat_amount_Str					_T("texmap_coat_amount")			//  texturemap
#define vr_texmap_coat_amount_on_Str				_T("texmap_coat_amount_on")			//  boolean
#define vr_texmap_coat_amount_multiplier_Str		_T("texmap_coat_amount_multiplier")	//  float
#define vr_texmap_coat_glossiness_Str				_T("texmap_coat_glossiness")		//  texturemap
#define vr_texmap_coat_glossiness_on_Str			_T("texmap_coat_glossiness_on")		//  boolean
#define vr_texmap_coat_glossiness_multiplier_Str	_T("texmap_coat_glossiness_multiplier")		//  float
#define vr_texmap_coat_ior_Str						_T("texmap_coat_ior")				//  texturemap
#define vr_texmap_coat_ior_on_Str					_T("texmap_coat_ior_on")			//  boolean
#define vr_texmap_coat_ior_multiplier_Str			_T("texmap_coat_ior_multiplier")	//  float
#define vr_texmap_coat_bump_Str						_T("texmap_coat_bump")				//  texturemap
#define vr_texmap_coat_bump_on_Str					_T("texmap_coat_bump_on")			//  boolean
//#define vr_texmap_coat_bump_multiplier			_T("texmap_coat_bump_multiplier")	//  float
#define vr_texmap_coat_bump_multiplier				104	//  float

// ParameterBlock2 0
#define vr_thinfilm_on								141	// boolean
#define vr_thinfilm_thickness_min					137	// float
#define vr_thinfilm_thickness_max					142	// float
#define vr_thinfilm_ior								138	// float
#define vr_texmap_thinFilm_thickness				139 // Texmap
#define vr_texmap_thinfilm_ior						140	// Texmap

// ParameterBlock2 3
#define vr_texmap_thinfilm_thickness_multiplier	229	// float
#define vr_texmap_thinfilm_thickness_on			179 // Boolean
#define vr_texmap_thinfilm_ior_on					180 // Boolean
#define vr_texmap_thinfilm_ior_multiplier			230 // float


// ParameterBlock2 4
#define vr_reflect_minRate 					56 // integer
#define vr_reflect_maxRate 					39 // integer
#define vr_reflect_interpSamples 			40 // integer
#define vr_reflect_colorThreshold			43 // float (reflect_clrThresh) 
#define vr_reflect_normalThreshold			41 // float (reflect_nrmThresh) 	
#define vr_refract_minRate 					42 // integer

// VRayNormal
#define vr_nrm_normal_map					0	// texturemap
#define vr_nrm_normal_map_on 				2	// boolean
#define vr_nrm_normal_map_multiplier 		3	// float
#define vr_nrm_bump_map 					4	// texturemap
#define vr_nrm_bump_map_on 					5	// boolean
#define vr_nrm_bump_map_multiplier			6	// float
#define vr_nrm_map_channel 					1	// integer
#define vr_nrm_flip_red 					7	// boolean
#define vr_nrm_flip_green 					8	// boolean
#define vr_nrm_swap_red_and_green 			9	// boolean
#define vr_nrm_map_rotation 				10	// float
#define vr_nrm_apply_gamma 					11	// boolean
#define vr_nrm_blue2Z_mapping_method 		12	// integer

//=================================================
// Corona ParamBlock index
//=================================================
#define crn_baseColor						101	// color
#define crn_baseLevel						102	// float
#define crn_baseTexmap						103	// texturemap
#define crn_baseTexmapOn					104	// boolean
#define crn_baseMapAmount					105	// float
#define crn_metalnessMode					111	// integer
#define crn_opacityColor					121	// color
#define crn_opacityLevel					122	// float
#define crn_opacityTexmap					123	// texturemap
#define crn_opacityTexmapOn					124	// boolean
#define crn_opacityMapAmount				125	// float
#define crn_opacityCutout					126	// boolean
#define crn_baseRoughness					131	// float
#define crn_baseRoughnessTexmap				132	// texturemap
#define crn_baseRoughnessTexmapOn			133	// boolean
#define crn_baseRoughnessMapAmount			134	// float
#define crn_baseAnisotropy					141	// float
#define crn_baseAnisotropyTexmap			142	// texturemap
#define crn_baseAnisotropyTexmapOn			143	// boolean
#define crn_baseAnisotropyMapAmount		144	// float
#define crn_baseAnisoRotation				151	// float
#define crn_baseAnisoRotationTexmap		152	// texturemap
#define crn_baseAnisoRotationTexmapOn		153	// boolean
#define crn_baseAnisoRotationMapAmount		154	// float
#define crn_baseIor							171	// float
#define crn_baseIorTexmap					172	// texturemap
#define crn_baseIorTexmapOn					173	// boolean
#define crn_baseIorMapAmount				174	// float
#define crn_refractionAmount				191	// float
#define crn_refractionAmountTexmap			192	// texturemap
#define crn_refractionAmountTexmapOn		193	// boolean
#define crn_refractionAmountMapAmount		194	// float
#define crn_dispersionEnable				201	// boolean
#define crn_dispersion						202	// float
#define crn_useThinMode						203	// boolean
#define crn_useCaustics						204	// boolean
#define crn_clearcoatAmount					211	// float
#define crn_clearcoatAmountTexmap			212	// texturemap
#define crn_clearcoatAmountTexmapOn		213	// boolean
#define crn_clearcoatAmountMapAmount		214	// float
#define crn_clearcoatIor					221	// float
#define crn_clearcoatIorTexmap				222	// texturemap
#define crn_clearcoatIorTexmapOn			223	// boolean
#define crn_clearcoatIorMapAmount			224	// float
#define crn_clearcoatRoughness				231	// float
#define crn_clearcoatRoughnessTexmap		232	// texturemap
#define crn_clearcoatRoughnessTexmapOn		233	// boolean
#define crn_clearcoatRoughnessMapAmount	234	// float
#define crn_sheenAmount						241	// float
#define crn_sheenAmountTexmap				242	// texturemap
#define crn_sheenAmountTexmapOn				243	// boolean
#define crn_sheenAmountMapAmount			244	// float
#define crn_sheenColor						245	// color
#define crn_sheenColorTexmap				246	// texturemap
#define crn_sheenColorTexmapOn				247	// boolean
#define crn_sheenColorMapAmount				248	// float
#define crn_sheenRoughness					251	// float
#define crn_sheenRoughnessTexmap			252	// texturemap
#define crn_sheenRoughnessTexmapOn			253	// boolean
#define crn_sheenRoughnessMapAmount			254	// float
#define crn_volumetricAbsorptionColor		261	// color
#define crn_volumetricAbsorptionTexmap		262	// texturemap
#define crn_volumetricAbsorptionTexmapOn	263	// boolean
#define crn_volumetricAbsorptionMapAmount	264	// float
#define crn_volumetricScatteringColor		271	// color
#define crn_volumetricScatteringTexmap		272	// texturemap
#define crn_volumetricScatteringTexmapOn	273	// boolean
#define crn_volumetricScatteringMapAmount	274	// float
#define crn_attenuationDistance				281	// worldUnits
#define crn_scatterDirectionality			282	// float
#define crn_scatterSingleBounce				283	// boolean
#define crn_sssAmount						291	// float
#define crn_sssAmountTexmap					292	// texturemap
#define crn_sssAmountTexmapOn				293	// boolean
#define crn_sssAmountMapAmount				294	// float
#define crn_sssRadius						301	// worldUnits
#define crn_sssRadiusTexmap					302	// texturemap
#define crn_sssRadiusTexmapOn				303	// boolean
#define crn_sssRadiusMapAmount				304	// float
#define crn_sssScatterColor					311	// color
#define crn_sssScatterTexmap				312	// texturemap
#define crn_sssScatterTexmapOn				313	// boolean
#define crn_sssScatterMapAmount				314	// float
#define crn_displacementMinimum				321	// worldUnits
#define crn_displacementMaximum				322	// worldUnits
#define crn_displacementWaterLevelOn		323	// boolean
#define crn_displacementWaterLevel			324	// float
#define crn_displacementTexmap				325	// texturemap
#define crn_displacementTexmapOn			326	// boolean
#define crn_selfIllumColor					331	// color
#define crn_selfIllumLevel					332	// float
#define crn_selfIllumTexmap					333	// texturemap
#define crn_selfIllumTexmapOn				334	// boolean
#define crn_selfIllumMapAmount				335	// float
#define crn_alphaMode						341	// integer
#define crn_gBufferOverride					342	// integer
#define crn_anisotropyOrientationMode		343	// integer
#define crn_anisotropyOrientationUvwChannel	344	// integer
#define crn_renderElementPropagation		345	// integer
#define crn_materialLibraryId				346	// string
#define crn_baseBumpTexmap					351	// texturemap
#define crn_baseBumpTexmapOn				352	// boolean
#define crn_baseBumpMapAmount				353	// float
#define crn_bgOverrideReflectTexmap			361	// texturemap
#define crn_bgOverrideReflectTexmapOn		362	// boolean
#define crn_bgOverrideRefractTexmap			363	// texturemap
#define crn_bgOverrideRefractTexmapOn		364	// boolean
#define crn_translucencyFraction			371	// float
#define crn_translucencyFractionTexmap		372	// texturemap
#define crn_translucencyFractionTexmapOn	373	// boolean
#define crn_translucencyFractionMapAmount	374	// float
#define crn_thinAbsorptionColor				381	// color
#define crn_thinAbsorptionTexmap			382	// texturemap
#define crn_thinAbsorptionTexmapOn			383	// boolean
#define crn_thinAbsorptionMapAmount			384	// float
#define crn_clearcoatAbsorptionColor		391	// color
#define crn_clearcoatAbsorptionTexmap		392	// texturemap
#define crn_clearcoatAbsorptionTexmapOn		393	// boolean
#define crn_clearcoatAbsorptionMapAmount	394	// float
#define crn_clearcoatBumpTexmap				401	// texturemap
#define crn_clearcoatBumpTexmapOn			402	// boolean
#define crn_clearcoatBumpMapAmount			403	// float
#define crn_metalnessTexmap					411	// texturemap
#define crn_metalnessTexmapOn				412	// boolean
#define crn_roughnessMode					421	// integer
#define crn_preset							431	// integer

#define crn_edgeColor						161	// color
#define crn_edgeColorTexmap					162	// texturemap
#define crn_edgeColorTexmapOn				163	// boolean
#define crn_edgeColorMapAmount				164	// float
#define crn_translucencyColor				375	// color
#define crn_translucencyColorTexmap			376	// texturemap
#define crn_translucencyColorTexmapOn		377	// boolean
#define crn_translucencyColorMapAmount		378	// float
#define crn_useComplexIor					451	// boolean
#define crn_complexIorNRed					452	// float
#define crn_complexIorNGreen				453	// float
#define crn_complexIorNBlue					454	// float
#define crn_complexIorKRed					455	// float
#define crn_complexIorKGreen				456	// float
#define crn_complexIorKBlue					457	// float
#define crn_iorMode							461	// integer

// CoronaNormal
#define crn_nrm_multplier					501
#define crn_nrm_normalMap					500
#define crn_nrm_addGamma					502
#define crn_nrm_flipRed						503
#define crn_nrm_flipGreen					504
#define crn_nrm_swapRedGreen				505
#define crn_nrm_gammaWarning				506
#define crn_nrm_additionalBump				507
#define crn_nrm_additionalBumpOn			509
#define crn_nrm_additionalBumpStrength		508

//=================================================
// USD ParamBlock index
//=================================================
//pBlock 0
#define usd_ao_affects_diffuse		0	// boolean
#define usd_ao_affects_reflection	1	// boolean
#define usd_normal_flip_red			2	// boolean
#define usd_normal_flip_green		3	// boolean
//pBlock 1
#define usd_useSpecularWorkflow		0	// boolean
#define usd_diffuseColor			1	// color
#define usd_diffuseColor_map		2	// texturemap
#define usd_metallic 				3	// float
#define usd_metallic_map			4	// texturemap
#define usd_specularcolor			5	// color
#define usd_specularColor_map		6	// texturemap
#define usd_roughness				7	// float
#define usd_roughness_map			8	// texturemap
#define usd_occlusion				9	// float
#define usd_occlusion_map			10	// texturemap
#define usd_normal					11	// color
#define usd_normal_map				12	// texturemap
#define usd_emissiveColor 			13	// color
#define usd_emissiveColor_map		14	// texturemap
#define usd_opacity					15	// float
#define usd_opacity_map				16	// texturemap
#define usd_opacityThreshold		17	// float
#define usd_displacement			18	// float
#define usd_displacement_map		19	// texturemap
#define usd_ior 					20	// float
#define usd_ior_map					21	// texturemap
#define usd_clearcoat				22	// float
#define usd_clearcoat_map			23	// texturemap
#define usd_clearcoatRoughness		24	// float
#define usd_clearcoatRoughness_map	25	// texturemap

//=================================================
// Pencil+4 ParamBlock index
//=================================================
//pBlock 0
#define pen_basicMaterial		0	// Mtl
#define pen_bumpEnable			14	// Int
#define pen_bumpAmount			15	// Int
#define pen_bumpMap				16	// Texmap

//BaseMaterial pBlock 3

#define pen_Zones				34
#define pen_Zone_posMin			0	// float
#define pen_Zone_posMax			1	// float
#define pen_Zone_color			3	// color
#define pen_Zone_mapOpacity		4	// float
#define pen_Zone_blendMode		7	// Int
#define pen_Zone_colorMap		13	// texmap

//=================================================
// OSL UberBitmap paramBlock 
//=================================================
#define UberBmp_UVSet		0	// Int
#define UberBmp_Scale		1	// float
#define UberBmp_Tiling		2	// Point3
#define UberBmp_Offset		3	// Point3
#define UberBmp_RealWorld	4	// float
#define UberBmp_RealWidth	5	// float
#define UberBmp_RealHeight	6	// float
#define UberBmp_Rotate		7	// float
#define UberBmp_RotCenter	8	// Point3
#define UberBmp_RotAxis		9	// Point3
#define UberBmp_FileName	10	// String
#define UberBmp_WrapMode	14	//  Point3
#define UberBmp_AutoGamma	15	// Int
#define UberBmp_ManuaklGamma 16	// float

