project('gooroom-feedback', 'c')

cc = meson.get_compiler('c')

gnome = import('gnome')

gtk_dep = dependency('gtk+-3.0')
glib_dep = dependency('glib-2.0')
gio_dep = dependency('gio-2.0')

gooroom_feedback_sources = [
  'src/gooroom-feedback-application.c',
  'src/gooroom-feedback-window.c',
  'src/gooroom-feedback-utils.c',
  'src/gooroom-feedback-main.c'
]

gooroom_feedback_sources += gnome.compile_resources(
  'gooroom-feedback-resources',
  'resources/gooroom-feedback.gresource.xml',
  source_dir: 'resources'
)

gooroom_feedback = executable(
  'gooroom-feedback',
  gooroom_feedback_sources,
  dependencies: [gtk_dep, glib_dep, gio_dep],
)

po_dir = join_paths(meson.source_root(), 'po')

subdir('po')
#run_command('/bin/bash', 'makemo')
