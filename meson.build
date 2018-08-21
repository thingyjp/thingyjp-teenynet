project('teenynet', 'c')

deps = [ dependency('glib-2.0'),
         dependency('gio-2.0') ]
src = [ 'dhcp4_client.c', 'dhcp4_server.c', 'dhcp4_model.c', 'packetsocket.c', 'http.c' ]
lib = shared_library('teenynet', src,
                     dependencies : deps,
                     install : true)
install_headers(['include/teenynet/dhcp4.h',
                 'include/teenynet/dhcp4_client.h',
                 'include/teenynet/dhcp4_server.h',
                 'include/teenynet/http.h'], 
                subdir : 'teenynet')

pkg = import('pkgconfig')
pkg.generate(lib,
			 version : '0.1',
             filebase : 'libteenynet')