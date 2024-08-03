project('tests', 'c', version: '0.0.1')

###########################################
############ CONFIGURE PROJECT ############
###########################################
deps = []
deps += dependency('gtk4')
deps += dependency('dbus-1')
deps += dependency('json-glib-1.0')

sources = files('example.c', 'njsonmanager.c', 'nlistener.c', 'nmanager.c', 'nobject.c')

executable(meson.project_name(), sources: sources, dependencies: deps)
