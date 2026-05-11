@ECHO OFF

SET VER_LIST=2020 2021 2022 2023 2024 2025 2026 2027

SETLOCAL EnableDelayedExpansion
FOR %%v IN (%VER_LIST%) DO (
	SET CONFIG=Release-Max%%v
	ECHO Building '!CONFIG!'
	msbuild HSglTF.sln /p:Configuration=!CONFIG! /p:Platform=x64 /v:minimal
	IF ERRORLEVEL 1 GOTO :Failed
)

exit /b

:Failed
	echo.
	echo ERROR: failed to build !CONFIG!
	echo.
	exit /b 1