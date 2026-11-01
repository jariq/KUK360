# Packs the installed KUK360.AppDir into the AppImage; run by "cmake --install" at the end
# (see CMakeLists.txt). Expects APPDIR, APPIMAGE, RUNTIME, MKSQUASHFS and DESKTOP_ID, and
# optionally MAX_GLIBC and MAX_GLIBCXX.
#
# An AppImage is the AppImage runtime (a static ELF executable that mounts the image and
# starts AppRun) followed by a squashfs image of the AppDir.

# The AppImage uses glibc and libstdc++ of the system it runs on, so it runs only where these
# are at least as new as the newest symbol versions required by the executable, the Qt
# libraries and the plugins. With MAX_GLIBC (and MAX_GLIBCXX) set, as by build/linux/build-dist.sh
# for the release in the container, a newer requirement stops the build.
if(MAX_GLIBC OR MAX_GLIBCXX)
    file(GLOB_RECURSE binaries "${APPDIR}/usr/bin/*" "${APPDIR}/usr/lib/*.so.*"
        "${APPDIR}/usr/plugins/*.so")
    set(newest_GLIBC 0)
    set(newest_GLIBCXX 0)
    foreach(binary IN LISTS binaries)
        if(IS_SYMLINK "${binary}" OR binary MATCHES "\\.conf$")
            continue()
        endif()
        execute_process(COMMAND objdump -T "${binary}"
            OUTPUT_VARIABLE symbols COMMAND_ERROR_IS_FATAL ANY)
        foreach(library GLIBC GLIBCXX)
            string(REGEX MATCHALL "${library}_[0-9.]+" versions "${symbols}")
            foreach(version IN LISTS versions)
                string(REPLACE "${library}_" "" version "${version}")
                if(version VERSION_GREATER newest_${library})
                    set(newest_${library} "${version}")
                    set(newest_${library}_in "${binary}")
                endif()
            endforeach()
        endforeach()
    endforeach()
    message(STATUS "Requires GLIBC ${newest_GLIBC}, GLIBCXX ${newest_GLIBCXX}")
    foreach(library GLIBC GLIBCXX)
        if(MAX_${library} AND newest_${library} VERSION_GREATER MAX_${library})
            message(FATAL_ERROR "${newest_${library}_in} requires ${library} "
                "${newest_${library}}, more than ${MAX_${library}}.")
        endif()
    endforeach()
endif()

# Root of the AppDir: AppRun starts the application, the .desktop file and the icon
# describe it (also for appimaged and similar tools), .DirIcon is the thumbnail. All are
# relative symlinks into usr, so the files exist only once.
file(CREATE_LINK usr/bin/KUK360 "${APPDIR}/AppRun" SYMBOLIC)
file(CREATE_LINK usr/share/applications/${DESKTOP_ID}.desktop
    "${APPDIR}/${DESKTOP_ID}.desktop" SYMBOLIC)
file(CREATE_LINK usr/share/icons/hicolor/scalable/apps/${DESKTOP_ID}.svg
    "${APPDIR}/${DESKTOP_ID}.svg" SYMBOLIC)
file(CREATE_LINK ${DESKTOP_ID}.svg "${APPDIR}/.DirIcon" SYMBOLIC)

message(STATUS "Creating AppImage: ${APPIMAGE}")
set(squashfs "${APPIMAGE}.squashfs")
file(REMOVE "${squashfs}" "${APPIMAGE}")
# Files owned by root and without timestamps of the build machine; zstd as appimagetool.
execute_process(
    COMMAND "${MKSQUASHFS}" "${APPDIR}" "${squashfs}"
        -root-owned -noappend -all-time 0 -mkfs-time 0 -comp zstd -quiet
    COMMAND_ERROR_IS_FATAL ANY)

# runtime + squashfs, executable
execute_process(
    COMMAND cat "${RUNTIME}" "${squashfs}"
    OUTPUT_FILE "${APPIMAGE}"
    COMMAND_ERROR_IS_FATAL ANY)
file(REMOVE "${squashfs}")
file(CHMOD "${APPIMAGE}"
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE)
