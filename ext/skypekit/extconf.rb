require 'mkmf'

LIBDIR      = Config::CONFIG['libdir']
INCLUDEDIR  = Config::CONFIG['includedir']
# TODO: add this like params
SKYPEKITDIR = "/home/leo/skype/skypekit-sdk_sdk-3.4.1.11_342604"

HEADER_DIRS = [
  # First search /opt/local for macports
  '/opt/local/include',

  # Then search /usr/local for people that installed from source
  '/usr/local/include',
  '/usr/local/cyassl/include',

  # Check the ruby install locations
  INCLUDEDIR,

  # Finally fall back to /usr
  '/usr/include',
  
  # skypekit
  "#{SKYPEKITDIR}/ipc/cpp/ssl/cyassl/include",
  "#{SKYPEKITDIR}/ipc/cpp/ssl/cyassl/ctaocrypt/include",
  "#{SKYPEKITDIR}/ipc/cpp",
  "#{SKYPEKITDIR}/ipc/cpp/platform/se",
  "#{SKYPEKITDIR}/interfaces/skype/cpp_embedded/src/api",
  "#{SKYPEKITDIR}/interfaces/skype/cpp_embedded/src/ipc",
  "#{SKYPEKITDIR}/interfaces/skype/cpp_embedded/src/types",
  "#{SKYPEKITDIR}/interfaces/skype/cpp_embedded/src/platform/threading",
  "#{SKYPEKITDIR}/interfaces/skype/cpp_embedded/src/platform/threading/pthread",
]

LIB_DIRS = [
  # First search /opt/local for macports
  '/opt/local/lib',

  # Then search /usr/local for people that installed from source
  '/usr/local/lib',
  '/usr/local/cyassl/lib',

  # Check the ruby install locations
  LIBDIR,

  # Finally fall back to /usr
  '/usr/lib',

  # skypekit
  "#{SKYPEKITDIR}/interfaces/skype/cpp_embedded/build",
  "#{SKYPEKITDIR}/bin/linux-x86",
]

dir_config('skypekit', HEADER_DIRS, LIB_DIRS)

# check cyassl
abort "cyassl headers is missing.  please install cyassl" if !find_header('cyassl_logging.h') || !find_header('ctc_rsa.h')
abort "cyassl lib is missing.  please install cyassl lib" unless find_library('cyassl', 'CyaSSL_CTX_use_certificate_buffer')

# chack skypekit
abort "skypekit headers is missing.  please install skypekit" unless find_header('skype-embedded_2.h')
abort "skypekit cppwrapper lib is missing.  please install skypekit libs" unless find_library('skypekit-cppwrapper_2_lib', nil)
abort "skypekit cyassl lib is missing.  please install skypekit libs" unless find_library('skypekit-cyassl_lib', nil)

#$libs = append_library( $libs, 'stdc++' ) # g++
#$libs = append_library($libs, "supc++") # g++
# Make flags
$CFLAGS << " -Wall -O2 -fno-exceptions -fno-rtti "
$defs.push "-MMD" 
$defs.push "-MP" 
$defs.push "-DNDEBUG"
$defs.push "-DSSL_LIB_CYASSL"
$defs.push "-DNO_FILESYSTEM"

create_makefile('skypekit/skypekit')
