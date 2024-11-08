-- Defines which version of the project generator to use, by default
-- (can be overriden by the environment variable PROJECT_GENERATOR_VERSION)
PROJECT_GENERATOR_VERSION = 3

newoption({
	trigger = "gmcommon",
	description = "Sets the path to the garrysmod_common (https://github.com/danielga/garrysmod_common) directory",
	default = "garrysmod_common"
})

local gmcommon = assert(_OPTIONS.gmcommon or os.getenv("GARRYSMOD_COMMON"),
	"you didn't provide a path to your garrysmod_common (https://github.com/danielga/garrysmod_common) directory")
include(gmcommon)
include("overrides.lua")

local current_dir = _SCRIPT_DIR
CreateWorkspace({name = "headtrack_oculus", abi_compatible = false})
	-- Serverside module (gmsv prefix)
	-- Can define "source_path", where the source files are located
	-- Can define "manual_files", which allows you to manually add files to the project,
	-- instead of automatically including them from the "source_path"
	-- Can also define "abi_compatible", for project specific compatibility
	CreateProject({serverside = true, manual_files = false})
		-- Remove some or all of these includes if they're not needed
		IncludeHelpersExtended()
		--IncludeLuaShared()
		--IncludeSDKEngine()
		--IncludeSDKCommon()
		--IncludeBootil()
		IncludeSDKCommon()
		IncludeSDKTier0()
		IncludeSDKTier1()
		IncludeSDKTier2()
		IncludeSDKTier3()
		IncludeSDKMathlib()
		--IncludeSDKRaytrace()
		--IncludeSDKBitmap()
		--IncludeSDKVTF()
		--IncludeSteamAPI()
		--IncludeDetouring()
		--IncludeScanning()

		files({
			[[garrysmod_common\sourcesdk-minimal\tier1\convar.cpp]],
			[[README.md]],
		})

		targetsuffix("")
		filter("system:windows")
			files({"source/win32/*.cpp", "source/win32/*.hpp"})
			libdirs("openvr-2.5.1/lib/win32/")
			links("openvr_api.lib")

		filter("system:linux or macosx")
			files({"source/posix/*.cpp", "source/posix/*.hpp"})
			libdirs("openvr-2.5.1/lib/linux32/")
			targetextension(".so")
			links({
				"dl",
				"openvr_api"
			})