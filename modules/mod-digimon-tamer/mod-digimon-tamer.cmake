# The module owns a prepared-statement connection and needs the native client headers.
target_link_libraries(modules PRIVATE mysql)
