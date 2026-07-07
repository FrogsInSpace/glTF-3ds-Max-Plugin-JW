
--=================================================
-- Standard Material Switcher
--=================================================
plugin material stdMtlSwitcher
name:"Standard Material Switcher"
classID:#(0x2fb3468d, 0x25fd57a2)
extends:standardMaterial replaceUI:true version:1
(
	local InitMtl = standardMaterial()

	fn SetParam mtl = (
		if mtl == undefined do mtl = InitMtl

		local ary = getPropNames mtl
		for p in ary do (
			try (
				local v = getProperty mtl p
				setProperty delegate p v
			) catch()
		)
	)
	parameters main rollout:params
	(
		index type:#integer default:1 ui:MtlSwitch
		Material type:#materialTab tabSize:8 ui:(mtlBtn1,mtlBtn2,mtlBtn3,mtlBtn4,mtlBtn5,mtlBtn6,mtlBtn7,mtlBtn8)

		on index set val do (
			SetParam Material[val]
		)
		on Material PostSet val idx do (
			if classOf Material[idx] != classOf InitMtl do Material[idx] = undefined
			if idx == index do SetParam Material[idx]
		)
	)
	rollout params "Parameters"
	(
		button updateBtn "Update" width:250
		radiobuttons MtlSwitch labels:#("", "","", "", "","", "", "") columns:1 align:#left offsets:#([0,8],[0,19],[0,30],[0,41],[0,52],[0,63],[0,74],[0,85])
		MaterialButton mtlBtn1 "" width:250 align:#left offset:[40,-205]
		MaterialButton mtlBtn2 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn3 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn4 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn5 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn6 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn7 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn8 "" width:250 align:#left offset:[40,0]

		on updateBtn pressed do (
			SetParam Material[MtlSwitch.state]
		)
	)
	fn Init = (
		SetParam Material[1]
	)
	on create do (
	)
)

--=================================================
-- Physical Material Switcher
--=================================================
plugin material physicalMtlSwitcher
name:"Physical Material Switcher"
classID:#(0x6af0dc4, 0x624849a5)
extends:PhysicalMaterial replaceUI:true version:1
(
	local InitMtl = PhysicalMaterial()
	
	fn SetParam mtl = (
		if mtl == undefined do mtl = InitMtl

		local ary = getPropNames mtl
		for p in ary do (
			try(
				local v = getProperty mtl p
				setProperty delegate p v
			) catch()
		)
	)
	parameters main rollout:params
	(
		index type:#integer default:1 ui:MtlSwitch
		Material type:#materialTab tabSize:8 ui:(mtlBtn1,mtlBtn2,mtlBtn3,mtlBtn4,mtlBtn5,mtlBtn6,mtlBtn7,mtlBtn8)

		on index set val do (
			SetParam Material[val]
		)
		on Material PostSet val idx do (
			if classOf Material[idx] != classOf InitMtl do Material[idx] = undefined
			if idx == index do SetParam Material[idx]
		)		
	)
	rollout params "Parameters"
	(
		button updateBtn "Update" width:250
		radiobuttons MtlSwitch labels:#("", "","", "", "","", "", "") columns:1 align:#left offsets:#([0,8],[0,19],[0,30],[0,41],[0,52],[0,63],[0,74],[0,85])
		MaterialButton mtlBtn1 "" width:250 align:#left offset:[40,-205]
		MaterialButton mtlBtn2 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn3 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn4 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn5 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn6 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn7 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn8 "" width:250 align:#left offset:[40,0]

		on updateBtn pressed do (
			SetParam Material[MtlSwitch.state]
		)
	)
	fn Init = (
		SetParam Material[1]
	)
	on create do (
	)
)

--=================================================
-- PBR Material Switcher
--=================================================
plugin material PBRMtlSwitcher
name:"PBR Material Switcher"
classID:#(0x2f4e61ce, 0x7a754a41)
extends:PBRMetalRough replaceUI:true version:1
(
	local InitMtl = PBRMetalRough()

	fn SetParam mtl = (
		if mtl == undefined do mtl = InitMtl

		local ary = getPropNames mtl
		for p in ary do (
			try (
				local v = getProperty mtl p
				setProperty delegate p v
			) catch()
		)
	)
	parameters main rollout:params
	(
		index type:#integer default:1 ui:MtlSwitch
		Material type:#materialTab tabSize:8 ui:(mtlBtn1,mtlBtn2,mtlBtn3,mtlBtn4,mtlBtn5,mtlBtn6,mtlBtn7,mtlBtn8)

		on index set val do (
			SetParam Material[val]
		)
		on Material PostSet val idx do (
			if classOf Material[idx] != classOf InitMtl do Material[idx] = undefined
			if idx == index do SetParam Material[idx]
		)
	)
	rollout params "Parameters"
	(
		button updateBtn "Update" width:250
		radiobuttons MtlSwitch labels:#("", "","", "", "","", "", "") columns:1 align:#left offsets:#([0,8],[0,19],[0,30],[0,41],[0,52],[0,63],[0,74],[0,85])
		MaterialButton mtlBtn1 "" width:250 align:#left offset:[40,-205]
		MaterialButton mtlBtn2 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn3 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn4 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn5 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn6 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn7 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn8 "" width:250 align:#left offset:[40,0]

		on updateBtn pressed do (
			SetParam Material[MtlSwitch.state]
		)
	)
	fn Init = (
		SetParam Material[1]
	)
	on create do (
	)
)

--=================================================
-- glTF Material Switcher
--=================================================
if glTFMaterial == undefined do glTFMaterial = standardMaterial

plugin material glTFMtlSwitcher
name:"glTFMaterial Switcher"
classID:#(0x9587a1a, 0x42bc9eb6)
invisible:( glTFMaterial == standardMaterial)
extends:glTFMaterial replaceUI:true version:1
(
	local InitMtl = glTFMaterial()
	
	fn SetParam mtl = (
		if mtl == undefined do mtl = InitMtl

		local ary = getPropNames mtl
		for p in ary do (
			try (
				local v = getProperty mtl p
				setProperty delegate p v
			) catch()
		)
	)
	parameters main rollout:params
	(
		index type:#integer default:1 ui:MtlSwitch
		Material type:#materialTab tabSize:8 ui:(mtlBtn1,mtlBtn2,mtlBtn3,mtlBtn4,mtlBtn5,mtlBtn6,mtlBtn7,mtlBtn8)

		on index set val do (
			SetParam Material[val]
		)
		on Material PostSet val idx do (
			if classOf Material[idx] != classOf InitMtl do Material[idx] = undefined
			if idx == index do SetParam Material[idx]
		)		
	)
	rollout params "Parameters"
	(
		button updateBtn "Update" width:250
		radiobuttons MtlSwitch labels:#("", "","", "", "","", "", "") columns:1 align:#left offsets:#([0,8],[0,19],[0,30],[0,41],[0,52],[0,63],[0,74],[0,85])
		MaterialButton mtlBtn1 "" width:250 align:#left offset:[40,-205]
		MaterialButton mtlBtn2 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn3 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn4 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn5 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn6 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn7 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn8 "" width:250 align:#left offset:[40,0]

		on updateBtn pressed do (
			SetParam Material[MtlSwitch.state]
		)
	)
	fn Init = (
		SetParam Material[1]
	)
	on create do (
	)
)

--=================================================
-- USD Material Switcher
--=================================================
if MaxUsdPreviewSurface == undefined do MaxUsdPreviewSurface = standardMaterial

plugin material USDMtlSwitcher
name:"USD Material Switcher"
classID:#(0x506a52fd, 0x75430b22)
invisible:( MaxUsdPreviewSurface == standardMaterial)
extends:MaxUsdPreviewSurface replaceUI:true version:1
(
	local InitMtl = MaxUsdPreviewSurface()
	
	fn SetParam mtl = (
		if mtl == undefined do mtl = InitMtl

		local ary = getPropNames mtl
		for p in ary do (
			try (
				local v = getProperty mtl p
				setProperty delegate p v
			) catch()
		)
	)
	parameters main rollout:params
	(
		index type:#integer default:1 ui:MtlSwitch
		Material type:#materialTab tabSize:8 ui:(mtlBtn1,mtlBtn2,mtlBtn3,mtlBtn4,mtlBtn5,mtlBtn6,mtlBtn7,mtlBtn8)

		on index set val do (
			SetParam Material[val]
		)
		on Material PostSet val idx do (
			if classOf Material[idx] != classOf InitMtl do Material[idx] = undefined
			if idx == index do SetParam Material[idx]
		)		
	)
	rollout params "Parameters"
	(
		button updateBtn "Update" width:250
		radiobuttons MtlSwitch labels:#("", "","", "", "","", "", "") columns:1 align:#left offsets:#([0,8],[0,19],[0,30],[0,41],[0,52],[0,63],[0,74],[0,85])
		MaterialButton mtlBtn1 "" width:250 align:#left offset:[40,-205]
		MaterialButton mtlBtn2 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn3 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn4 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn5 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn6 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn7 "" width:250 align:#left offset:[40,0]
		MaterialButton mtlBtn8 "" width:250 align:#left offset:[40,0]

		on updateBtn pressed do (
			SetParam Material[MtlSwitch.state]
		)
	)
	fn Init = (
		SetParam Material[1]
	)
	on create do (
	)
)
