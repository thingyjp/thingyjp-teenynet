project('teenynet', 'c')

deps = [ dependency('glib-2.0'),
         dependency('gio-2.0') ]
src = [ 'dhcp4_client.c', 'dhcp4_server.c', 'dhcp4_model.c', 'packetsocket.c', 'http.c' ]
lib = shared_library('teenynet', src,
                     dependencies : deps,
                     install : true)
                     
conf_data = configuration_data()
conf_data.set('debug', get_option('debug') ? 1 : 0)
conf_data.set('debug_ps', get_option('debug_packetsocket') ? 1 : 0)
conf_data.set('debug_d4m', get_option('debug_dhcp4model') ? 1 : 0)
conf_data.set('debug_d4c', get_option('debug_dhcp4client') ? 1 : 0)
configure_file(input : 'config.h.in',
               output : 'config.h',
               configuration : conf_data)
                     
install_headers(['include/teenynet/dhcp4.h',
                 'include/teenynet/dhcp4_client.h',
                 'include/teenynet/dhcp4_server.h',
                 'include/teenynet/http.h'], 
                subdir : 'teenynet')

pkg = import('pkgconfig')
pkg.generate(lib,
			 version : '0.1',
             filebase : 'libteenynet')