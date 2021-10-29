# Create the intermediate directory
set(TFLIB_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/tf-lib")
set(TFLIB_INTERMEDIATE "${CMAKE_CURRENT_BINARY_DIR}/tf-lib")

# These are symlinks for backward compatibility
# Format is "new; old;"
set(COMPAT_LINKS
	 kb-bash.tf;  bind-bash.tf;
	 kb-emacs.tf; bind-emacs.tf;
	 complete.tf; completion.tf;
	 factoral.tf; factorial.tf;
	 filexfer.tf; file-xfer.tf;
	 local-eg.tf; local.tf.sample;
	 psh.tf;      pref-shell.tf;
	 spc-page.tf; space_page.tf;
	 spedwalk.tf; speedwalk.tf;
	 stack-q.tf;  stack_queue.tf;
	 world-q.tf;  worldqueue.tf;
)

execute_process(COMMAND mkdir -p ${TFLIB_INTERMEDIATE})
file(GLOB tflib_files "${TFLIB_SOURCE}/*")
foreach(file ${tflib_files})
    get_filename_component(file_name ${file} NAME)
    set(intermediate_file ${TFLIB_INTERMEDIATE}/${file_name})
    configure_file(${file} ${intermediate_file} COPYONLY)
endforeach()

while(COMPAT_LINKS)
    list(POP_FRONT COMPAT_LINKS new old)
    execute_process(
        COMMAND rm -f ${TFLIB_INTERMEDIATE}/${old}
        COMMAND ln -s ${new} ${TFLIB_INTERMEDIATE}/${old}
    )
endwhile()
