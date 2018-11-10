project('teenynet', 'c')

deps = [ dependency('glib-2.0'),
         dependency('gio-2.0'),
         dependency('gio-unix-2.0') ]
src = [ 'dhcp4_client.c',
        'dhcp4_server.c',
        'dhcp4_model.c',
        'packetsocket.c',
        'http.c',
        'unix.c' ]
lib = shared_library('teenynet', src,
                     dependencies : deps,
                     install : true)
                     
conf_data = configuration_data()
conf_data.set('debugbuild', get_option('debugbuild') ? 1 : 0)
conf_data.set('debug_ps', get_option('debug_packetsocket') ? 1 : 0)
conf_data.set('debug_d4m', get_option('debug_dhcp4model') ? 1 : 0)
conf_data.set('debug_d4c', get_option('debug_dhcp4client') ? 1 : 0)
configure_file(input : 'config.h.in',
               output : 'config.h',
               configuration : conf_data)
                     
install_headers(['include/teenynet/dhcp4.h',
                 'include/teenynet/dhcp4_client.h',
                 'include/teenynet/dhcp4_server.h',
                 'include/teenynet/http.h',
                 'include/teenynet/ip4.h',
                 'include/teenynet/unix.h'],
                subdir : 'teenynet')
                
inc = include_directories('include')

pkg = import('pkgconfig')
pkg.generate(lib,
			 version : '0.1',
             filebase : 'libteenynet')

teenynet_dep = declare_dependency(include_directories : inc, link_with : lib)
