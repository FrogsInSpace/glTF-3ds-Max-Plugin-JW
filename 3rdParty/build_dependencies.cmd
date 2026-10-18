@echo off
setlocal EnableDelayedExpansion

:: usage:
::   build_dependencies.cmd            fetch, build and install the dependencies
::   build_dependencies.cmd clean      delete the dist folder and the build folders, the cloned repositories are kept
::   build_dependencies.cmd cleanall   additionally delete the cloned repositories and the downloaded archives
set "CLEAN_MODE="
if /I "%~1"=="clean" set "CLEAN_MODE=1"
if /I "%~1"=="cleanall" set "CLEAN_MODE=2"
if not "%~1"=="" if not defined CLEAN_MODE (
	echo ERROR: unknown argument %~1, expected clean or cleanall
	exit /b 1
)

:: Check for Visual Studio x64 dev environment
if not defined CLEAN_MODE if /I NOT "%VSCMD_ARG_TGT_ARCH%"=="x64" (
	echo.
	echo ##############################################
    echo Visual Studio x64 environment not initialized!
	echo ##############################################
    exit /b 1
)

:: the MSVC toolset version to use, has to match the minimal targeted 3ds Max version's SDK requirement ( v141 for 3ds Max 2020 - 2022 )
set "VC_TOOLSET=v141"

:: to hold the built libraries ( %DIST_DIR%\Debug and %DIST_DIR%\Release) and the public headers ( %DIST_DIR%\include\<dependency folder>)
set "DIST_DIR=%~dp0_dist_%VC_TOOLSET%"
:: to hold the downloaded zip archives
set "ARCHIVE_DIR=%~dp0_archives"
:: the folder containing this script ( with trailing backslash ), the dependency repositories are cloned into it
set "ROOT_DIR=%~dp0"

:: the clean modes do not need any of the build tooling, so they are handled before anything else
if defined CLEAN_MODE call :CLEAN
if defined CLEAN_MODE exit /b %ERRORLEVEL%

:: written as the very last step of a successful run, a dist folder without it comes from a failed or an interrupted run
set "COMPLETE_MARKER=%DIST_DIR%\.complete"
if exist "%DIST_DIR%" if not exist "%COMPLETE_MARKER%" (
	echo.
	echo WARNING: the dist folder exists but is not marked as complete.
	echo The previous run failed or was interrupted, or it was made by an older version of this script.
	echo Run 'build_dependencies.cmd clean' to start over from a clean state.
)
if exist "%COMPLETE_MARKER%" del "%COMPLETE_MARKER%"

:: skip tinygltf as it has been ( slightly ) modified and moved to the KHRglTFEporter Project folder
set "CLONE_TINYGLTF=0"

echo.
echo =====================================================
echo Building 3rd party dependencies
echo =====================================================
echo.

if "%CLONE_TINYGLTF%"=="1" (
	echo.
	echo "Cloning 'tinygltf' ..."
	echo =====================================================
	call :CLONE "tinygltf" "https://github.com/syoyo/tinygltf.git" "v3.0.1"
	if errorlevel 1 goto :ERROR
)

echo.
echo "Cloning and building 'draco' ..."
echo =====================================================
call :CLONE "draco" "https://github.com/Google/draco.git" "1.5.7"
if errorlevel 1 goto :ERROR
call :CONFIGURE "draco"
if errorlevel 1 goto :ERROR
:: the install rules of draco also install draco_decoder / draco_encoder, so the whole project has to be built
call :BUILD "draco"
if errorlevel 1 goto :ERROR
call :INSTALL "draco"
if errorlevel 1 goto :ERROR

echo.
echo "Cloning and building 'jsoncpp' ..."
echo =====================================================
call :CLONE "jsoncpp" "https://github.com/open-source-parsers/jsoncpp.git" "1.9.8"
if errorlevel 1 goto :ERROR
call :CONFIGURE "jsoncpp" "-DJSONCPP_WITH_TESTS=OFF" ^
    "-DJSONCPP_WITH_POST_BUILD_UNITTEST=OFF" ^
    "-DJSONCPP_WITH_PKGCONFIG_SUPPORT=OFF" ^
    "-DJSONCPP_WITH_CMAKE_PACKAGE=OFF" ^
    "-DJSONCPP_WITH_INSTALL=ON" ^
    "-DBUILD_SHARED_LIBS=OFF" ^
    "-DBUILD_OBJECT_LIBS=OFF" ^
    "-DBUILD_STATIC_LIBS=ON" ^
    "-DSTATIC_SUFFIX=_static"
if errorlevel 1 goto :ERROR
call :BUILD "jsoncpp" "jsoncpp_static"
if errorlevel 1 goto :ERROR
call :INSTALL "jsoncpp"
if errorlevel 1 goto :ERROR

echo.
echo "Cloning and building 'KTX' ..."
echo =====================================================
call :CLONE "KTX-Software" "https://github.com/KhronosGroup/KTX-Software.git" "v4.1.0"
if errorlevel 1 goto :ERROR
:: FIX for CMAKE 4: override "cmake_minimum_required 2.8.12" declared in cmake/cputypetest.cmake, which CMake 4 would reject
call :CONFIGURE "KTX-Software" "-DKTX_FEATURE_STATIC_LIBRARY=ON" "-DKTX_FEATURE_TOOLS=OFF" "-DCMAKE_POLICY_VERSION_MINIMUM=3.5" "-DISA_SSE41=ON"
if errorlevel 1 goto :ERROR
call :BUILD "KTX-Software" "ktx"
if errorlevel 1 goto :ERROR
call :INSTALL "KTX-Software"
if errorlevel 1 goto :ERROR
:: vkformat_enum.h is an internal libktx header that is not part of the KTX install rules, but the plugins use it
copy /Y "%~dp0KTX-Software\lib\vkformat_enum.h" "%DIST_DIR%\include\KTX-Software\" >nul
if errorlevel 1 goto :ERROR

echo.
echo "Cloning and building 'libwebp' ..."
echo =====================================================
call :CLONE "libwebp" "https://github.com/webmproject/libwebp.git" "v1.6.0"
if errorlevel 1 goto :ERROR
call :CONFIGURE "libwebp" "-DWEBP_BUILD_ANIM_UTILS=OFF" ^
	"-DWEBP_BUILD_CWEBP=OFF" ^
	"-DWEBP_BUILD_DWEBP=OFF" ^
	"-DWEBP_BUILD_GIF2WEBP=OFF" ^
	"-DWEBP_BUILD_IMG2WEBP=OFF" ^
	"-DWEBP_BUILD_VWEBP=OFF" ^
	"-DWEBP_BUILD_WEBPINFO=OFF" ^
	"-DWEBP_BUILD_WEBPMUX=OFF" ^
	"-DWEBP_BUILD_LIBWEBPMUX=OFF" ^
	"-DWEBP_BUILD_EXTRAS=OFF"

if errorlevel 1 goto :ERROR
:: webpdemux is always part of the libwebp install rules, so it has to be built as well
call :BUILD "libwebp" "webp" "sharpyuv" "webpdecoder" "webpdemux"
if errorlevel 1 goto :ERROR
call :INSTALL "libwebp"
if errorlevel 1 goto :ERROR
call :LICENSES
if errorlevel 1 goto :ERROR

echo.
echo =====================================================
type nul > "%COMPLETE_MARKER%"
echo SUCCESS: all operations finished successfully.
echo =====================================================
echo.
exit /b 0


:: =====================================================
:: CLONE
:: %~1 = target folder
:: %~2 = git repository
:: %~3 = tag
:: =====================================================

:CLONE
  if not exist "%~1" (
  	:: Check if git is available
	where git >nul 2>&1
	if errorlevel 1 (
		echo ERROR: Git is not available in PATH. Aborting...
		exit /b 1
	)  
    git -c advice.detachedHead=false clone --depth 1 -b "%~3" "%~2" "%~1"
    if errorlevel 1 (
        if exist "%~1" rmdir /S /Q "%~1"
        echo ERROR: failed to clone %~1
        exit /b 1
    )
    :: additionally also download the archive-zip for backup purposes 
    if not exist "%ARCHIVE_DIR%" mkdir "%ARCHIVE_DIR%"
    set "ZIP_URL=%~2"
    :: strip .git and append archive path
    set "ZIP_URL=!ZIP_URL:.git=!/archive/refs/tags/%~3.zip"
    echo !ZIP_URL!
    echo Downloading archive for %~2 %~3...
    curl -L -f --progress-bar -o "%ARCHIVE_DIR%/%~1-%~3.zip" "!ZIP_URL!"
    if errorlevel 1 (
      if exist "%~1" rmdir /S /Q "%~1"
      echo ERROR: Failed to download archive for %~2 %~3
      exit /b 1
    ) 
  )
  exit /b 0

:: =====================================================
:: CONFIGURE
:: %~1 = source folder
:: %~2 and higher = CMake options
:: =====================================================

:CONFIGURE
  :: Check if cmake is available
  where cmake >nul 2>&1
  if errorlevel 1 (
    echo ERROR: cmake is not available in PATH. Aborting...
    exit /b 1
  )  

  set "SRC_DIR=%~1"
  :: Strip source directory argument.
  shift

  set "CMAKE_OPTIONS="

  :CONFIGURE_OPTIONS
    if "%~1"=="" goto :RUN_CONFIGURE
    set "CMAKE_OPTIONS=%CMAKE_OPTIONS% %1"
    shift
    goto :CONFIGURE_OPTIONS

  :RUN_CONFIGURE
    echo.
    echo Configuring "%SRC_DIR%" ...
    pushd "%SRC_DIR%"
    if errorlevel 1 (
        echo ERROR: could not enter "%SRC_DIR%"
        exit /b 1
    )
    cmake -S . -B "build_%VC_TOOLSET%" ^
        -G "Visual Studio 17 2022" ^
        -A x64 ^
        -T "%VC_TOOLSET%" ^
        -D"CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG=%DIST_DIR%\Debug" ^
        -D"CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE=%DIST_DIR%\Release" ^
        -DCMAKE_CXX_STANDARD=17 ^
        -DCMAKE_CXX_STANDARD_REQUIRED=ON ^
        %CMAKE_OPTIONS%
    popd
    exit /b %ERRORLEVEL%

:: =====================================================
:: BUILD
:: %~1 = source folder
:: %~2 and higher = CMake targets
::
:: If no targets are specified, the default target is built.
:: =====================================================

:BUILD
  :: Check if cmake is available
  where cmake >nul 2>&1
  if errorlevel 1 (
    echo ERROR: cmake is not available in PATH. Aborting...
    exit /b 1
  )  
  
  set "SRC_DIR=%~1"
  :: Strip source directory argument.
  shift
  set "TARGETS="

  :EXTRACT_TARGETS
    if "%~1"=="" goto :RUN_BUILD
    set "TARGETS=%TARGETS% %1"
    shift
    goto :EXTRACT_TARGETS

  :RUN_BUILD
    echo.
    echo Building "%SRC_DIR%" ...
    pushd "%SRC_DIR%"
    if not "%TARGETS%"=="" (
      cmake --build build_%VC_TOOLSET% --config Debug --target %TARGETS%
      if errorlevel 1 goto :BUILD_FAILED
      cmake --build build_%VC_TOOLSET% --config Release --target %TARGETS%
      if errorlevel 1 goto :BUILD_FAILED
    ) else (
      cmake --build build_%VC_TOOLSET% --config Debug
      if errorlevel 1 goto :BUILD_FAILED
      cmake --build build_%VC_TOOLSET% --config Release
      if errorlevel 1 goto :BUILD_FAILED
    )
    popd
    exit /b %ERRORLEVEL%


:: =====================================================
:: BUILD_FAILED
:: Reached when a cmake --build call fails, so that the first failure is reported instead of being overwritten by a later success
:: =====================================================

:BUILD_FAILED
  popd
  echo ERROR: the build of "%SRC_DIR%" failed
  exit /b 1


:: =====================================================
:: INSTALL
:: %~1 = source folder ( also used as the name of the include sub-folder )
:: =====================================================

:INSTALL
  where cmake >nul 2>&1
  if errorlevel 1 (
    echo ERROR: cmake is not available in PATH. Aborting...
    exit /b 1
  )

  set "SRC_DIR=%~1"
  echo.
  echo Installing headers of "%SRC_DIR%" to "%DIST_DIR%\include\%SRC_DIR%" ...
  pushd "%SRC_DIR%"
  if errorlevel 1 (
    echo ERROR: could not enter "%SRC_DIR%"
    exit /b 1
  )

  set "STAGE_DIR=%CD%\build_%VC_TOOLSET%\_install_stage"
  if exist "%STAGE_DIR%" rmdir /S /Q "%STAGE_DIR%"

  cmake --install "build_%VC_TOOLSET%" --config Release --prefix "%STAGE_DIR%"
  if errorlevel 1 (
    popd
    echo ERROR: cmake --install failed for "%SRC_DIR%"
    exit /b 1
  )

  :: robocopy returns 0-7 on success, 8 and above on failure
  robocopy "%STAGE_DIR%\include" "%DIST_DIR%\include\%SRC_DIR%" /MIR /NJH /NJS /NP /NFL /NDL >nul
  if errorlevel 8 (
    popd
    echo ERROR: failed to copy the headers of "%SRC_DIR%"
    exit /b 1
  )

  popd
  exit /b 0


:: =====================================================
:: LICENSES
:: Copies the license files of the dependencies to %DIST_DIR%\licenses
:: in case the whole %DIST_DIR% is planned to be zipped and distributed later
:: =====================================================

:LICENSES
  set "LICENSE_DIR=%DIST_DIR%\licenses"
  if not exist "%LICENSE_DIR%" mkdir "%LICENSE_DIR%"
  echo.
  echo Copying licenses to "%LICENSE_DIR%" ...

  copy /Y "%ROOT_DIR%draco\LICENSE" "%LICENSE_DIR%\draco-LICENSE" >nul
  if errorlevel 1 exit /b 1
  copy /Y "%ROOT_DIR%jsoncpp\LICENSE" "%LICENSE_DIR%\jsoncpp-LICENSE" >nul
  if errorlevel 1 exit /b 1
  copy /Y "%ROOT_DIR%KTX-Software\LICENSE.md" "%LICENSE_DIR%\ktx-LICENSE.md" >nul
  if errorlevel 1 exit /b 1
  copy /Y "%ROOT_DIR%libwebp\COPYING" "%LICENSE_DIR%\libwebp-COPYING" >nul
  if errorlevel 1 exit /b 1
  copy /Y "%ROOT_DIR%libwebp\PATENTS" "%LICENSE_DIR%\libwebp-PATENTS" >nul
  if errorlevel 1 exit /b 1

  :: the license texts referenced by KTX ( robocopy returns 0-7 on success, 8 and above on failure )
  robocopy "%ROOT_DIR%KTX-Software\LICENSES" "%LICENSE_DIR%\KTX-LICENSES" /MIR /NJH /NJS /NP /NFL /NDL >nul
  if errorlevel 8 exit /b 1

  exit /b 0


:: =====================================================
:: CLEAN
:: %CLEAN_MODE% = 1 : delete the dist folder and the build folders of all dependencies
::                2 : additionally delete the cloned repositories and the downloaded archives
:: =====================================================

:CLEAN
  echo.
  echo =====================================================
  if "%CLEAN_MODE%"=="2" (
    echo Cleaning everything: dist folder, build folders, cloned repositories and archives
  ) else (
    echo Cleaning the dist folder and the build folders
  )
  echo =====================================================

  set "CLEAN_FAILED="
  call :REMOVE_DIR "%DIST_DIR%"
  for %%D in (draco jsoncpp KTX-Software libwebp) do (
    call :REMOVE_DIR "%ROOT_DIR%%%D\build_%VC_TOOLSET%"
    if "%CLEAN_MODE%"=="2" call :REMOVE_DIR "%ROOT_DIR%%%D"
  )
  if "%CLEAN_MODE%"=="2" call :REMOVE_DIR "%ARCHIVE_DIR%"

  if defined CLEAN_FAILED (
    echo ERROR: some folders could not be removed.
    echo Close Visual Studio and any other program using them and run the command again.
    exit /b 1
  )
  echo Done.
  exit /b 0

:REMOVE_DIR
  if not exist "%~1" exit /b 0
  echo Removing "%~1" ...
  rmdir /S /Q "%~1"
  if exist "%~1" set "CLEAN_FAILED=1"
  exit /b 0


:: =====================================================
:: ERROR
:: =====================================================

:ERROR
  echo =====================================================
  echo ERROR: dependency build failed.
  echo If the state of the folders is unclear, run 'build_dependencies.cmd clean' ( or 'cleanall' ) and start again.
  echo =====================================================
  echo.

  exit /b 1