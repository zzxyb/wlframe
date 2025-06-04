# Font module build configuration

font_sources = files([
  'wlf_font.c',
  'wlf_font_backend.c',
])

# Platform-specific backend sources
if host_machine.system() == 'darwin'
  font_sources += files(['wlf_font_backend_macos.c'])
endif

if host_machine.system() == 'linux'
  font_sources += files(['wlf_font_backend_fontconfig.c'])
endif

# Dependencies
font_deps = []

# macOS Core Text framework
if host_machine.system() == 'darwin'
  font_deps += dependency('appleframeworks', modules: ['CoreText', 'CoreFoundation'], required: false)
endif

# Linux FontConfig
if host_machine.system() == 'linux'
  fontconfig_dep = dependency('fontconfig', required: false)
  if fontconfig_dep.found()
    font_deps += fontconfig_dep
  endif
endif

# Add to global files and dependencies
wlf_files += font_sources
wlf_deps += font_deps
