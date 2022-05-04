#
#
#
MACRO(admGetGitRevision _dir _rev)

        FIND_PROGRAM(GIT_EXECUTABLE git
                DOC "git command line client")
        MARK_AS_ADVANCED(GIT_EXECUTABLE)

        IF(GIT_EXECUTABLE)

                MESSAGE(STATUS "Getting git-svn version from ${_dir}")
                EXECUTE_PROCESS(
                        COMMAND ${GIT_EXECUTABLE} rev-parse  --show-toplevel ${_dir}
                        WORKING_DIRECTORY ${_dir} 
                        RESULT_VARIABLE result 
                        OUTPUT_VARIABLE topdir
                        )
                STRING(STRIP "${topdir}" topdir)
                EXECUTE_PROCESS(
                        COMMAND echo  log --format=oneline -1   ${topdir}
                        COMMAND xargs ${GIT_EXECUTABLE} 
                        COMMAND head -c 11
                        WORKING_DIRECTORY ${_dir} 
                        RESULT_VARIABLE result 
                        OUTPUT_VARIABLE output
                        )
                STRING(STRIP "${output}" output)
                STRING(SUBSTRING ${output} 0 4 output)
                MESSAGE(STATUS ": ${output}")

                SET( ${_rev} "${output}")
                       
        ELSE(GIT_EXECUTABLE)
                SET(ADM_GIT_SVN_REVISION 0)
        ENDIF(GIT_EXECUTABLE)

ENDMACRO(admGetGitRevision _dir _rev)

# FindSubversion.cmake ends here.
