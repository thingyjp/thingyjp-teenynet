project('teenynet-clients', 'c')

deps = [ dependency('libteenynet'),
		 dependency('libnlglue'),
         dependency('glib-2.0'),
         dependency('gio-2.0') ]
dhcpc_src = [ 'teenydhcpc.c' ]
executable('teenydhcpc', dhcpc_src, dependencies : deps)