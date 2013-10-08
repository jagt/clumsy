-- currently only supports vs2010.
-- and premake4 4.4 doesn't have vs2012 support yet. but seems auto upgrade works fine

if _ACTION == 'clean' then
    os.rmdir('./build')
    os.rmdir('./bin')
end

solution('clumsy')
    location("./build")
    configurations({'Debug', 'Release'})
    platforms({'x32', 'x64'})

    project('clumsy')
        language("C") -- not really a C++ project, just visual studio c subset sucks too hard
        includedirs({'external/WinDivert-1.0.5-MSVC/include'})
        files({'src/**.c', 'src/**.h'})
        -- flags({'ExtraWarnings', 'StaticRuntime', 'Symbols'})
        links({'WinDivert', 'iup', 'comctl32', 'Winmm', 'ws2_32'}) 
        --links({'gdi32', 'comdlg32', 'uuid', 'ole32'}) -- covered by default in vs already
        files({'./etc/clumsy.rc'})

        configuration('Debug')
            flags({'ExtraWarnings', 'Symbols'})
            defines({'_DEBUG'})
            kind("ConsoleApp")

        configuration('Release')
            flags({'Optimize'})
            defines({'NDEBUG'})
            kind("WindowedApp")

        configuration("vs*")
            defines({"_CRT_SECURE_NO_WARNINGS"})
            flags({'NoManifest'})
            buildoptions({'/wd"4214"'})

        configuration('x32')
            defines({'X32'}) -- defines would be passed to resource compiler for whatever reason
            includedirs({'external/iup-3.8_Win32_dll11_lib/include'})
            libdirs({
                'external/WinDivert-1.0.5-MSVC/x86',
                'external/iup-3.8_Win32_dll11_lib'
                })

        configuration('x64')
            defines({'X64'})
            includedirs({'external/iup-3.8_Win64_dll11_lib/include'})
            libdirs({
                'external/WinDivert-1.0.5-MSVC/amd64',
                'external/iup-3.8_Win64_dll11_lib'
                })

        configuration({"Debug", "x32"})
            local subdir = 'bin/debug/x32'
            targetdir(subdir)
            debugdir(subdir)
            -- cwd is ./build

            postbuildcommands({
                "robocopy ../external/WinDivert-1.0.5-MSVC/x86/ ../"   .. subdir .. '  *.dll *.sys *.inf > robolog.txt',
                "robocopy ../external/iup-3.8_Win32_dll11_lib ../"   .. subdir .. ' iup.dll >> robolog.txt',
                "exit /B 0"
            })

        configuration({"Debug", "x64"})
            local subdir = 'bin/debug/x64'
            targetdir(subdir)
            debugdir(subdir)
            postbuildcommands({
                "robocopy ../external/WinDivert-1.0.5-MSVC/amd64/ ../"   .. subdir .. '  *.dll *.sys *.inf > robolog.txt',
                "robocopy ../external/iup-3.8_Win64_dll11_lib ../"   .. subdir ..  ' iup.dll >> robolog.txt',
                "exit /B 0"
            })

        configuration({"Release", "x32"})
            local subdir = 'bin/release/x32'
            targetdir(subdir)
            debugdir(subdir)
            postbuildcommands({
                "robocopy ../external/WinDivert-1.0.5-MSVC/x86/ ../"   .. subdir .. '  *.dll *.sys *.inf > robolog.txt',
                "robocopy ../external/iup-3.8_Win32_dll11_lib ../"   .. subdir .. ' iup.dll >> robolog.txt',
                "exit /B 0"
            })

        configuration({"Release", "x64"})
            local subdir = 'bin/release/x64'
            targetdir(subdir)
            debugdir(subdir)
            postbuildcommands({
                "robocopy ../external/WinDivert-1.0.5-MSVC/amd64/ ../"   .. subdir .. '  *.dll *.sys *.inf > robolog.txt',
                "robocopy ../external/iup-3.8_Win64_dll11_lib ../"   .. subdir ..  ' iup.dll >> robolog.txt',
                "exit /B 0"
            })
